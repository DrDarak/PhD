////////////////////////////////////////////////////////////////////////////////////
//                      David Bethel											  //
//                  Darak@compuserve.com										  //
//					    Qtree main code											  //
////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "codec.h"
#include "pdf.h"
#define MIN_BLOCK_SIZE 2

////////////////////////////////////////////////////////////////////////////////////

Qtree *Qtree_Create()
{
	Qtree *inst;
	bool ok=true;

	// basic codec data
	inst=(Qtree *)malloc(sizeof(Qtree));
	inst->m_BPP=(float)0.8;
	inst->m_MSEThreshold=(float)0.0;
	inst->m_noUpdate=true;
	inst->m_frameSize=0;
	inst->m_rateThreshold=0.0;
	inst->m_FFBPP=(float)0.8;
	inst->m_recoded=NULL;
	inst->m_postProcess=false;

	inst->m_compressed=Dump_Create();
	inst->m_image=VideoData_Create(352,352,288,false,false);
	inst->m_last=VideoData_Create(352,352,288,false,false);
	inst->m_backGround=VideoData_Create(352,352,288,false,true);
	inst->m_backGroundMask=VideoData_Create(352>>BG_SHIFT,352>>BG_SHIFT,288>>BG_SHIFT,false,true);
	// qtree data
	inst->m_size=16;
	inst->m_list=BuffList_Create(128);
	inst->m_top=NULL;
	inst->m_spareBlocks=NULL;
	inst->m_numBlocks=0;
	
	inst->m_qtree=Huffman_Create();
	inst->m_coeff[0]=Huffman_Create();
	inst->m_coeff[1]=Huffman_Create();
	inst->m_coeff[3]=inst->m_coeff[1];		// symetry	01 =10
	inst->m_coeff[2]=Huffman_Create();
	inst->m_coeff[5]=inst->m_coeff[2];		// symetry	02 =20
	inst->m_coeff[4]=Huffman_Create();

	// set huffman tables up from golbal codes them up
	if (ok)
		ok=Huffman_Setup(inst->m_qtree,inst->m_compressed,gPdfQ,0,3);
	if (ok)
		ok=Huffman_Setup(inst->m_coeff[0],inst->m_compressed,gPdf0,0,256);
	if (ok)
		ok=Huffman_Setup(inst->m_coeff[1],inst->m_compressed,gPdf1,-96,96);
	if (ok)
		ok=Huffman_Setup(inst->m_coeff[2],inst->m_compressed,gPdf2,-64,64);
	if (ok)
		ok=Huffman_Setup(inst->m_coeff[4],inst->m_compressed,gPdf4,-64,64);

	Qtree_SetFocus(inst,32,4);

	return inst;
}

////////////////////////////////////////////////////////////////////////////////////

void Qtree_Destroy(Qtree **inst)
{	
	Qtree *pt;
	if (*inst)
	{
		pt=*inst;
		Dump_Destroy(&pt->m_recoded);
		Dump_Destroy(&pt->m_compressed);
		VideoData_Destroy(&pt->m_image);
		VideoData_Destroy(&pt->m_last);
		VideoData_Destroy(&pt->m_backGround);
		VideoData_Destroy(&pt->m_backGroundMask);
		Huffman_Destroy(&pt->m_qtree);
		Huffman_Destroy(&pt->m_coeff[0]);
		Huffman_Destroy(&pt->m_coeff[1]);
		Huffman_Destroy(&pt->m_coeff[2]);
		pt->m_coeff[3]=NULL;
		Huffman_Destroy(&pt->m_coeff[4]);
		pt->m_coeff[5]=NULL;
		Qtree_DeleteListManager(pt);
		BuffList_Destroy(&pt->m_list);
		free(*inst);
	}
}

////////////////////////////////////////////////////////////////////////////////////

void Qtree_BackGroundRecode(Qtree *inst)
{
	if (!inst->m_recoded)
		inst->m_recoded=Dump_Create();
}

////////////////////////////////////////////////////////////////////////////////////

void Qtree_SetCompression(Qtree *inst,float bpp)
{
	inst->m_BPP=bpp;
	inst->m_frameSize=0;
}

////////////////////////////////////////////////////////////////////////////////////

void Qtree_SetFrameSize(Qtree *inst,unsigned int size)
{
	inst->m_frameSize=size;
}

////////////////////////////////////////////////////////////////////////////////////

void Qtree_SetMSEThreshold(Qtree *inst,float mse)	
{
	inst->m_MSEThreshold=mse;
}

////////////////////////////////////////////////////////////////////////////////////

void Qtree_SetRateThreshold(Qtree *inst,float rate)	
{
	inst->m_rateThreshold=rate;
}

////////////////////////////////////////////////////////////////////////////////////

bool Qtree_CompressImage(Qtree *inst,VideoData *image)
{
	bool ok=true; 
//	VideoData *image;
	
	if ((image->m_width)<32 || (image->m_height)<32)
	   return false;

	// copy image into codec - may need doihng better
	Qtree_AddImage(inst,image);

	// check that last frame is valide
	// if updating make sure last frmae is present
	//if it is not the same size
	ok=Qtree_PrepImages(inst);

	//rewind buffer - need to be set up before using comopressors 
	Dump_Rewind(inst->m_compressed);

	if (ok)
		ok=Qtree_WriteCompressedFileHeader(inst);

	// move stream into bit mode
	Dump_BitMode(inst->m_compressed);

	if (inst->m_noUpdate)
		Qtree_ClearBackGround(inst);

	// compress image
	if (ok)
		ok=Qtree_Compressor(inst);

	// termate buffer
	Dump_TerminateBitWrite(inst->m_compressed);

	// check for any background in last frame before copying
	if (ok)
		Qtree_BackGroundComparison(inst);

	// swap last image with cuurent one
	image=inst->m_image;
	if (ok)
	{
		inst->m_image=inst->m_last;
		inst->m_last=image;
	}

	return ok;
}

////////////////////////////////////////////////////////////////////////////////////

void Qtree_SetImageSize(Qtree *inst,int width,int height)
{
	int pitch;

	if (width!=inst->m_image->m_width ||
		height!=inst->m_image->m_height)
	{
		pitch=(width+0x1f)&~0x1f;	// convert to units of 32
		VideoData_Destroy(&inst->m_image);
		inst->m_image=VideoData_Create(pitch,width,height,false,false);
	}
}

////////////////////////////////////////////////////////////////////////////////////

void Qtree_AddImage(Qtree *inst,VideoData *image)
{
	int j;
	int width,height,hPitch,pitch,hInputPitch,inputPitch,hWidth;
	unsigned char *Y,*U,*V,*inY,*inU,*inV;

	width=image->m_width;
	height=image->m_height;
	hWidth=width>>1;
	Qtree_SetImageSize(inst,width,height);

	Y=inst->m_image->m_Y;
	U=inst->m_image->m_U;
	V=inst->m_image->m_V;
	pitch=inst->m_image->m_pitch;
	hPitch=pitch>>1;
	inY=image->m_Y;
	inU=image->m_U;
	inV=image->m_V;
	inputPitch=image->m_pitch;
	hInputPitch=inputPitch>>1;

	for (j=0;j<height;j++)
	{
		memcpy(Y,inY,width);
		memcpy(U,inU,hWidth);
		memcpy(V,inV,hWidth);
		Y+=pitch;
		U+=hPitch;
		V+=hPitch;
		inY+=inputPitch;
		inU+=hInputPitch;
		inV+=hInputPitch;
	}

}

////////////////////////////////////////////////////////////////////////////////////

bool Qtree_PrepImages(Qtree *inst)
{
	bool ok=true;
	int width,height,pitch;

	if (inst->m_last->m_width==inst->m_image->m_width &&
		inst->m_last->m_height==inst->m_image->m_height && 
		inst->m_last->m_pitch==inst->m_image->m_pitch)
		return true;

	width=inst->m_image->m_width;
	height=inst->m_image->m_height;
	pitch=inst->m_image->m_pitch;	

	VideoData_Destroy(&inst->m_last);
	inst->m_last=VideoData_Create(pitch,width,height,false,false);

	VideoData_Destroy(&inst->m_backGround);
	inst->m_backGround=VideoData_Create(pitch,width,height,false,true);

	// convert down to BG size
	width>>=BG_SHIFT;
	height>>=BG_SHIFT;
	pitch>>=BG_SHIFT;
	pitch=(width+0x1f)&~0x1f;	// convert to units of 32

	VideoData_Destroy(&inst->m_backGroundMask);
	inst->m_backGroundMask=VideoData_Create(pitch,width,height,false,true);
	
	// create new background
	Qtree_ClearBackGround(inst);
	return ok;

}

////////////////////////////////////////////////////////////////////////////////////

void Qtree_BackGroundComparison(Qtree *inst)
{
// compare last and present image to see where last image was same
// if different reset background cnt is same inc.... 
// if cnt > 5 then copy to background
	float mse=0.0;
	int i,j,k,l,width,height,pitch,maskJump;
	int sum,jump,diff,t_hold=6;
	unsigned char *imagePtr,*lastPtr,*ptr;

	if (inst->m_noUpdate)
		return;

	width=inst->m_image->m_width;
	height=inst->m_image->m_height;
	pitch=inst->m_image->m_pitch;
	jump=pitch-BG_BLOCKSIZE;
	maskJump=inst->m_backGroundMask->m_pitch-inst->m_backGroundMask->m_width;
	ptr=inst->m_backGroundMask->m_Y;


	for (j=0;j<height;j+=BG_BLOCKSIZE,ptr+=maskJump)
		for (i=0;i<width;i+=BG_BLOCKSIZE,ptr++)
		{
			// find mse of coded region
			sum=0;
			imagePtr=inst->m_image->m_Y+i+j*pitch;
			lastPtr=inst->m_last->m_Y+i+j*pitch;
			for (l=0;l<BG_BLOCKSIZE;l++,imagePtr+=jump,lastPtr+=jump)
				for (k=0;k<BG_BLOCKSIZE;k++,imagePtr++,lastPtr++)
				{
					diff=AbsLimit((*imagePtr-*lastPtr));
					sum+=diff*diff;
				}

			// access the mask location
			if (sum<BG_BLOCKSIZE2*16) 
				(*ptr)++;	
			else
				*ptr=0;

			// copy background in if its stable
			if (*ptr>=t_hold)
			{
				imagePtr=inst->m_image->m_Y+i+j*pitch;
				lastPtr=inst->m_backGround->m_Y+i+j*pitch;
				*ptr=0;
				for (l=0;l<BG_BLOCKSIZE;l++,imagePtr+=jump,lastPtr+=jump)
					for (k=0;k<BG_BLOCKSIZE;k++,imagePtr++,lastPtr++)
						*lastPtr=*imagePtr;
			}
		}
}

//////////////////////////////////////////////////////////////////
// must have image size setup before entering

void Qtree_DecompressData(Qtree *inst,unsigned char *pt,int size)
{
	if (pt)
	{
		memcpy(inst->m_compressed->m_pt,pt,size);
		inst->m_compressed->m_size=size;
	}
}

//////////////////////////////////////////////////////////////////

bool Qtree_DecompressImage(Qtree *inst,unsigned char *pt,int size)
{
	bool ok=true;
	VideoData *image;
	Dump *compressed,*recoded;
	int width,height,pitch;

	recoded=inst->m_recoded;
	compressed=inst->m_compressed;

	// copy in external data
	if (pt)
	{
		memcpy(compressed->m_pt,pt,size);
		compressed->m_size=size;
	}

	//rewind buufer 
	if (recoded)
	{
		Dump_Rewind(recoded);
		memcpy(recoded->m_pt,compressed->m_pt,compressed->m_size);
	}
	Dump_Rewind(compressed);

	// read header data
    if (!Qtree_ReadCompressedFileHeader(inst))
		return false;

	if (recoded)
	{
		recoded->m_packPtr+=compressed->m_packPtr-compressed->m_pt;
		Dump_BitMode(recoded);
	}
	// move into bit mode
	Dump_BitMode(compressed);

    // if updating make sure last frmae is present
	//if it is not the same size
	if (ok)
       ok=Qtree_PrepImages(inst);

	if (inst->m_noUpdate)
		Qtree_ClearBackGround(inst);
	
	width=inst->m_image->m_width;
	height=inst->m_image->m_height;
	pitch=inst->m_image->m_pitch;

	if (ok)
	   ok=Qtree_DecompressY(inst,inst->m_image->m_Y,inst->m_last->m_Y,inst->m_backGround->m_Y,
							width,height,pitch);
	if (ok)
	   ok=Qtree_DecompressY(inst,inst->m_image->m_U,inst->m_last->m_U,inst->m_backGround->m_U,
							width>>1,height,pitch>>1);
	if (ok)
	   ok=Qtree_DecompressY(inst,inst->m_image->m_V,inst->m_last->m_V,inst->m_backGround->m_V,
							width>>1,height,pitch>>1);
	// termate buffer
	Dump_TerminateBitRead(compressed);
	if (recoded)
		Dump_TerminateBitWrite(recoded);

	// check for any background in last frame before copying
	if (ok)
		Qtree_BackGroundComparison(inst);

	// swap last image with cuurent one
	image=inst->m_image;
	if (ok)
	{
		inst->m_image=inst->m_last;
		inst->m_last=image;
	}

	return ok;
}

//////////////////////////////////////////////////////////////////

void Qtree_ClearBackGround(Qtree *inst)
{
	VideoData_Set(inst->m_backGround,0);
	VideoData_Set(inst->m_backGroundMask,0);
	return;
}

//////////////////////////////////////////////////////////////////

bool Qtree_Compressor(Qtree *inst)
{
	bool ok=true;
	float bitsY=(float)0.0,bitsV=(float)0.0,bitsU=(float)0.0;
	float bits=(float)0.0;
	int width,height,pitch;

	// set focu for this instance
	Qtree_StartFocus(inst);

	// work out bits availible 
	if (!inst->m_frameSize)
		bitsY=(float)(inst->m_image->m_height*inst->m_image->m_width)*inst->m_BPP;
	else
		bitsY=(float)(inst->m_frameSize*8);

	if (inst->m_noUpdate)
		bits=(float)(inst->m_image->m_height*inst->m_image->m_width)*inst->m_FFBPP;

	// decide which to use - don't decide to use less
	if (bits>bitsY)
		bitsY=bits;

	// set colour compression strengths 
	bitsU=(float)0.075*(float)bitsY; // 15% colour 
	bitsV=(float)0.075*(float)bitsY;
	bitsY-=(bitsU+bitsV);     // keep totals bits same 
 	
	width=inst->m_image->m_width;
	height=inst->m_image->m_height;
	pitch=inst->m_image->m_pitch;
	if (ok)
		ok=Qtree_CompressY(inst,inst->m_image->m_Y,inst->m_last->m_Y,inst->m_backGround->m_Y,
						   width,height,pitch,(int)bitsY);
	if (ok)
		ok=Qtree_CompressY(inst,inst->m_image->m_U,inst->m_last->m_U,inst->m_backGround->m_U,
						   width>>1,height,pitch>>1,(int)bitsU);
	if (ok)
		ok=Qtree_CompressY(inst,inst->m_image->m_V,inst->m_last->m_V,inst->m_backGround->m_V,
						   width>>1,height,pitch>>1,(int)bitsV);

	Qtree_NextFocus(inst);
	return ok;

}

//////////////////////////////////////////////////////////////////

bool Qtree_CompressY
(Qtree *inst,
 unsigned char *image,
 unsigned char *last,
 unsigned char *backGround,
 int width,
 int height,
 int pitch,
 int bits)
{
	bool ok=true;
	int nBlocks;

	//test data to see if it can be coded
	if (!image)
	   ok=false;
	if (!inst->m_coeff[0] || !inst->m_qtree)
	   ok=false;

	//Setting up blocks
	if (ok)
	{
		if (inst->m_noUpdate || !last || !backGround)
		   nBlocks=Qtree_SetupBlocks(inst,image,NULL,NULL,width,height,pitch);
		else
			nBlocks=Qtree_SetupBlocks(inst,image,last,backGround,width,height,pitch);
		if (!nBlocks)
		   ok=false;
	}

	//Fitting basis to image
	if (ok)
	{
		ok=Qtree_FitBlocksToImage(inst,bits,inst->m_rateThreshold,false);
	
		//Huffman encode qtree;
		if (ok)
		   Qtree_EncodeQtreeImage(inst);

		//huffman encode image
		if (ok)
		   Qtree_HuffmanEncodeImage(inst);

		// clear image
		memset(image,0,height*pitch);
		if (ok)
			Qtree_UnquantiseImage(inst);
		
		//render image
		if (ok)
			Qtree_RenderImage(inst);
		
		// remove virtual list
		Qtree_FreeBlockList(inst);
	}

	return ok; 
}

//////////////////////////////////////////////////////////////////

int Qtree_SetupBlocks 
(Qtree *inst,
 unsigned char *image,
 unsigned char *lastImage,
 unsigned char *backGround,
 int width,
 int height,
 int pitch)
{
	int i,j,n_blocks;
	Block *block,*last;
         
	last=NULL;
	n_blocks=0;

	for (j=0;j<height;j+=inst->m_size)  
        for (i=0;i<width;i+=inst->m_size)
		{
	        n_blocks++;
            
			// allocate block memory 
            block=Qtree_GetFreeBlock(inst);
			if (!block)
			   return 0;

			Block_SetupBlock(block,i,j,inst->m_size,image,lastImage,backGround,pitch);

            // load top of tree into image  struct 
            if (i==0 && j==0)
               inst->m_top=block;
            else
                last->m_next=block; // form link list if applicable 

            // load last pointer for next pass 
            last=block;
		}   
		
	return n_blocks;
}

//////////////////////////////////////////////////////////////////

bool Qtree_FitBlocksToImage 
(Qtree *inst,
 int budget,
 float RDThreshold,
 bool recode)
{
	bool ok=true;
	Block *block,*best;
	int error=0;
	int bits=0,oldBits=0,i;
	int avg=0x0fffffff,oldAvg=0x0fffffff;
	float rateDistortion=0;
	int distortion[32],rate=0; // keep record of errors 
	Huffman **coeff,*qtree;
	BuffList *list;

	for (i=0;i<32;i++)
		distortion[i]=0x0fffffff;

    // code orginal tree of image 
    block=inst->m_top;
	coeff=inst->m_coeff;
	qtree=inst->m_qtree;
	list=inst->m_list;

    for (block=inst->m_top;block && !recode;block=block->m_next)
	{
		Block_CodeBlock(block);
		bits+=Block_ChoseMethod(block,coeff,qtree,false);
		// increase error
		error+=block->m_error;
		// put block back in list
		if (block->m_size>MIN_BLOCK_SIZE && block->m_error!=0)
			BuffList_Put(list,block);		
    }

//	BuffList_ClearListLinking(list);
//	return true;

	// now quadtree image 
	oldBits=bits;
    while (bits<budget || recode)
	{
		// save error
		distortion[rate]=error;
		rate++;
		// check for breakout every 32 blocks
		if (rate==32 && !recode)
		{
			oldAvg=avg;
			avg=0;
			for(i=0;i<32;i++)
				avg+=distortion[i];
			avg/=32;
		    rateDistortion=(float)(oldAvg-avg)/(float)(bits-oldBits);
			
			if (avg>=oldAvg || rateDistortion< RDThreshold)
				break;
			
			oldBits=bits;
		}
	
		rate%=32;

		best=BuffList_Get(list);
		if (best==NULL)
		   break;
		
		//remove bits for old block 
		switch (best->m_blockType)
		{
			case(currentFrameBlock):
				bits-=Block_HuffmanEstimateBlock(best,coeff);
				bits-=qtree->EstimateCompressSymbol(0); // code cuurents frame
				break;
			case(lastFrameBlock):
				bits-=qtree->EstimateCompressSymbol(2); // code last frame
				break;
			case(backGroundBlock):
				bits-=qtree->EstimateCompressSymbol(3);
				break;
		}
		error-=best->m_error;
		// add bits for split coefficent
   		bits+=qtree->EstimateCompressSymbol(1); // split

		// split best block into 4 quads 
		if (!Qtree_SplitBlock(inst,best))
			break; // bug out if no more blcoks are availible

		// code the four blocks 
		for (i=0;i<4;i++)
		{
			Block_CodeBlock(best);
			// is block is min_size only code it with intra/full coeff
			if (best->m_size<=MIN_BLOCK_SIZE)
			{
				Block_QuantiseBlock(best);
				bits+=Block_HuffmanEstimateBlock(best,coeff);
			}
			else
				bits+=Block_ChoseMethod(best,coeff,qtree,recode);
			
			// increase error
			error+=best->m_error;
			
			// put block back in list
			if (best->m_size>MIN_BLOCK_SIZE && (best->m_error!=0 || recode))
				BuffList_Put(list,best);
			
			//move to next block
			best=best->m_next;
		}   
	}

	BuffList_ClearListLinking(list);

    return true;

}

//////////////////////////////////////////////////////////////////

void Qtree_HuffmanEncodeImage(Qtree *inst)
{
    Block *block;
	Huffman **coeff;
	coeff=inst->m_coeff;
	for (block=inst->m_top;block;block=block->m_next)
		if (block->m_blockType==currentFrameBlock)
           Block_HuffmanEncodeBlock(block,coeff);

}

//////////////////////////////////////////////////////////////////

bool Qtree_DecompressY
(Qtree *inst,
 unsigned char *image,
 unsigned char *last,
 unsigned char *backGround,
 int width,
 int height,
 int pitch)
{
	bool ok=true;
	
	// check errything is valide
	if (!image)
	   ok=false;

	if (!inst->m_coeff[0] || !inst->m_qtree)
	   ok=false;

	//decode qtree
	if (ok)
	   ok=Qtree_DecodeQtreeImage(inst,image,last,backGround,width,height,pitch);

	//huffman decode
	if (ok)
	   Qtree_HuffmanDecodeImage(inst);

	//unquant image
	if (ok)
	   Qtree_UnquantiseImage(inst);

	//render image
	if (ok)
	   Qtree_RenderImage(inst);
	
	if (inst->m_recoded && ok)
		Qtree_Recode(inst);

	// postprocess image
	if (inst->m_postProcess)
		Qtree_PostProcess(inst,image,width,height,pitch);
	
	Qtree_FreeBlockList(inst);

	return ok;

}

//////////////////////////////////////////////////////////////////
#define FILTER_BLOCK_SIZE	16
bool Qtree_PostProcess
(Qtree *inst,
 unsigned char *image,
 int width,
 int height,
 int pitch)
{
	int i,j;
	Block *block;
	unsigned char data[FILTER_BLOCK_SIZE*5];
	BuffList *list;
	int jump=pitch-FILTER_BLOCK_SIZE;
	double a,b,c,y0,y1,y2;
	unsigned char a0,a1,a2,a3,a4,a5,a6;

	unsigned char *pt;
	// first quantise existing info
	for (block=inst->m_top;block!=NULL;block=block->m_next)
		if (block->m_size==FILTER_BLOCK_SIZE && 
			block->m_x>=FILTER_BLOCK_SIZE && block->m_x<width-FILTER_BLOCK_SIZE  &&
			block->m_y>=FILTER_BLOCK_SIZE && block->m_y<height-FILTER_BLOCK_SIZE )
		{
			pt=image+pitch*block->m_y+block->m_x;
			// get data
			for (i=0;i<FILTER_BLOCK_SIZE;i++,pt++)
			{
				a0=pt[i-pitch*2];
				a1=pt[i-pitch];
				a2=pt[i];
				a3=pt[i+pitch];
				a4=pt[i+pitch*2];
				a5=pt[i+pitch*3];
				pt[i]=(a0+a1+a2+a3+a4)/5;
				pt[i-pitch]=(a1+a2+a3+a4+a5)/5;
			}
	
			for (j=0;j<FILTER_BLOCK_SIZE;j++,pt+=jump)
			{
				a0=pt[-2];
				a1=pt[-1];
				a2=pt[0];
				a3=pt[1];
				a4=pt[2];
				a5=pt[3];
				pt[0]=(a0+a1+a2+a3+a4)/5;
				pt[1]=(a1+a2+a3+a4+a5)/5;
				pt+=FILTER_BLOCK_SIZE;
				a0=pt[-4];
				a1=pt[-3];
				a2=pt[-2];
				a3=pt[-1];
				a4=pt[0];
				a5=pt[1];
				pt[-2]=(a0+a1+a2+a3+a4)/5;
				pt[-1]=(a1+a2+a3+a4+a5)/5;
			}

			pt=image+pitch*block->m_y+block->m_x+(FILTER_BLOCK_SIZE-1)*pitch;
			for (i=0;i<FILTER_BLOCK_SIZE;i++,pt++)
			{
				a0=pt[i-pitch*3];
				a1=pt[i-pitch*2];
				a2=pt[i-pitch];
				a3=pt[i];
				a4=pt[i+pitch];
				a5=pt[i+2*pitch];

				pt[i-pitch]=(a0+a1+a2+a3+a4)/5;
				pt[i]=(a1+a2+a3+a4+a5)/5;
			}
		}

	return true;
}

//////////////////////////////////////////////////////////////////

void Qtree_Recode(Qtree *inst)
{
	int i;
	Block *block;
	BuffList *list;

	// first quantise existing info
	for (block=inst->m_top;block!=NULL;block=block->m_next)
		if (block->m_blockType==currentFrameBlock)
		   Block_QuantiseBlock(block);

	// now add it back to codeing list
	list=inst->m_list;
	list->m_getNew=true;
	for (block=inst->m_top;block;block=block->m_next)
		BuffList_Put(list,block);

	// now fit any uncoded blocks back
	Qtree_FitBlocksToImage(inst,0,0.0,true);

	// change coding stream
	inst->m_qtree->m_dump=inst->m_recoded;
	for (i=0;i<MAX_COEFF;i++)
		inst->m_coeff[i]->m_dump=inst->m_recoded;

	Qtree_EncodeQtreeImage(inst);
	Qtree_HuffmanEncodeImage(inst);

	// change coding stream
	inst->m_qtree->m_dump=inst->m_compressed;
	for (i=0;i<MAX_COEFF;i++)
		inst->m_coeff[i]->m_dump=inst->m_compressed;

	list->m_getNew=false;
}

//////////////////////////////////////////////////////////////////

void Qtree_HuffmanDecodeImage(Qtree *inst)
{
    Block *block;
	Huffman **coeff;
	coeff=inst->m_coeff;

	for (block=inst->m_top;block;block=block->m_next)
		if (block->m_blockType==currentFrameBlock)
           Block_HuffmanDecodeBlock(block,coeff);
 
}

//////////////////////////////////////////////////////////////////
//			Quad-tree Functions									//
//////////////////////////////////////////////////////////////////

#define AppendBlockToList(first,second) next=first->m_next; first->m_next=second; second->m_next=next

// LQDCT quadtree spliting function 

bool Qtree_SplitBlock(Qtree *inst,Block *parent)
{
	bool ok=true;
    int size;
    Block *child,*last,*next;

    // determine child size 
    size=parent->m_size/2;

    // now place children into block structures

    // Top Right 
	child=Qtree_GetFreeBlock(inst);
	if (!child)
	   ok=false;

	if (ok)
	{
		Block_InternalSetupBlock(child,parent->m_x+size,parent->m_y,size,parent);
		AppendBlockToList(parent,child);
		last=child;
	}

    // Bottom left 
	child=Qtree_GetFreeBlock(inst);
	if (!child)
	   ok=false;
	if (ok)
	{
		Block_InternalSetupBlock(child,parent->m_x,parent->m_y+size,size,parent);
		AppendBlockToList(last,child);
		last=child;
	}

    // Bottom Right 
	    // Bottom left 
	child=Qtree_GetFreeBlock(inst);
	if (!child)
	   ok=false;
	if (ok)
	{
		Block_InternalSetupBlock(child,parent->m_x+size,parent->m_y+size,size,parent);
		AppendBlockToList(last,child);
	}

    //Top left - reset data
	parent->m_blockType=currentFrameBlock;
    parent->m_jump+=(parent->m_size-size); 
    parent->m_size=size;
	parent->m_error=0;
    
	return ok;
}

//////////////////////////////////////////////////////////////////

void Qtree_EncodeQtreeImage(Qtree *inst)
{
    int nitems;
    Block *block;
	// cnt units
	for (nitems=0,block=inst->m_top;block;block=block->m_next)
		nitems++;

    // encode qtree 
    block=inst->m_top;
    while (block!=NULL)
	{
          if (block->m_size==inst->m_size)
		  {
			 Huffman_CompressSymbol(inst->m_qtree,block->m_blockType);
			 block=block->m_next;
          }
          else 
		  {
			  Huffman_CompressSymbol(inst->m_qtree,1);
              block=Qtree_EncodeQtreeBlock(inst->m_qtree,block,inst->m_size/2);
          }
    }      
}

//////////////////////////////////////////////////////////////////

Block * Qtree_EncodeQtreeBlock(Huffman *qtree,Block *block,int size)
{
    int i;
	int symbol;

    for (i=0;i<4;i++)
        if (block->m_size<size)
		{  // split
			symbol=1;
			Huffman_CompressSymbol(qtree,1);
			block=Qtree_EncodeQtreeBlock(qtree,block,size/2);
        }
        else 
		{	// code current block somehow
			if (block->m_size>MIN_BLOCK_SIZE) // only plane block codeing at minsize 
				Huffman_CompressSymbol(qtree,block->m_blockType);
			block=block->m_next;
       }

    return block;
 
}

//////////////////////////////////////////////////////////////////
// Qtree decoders... 

bool Qtree_DecodeQtreeImage
(Qtree *inst,
 unsigned char  *image,
 unsigned char *last,
 unsigned char *backGround,
 int width,
 int height,
 int pitch)
{
	bool ok=true;
	int x,y;
	Block top;
	Block *block;
 
	// allocate dummy block 
	block=&top;
	x=0;
	y=0;
 
    for (;ok;) 
	{
        switch (Huffman_UncompressSymbol(inst->m_qtree))
		{
			case (0):
				block->m_next=Qtree_GetFreeBlock(inst);
				Block_SetupBlock(block->m_next,x,y,inst->m_size,image,last,backGround,pitch);
				block->m_next->m_blockType=currentFrameBlock;
				block=block->m_next;
				break;
			case(1):
				block=Qtree_DecodeQtreeBlock(inst,image,last,backGround,block,x,y,inst->m_size/2,pitch);
				break;
			case(2):
				block->m_next=Qtree_GetFreeBlock(inst);
				Block_SetupBlock(block->m_next,x,y,inst->m_size,image,last,backGround,pitch);
				block->m_next->m_blockType=lastFrameBlock;
				block=block->m_next;
				break;
			case(3):
				block->m_next=Qtree_GetFreeBlock(inst);
				Block_SetupBlock(block->m_next,x,y,inst->m_size,image,last,backGround,pitch);
				block->m_next->m_blockType=backGroundBlock;
				block=block->m_next;
				break;
			default:
				ok=false;//panic

        }

		// problems
		if (block==NULL)
		{	
			ok=false;
			break;
		}

        // increament position in image 
        if (x+inst->m_size<width)
           x+=inst->m_size;
        else 
		{
             x=0;
             y+=inst->m_size;
        }
           
        if (y>=height)
		{
           block->m_next=NULL;
           break;
        }
    }

	// store tmp true top location
	if (ok)
       inst->m_top=top.m_next;
    
	return ok;
}

//////////////////////////////////////////////////////////////////

Block *Qtree_DecodeQtreeBlock
(Qtree *inst,
 unsigned char *image,
 unsigned char *last,
 unsigned char *backGround,
 Block *block,
 int x,
 int y,
 int size,
 int pitch)
{
    int i,j;

    for (j=0;j<size*2;j+=size)
        for (i=0;i<size*2;i+=size)
			switch (Huffman_UncompressSymbol(inst->m_qtree))
			{
				case (0):
					block->m_next=Qtree_GetFreeBlock(inst);
					Block_SetupBlock(block->m_next,x+i,y+j,size,image,last,backGround,pitch);
					block->m_next->m_blockType=currentFrameBlock;
					block=block->m_next;
					break;
				case(1):
					if (size/2>MIN_BLOCK_SIZE)
						block=Qtree_DecodeQtreeBlock(inst,image,last,backGround,block,x+i,y+j,size/2,pitch);
					else
						block=Qtree_DecodeMinSizeQtreeBlocks(inst,image,last,backGround,block,x+i,y+j,size/2,pitch);
					break;
				case(2):
					block->m_next=Qtree_GetFreeBlock(inst);
					Block_SetupBlock(block->m_next,x+i,y+j,size,image,last,backGround,pitch);
					block->m_next->m_blockType=lastFrameBlock;
					block=block->m_next;
					break;
				case(3):
					block->m_next=Qtree_GetFreeBlock(inst);
					Block_SetupBlock(block->m_next,x+i,y+j,size,image,last,backGround,pitch);
					block->m_next->m_blockType=backGroundBlock;
					block=block->m_next;
					break;
				default:
					return NULL;//panic
			}

    return block;
 
}

//////////////////////////////////////////////////////////////////

Block *Qtree_DecodeMinSizeQtreeBlocks
(Qtree *inst,
 unsigned char *image,
 unsigned char *last,
 unsigned char *backGround,
 Block *block,
 int x,
 int y,
 int size,
 int pitch)
{
    int i,j;
	int size2=2*size;

    for (j=0;j<size2;j+=size)
        for (i=0;i<size2;i+=size)
		{
			block->m_next=Qtree_GetFreeBlock(inst);
			Block_SetupBlock(block->m_next,x+i,y+j,size,image,last,backGround,pitch);
			block->m_next->m_blockType=currentFrameBlock;
			block=block->m_next;
		}

	return block;
}

//////////////////////////////////////////////////////////////////
//		Misc block-image functions								//
//////////////////////////////////////////////////////////////////

void Qtree_UnquantiseImage(Qtree *inst)
{
	Block *block;
	for (block=inst->m_top;block;block=block->m_next)
		if (block->m_blockType==currentFrameBlock)
		   Block_UnquantiseBlock(block);
	return;
}

//////////////////////////////////////////////////////////////////

void Qtree_RenderImage(Qtree *inst)
{
	Block *block;

	for (block=inst->m_top;block;block=block->m_next)
		switch(block->m_blockType)
		{
			case(currentFrameBlock):	Block_RenderBlock(block);			break;
			case(lastFrameBlock):		Block_CopyLastFrame(block);			break;
			case(backGroundBlock):		Block_CopyBackGroundFrame(block);	break;
		}
	return;
}

////////////////////////////////////////////////////////////////////////////////////
//				Free list managment functions								      //
////////////////////////////////////////////////////////////////////////////////////

Block *Qtree_GetFreeBlock(Qtree *inst)
{
	Block *block=NULL;

	if (!inst->m_spareBlocks)
	{
		// make N new blocks
		int i,N=10;
		Block *last=NULL;
		
		for (i=0;i<N;i++)
		{
			block=Block_Create();
			if (!block)
			   return NULL;
			// make sure list is NULL terminated
			block->m_next=NULL;
			
			if (last)
			   last->m_next=block;
			else
				inst->m_spareBlocks=block;

			// save last block
			last=block;
		}
		inst->m_numBlocks+=10;
	}

	// get block for return
	block=inst->m_spareBlocks;
	// move spare list on
	inst->m_spareBlocks=inst->m_spareBlocks->m_next; 

	// null terminate new block
	block->m_next=NULL;
	block->m_sortNext=NULL;

	return block;

}

//////////////////////////////////////////////////////////////////

void Qtree_FreeBlockList(Qtree *inst)
{
	// need to retrun all blocks in m_top
	// to m_spareblocks

	Block *block,*spareBlocks,*top;
	top=inst->m_top;
	spareBlocks=inst->m_spareBlocks;

	while (top)
	{
		// save tmp of the top of the block list
		block=top;
		// move top of list on to next entery
		top=top->m_next;
		// make top of spare blocks list equal to tmp block
		block->m_next=spareBlocks;
		spareBlocks=block;
	}
	inst->m_top=NULL;
	inst->m_spareBlocks=spareBlocks;
	return;

}

//////////////////////////////////////////////////////////////////

void Qtree_DeleteListManager(Qtree *inst)
{

	Qtree_FreeAllBlocks(inst->m_spareBlocks);
	inst->m_spareBlocks=NULL;
	Qtree_FreeAllBlocks(inst->m_top);
	inst->m_top=NULL;

}

//////////////////////////////////////////////////////////////////

void Qtree_FreeAllBlocks(Block *top)
{
	Block *block,*next;
    block=top;

	for (block=top;block;block=next)
	{
          next=block->m_next;
	      Block_Destroy(&block);
    }
	return;
}

//////////////////////////////////////////////////////////////////

unsigned int StringToInt(char *s,int length)
{
	char tmp[256];
	unsigned int result;

	strcpy(tmp,s);	
	tmp[length]=0;

	result=atoi(tmp);

	return result;
}

//////////////////////////////////////////////////////////////////

bool Qtree_WriteCompressedFileHeader(Qtree *inst)
{
	int width,height;
	unsigned char *packPtr;

	width=inst->m_image->m_width; 
    height=inst->m_image->m_height; 

	// write image stats + comments 
	packPtr=inst->m_compressed->m_packPtr;
	*packPtr++='L';
	*packPtr++='D';
	*packPtr++='C';
	*packPtr++='T';
	
	*packPtr++=(width>>8) & 0xff;
	*packPtr++=width & 0xff;
	*packPtr++=(height>>8) & 0xff;
	*packPtr++=height & 0xff;

	if (inst->m_noUpdate)
		*packPtr++=1;
	else
		*packPtr++=0;
	inst->m_compressed->m_packPtr=packPtr;
	return true;
}

//////////////////////////////////////////////////////////////////

bool Qtree_ReadCompressedFileHeader(Qtree *inst)
{
	int width,height;
	unsigned char *packPtr;

	// write image stats + comments 
	packPtr=inst->m_compressed->m_packPtr;
	if (memcmp(packPtr,"LDCT",4))
		return false;
	packPtr+=4;
	
	width=*packPtr++;
	width<<=8;
	width|=*packPtr++;
	height=*packPtr++;
	height<<=8;
	height|=*packPtr++;

	inst->m_noUpdate=false;
	if (*packPtr++)
		inst->m_noUpdate=true;

	Qtree_SetImageSize(inst,width,height);
	inst->m_compressed->m_packPtr=packPtr;
	return true;

}

//////////////////////////////////////////////////////////////////