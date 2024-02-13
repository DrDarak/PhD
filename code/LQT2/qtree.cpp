/*##############################################*/
/*				David Bethel					*/
/*		      Darak@compuserve.com				*/
/*				 LQTDCT Codec					*/
/*##############################################*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "blocks.h"
#include "image.h"
#include "List.h"
#include "IOBuff.h"
#include "qtree.h"


//////////////////////////////////////////////////////////////////

QtreeCodec::QtreeCodec():
Codec(),
m_size(16),
m_list(128), // length of buffer
m_qtree(NULL),
m_top(NULL),
m_spareBlocks(NULL),
m_type(normal)

{
	int i;

	for (i=0;i<MAX_COEFF;i++)
		m_coeff[i]=NULL;
}

//////////////////////////////////////////////////////////////////

QtreeCodec::~QtreeCodec()
{
	int i;
	
	if (m_qtree)
	{
		// read qtree huff tree from file 
		delete m_qtree;
	}


	if (m_coeff[0])
	{
		for (i=0;i<MAX_COEFF;i++)
			if (m_coeff[i])
			   delete m_coeff[i];
	}

	DeleteListManager();

}

//////////////////////////////////////////////////////////////////

bool QtreeCodec::SetFileNames(char *inputFname,char *outputFname,char *pdfFname)
{
	bool ok=true;

	ok=Codec::SetFileNames(inputFname,outputFname);
	if (pdfFname)
	   if (!strcpy(m_pdfFname,pdfFname))
	      ok=false;

	return ok;
}


//////////////////////////////////////////////////////////////////

bool QtreeCodec::CompressY(ImageData *image,ImageData *last,int bits)
{
	bool ok=true;
	int n_blocks;

	//Setting up blocks
	if (m_noUpdate)
	   n_blocks=SetupBlocks(image,NULL);
	else
        n_blocks=SetupBlocks(image,last);

	//Fitting basis to image
    FitBlocksToImage(bits);

	//Huffman encode qtree;
	EncodeQtreeImage();

	//huffman encode image
    HuffmanEncodeImage();

	// clear rubbish 
	FreeBlockList();

	return ok; // needs to implement Error trapping in o
			   // other routines
}

//////////////////////////////////////////////////////////////////

int QtreeCodec::SetupBlocks (ImageData *image,ImageData *lastImage)
{
	int i,j,n_blocks;
	Block *block,*last;
         
	last=NULL;
	n_blocks=0;

	for (j=0;j<image->m_height;j+=m_size)  
        for (i=0;i<image->m_width;i+=m_size){
	        n_blocks++;
            
			// allocate block memory 
            block=GetFreeBlock();
			if (!block)
			   return 0;
			block->SetupBlock(i,j,m_size,image,lastImage);

            // load top of tree into image  struct 
            if (i==0 && j==0)
               m_top=block;
            else
                last->m_next=block; // form link list if applicable 

            // load last pointer for next pass 
            last=block;
		}   
		
	return n_blocks;
}

//////////////////////////////////////////////////////////////////

bool QtreeCodec::FitBlocksToImage (int budget)
{
	bool ok=true;
	Block *block,*best;
	int error,errorLast,errorCurrent;
	int bits,i;

    // code orginal tree of image 
	error=0;
	bits=0;
    block=m_top;

    while (block!=NULL){
          block->CodeBlock();
		  errorCurrent=block->CalcCurrentBlockError();
		  
		  // decide which block to use
		  if (block->m_isLastFrame)
		  {
			 errorLast=block->CalcLastBlockError();
			 if (errorLast<=errorCurrent)
			 {
				block->m_error=errorLast;
				block->m_useCurrentFrame=false;
			 }
			 else
			 {
				block->m_error=errorCurrent;
			    block->m_useCurrentFrame=true;
			 }
		  }
		  else
			  block->m_error=errorCurrent;

		  if (block->m_useCurrentFrame)
		  {

              block->QuantiseBlock();
              bits+=block->HuffmanEstimateBlock(m_coeff);
		  }
		  else
			  bits++; // for using last sysmbol - may need changing

		  error+=block->m_error;
		  bits++;
		  if (block->m_error!=0)
		     m_list.Put(block);
          block=block->m_next;

    }
//	m_list.ClearListLinking();
//return true;
	// now quadtree image 
    while (bits<budget)
	{
		best=m_list.Get();
		if (best==NULL)
		   break;

		//remove bits for oold block 
		if (best->m_useCurrentFrame)
		   bits-=best->HuffmanEstimateBlock(m_coeff);
		else
			bits--;
					
		error-=best->m_error;
   
		// split best block into 4 quads 
		SplitBlock(best);

		// code the four blocks 
		for (i=0;i<4;i++)
		{
			best->CodeBlock();
			bits++;
			if (best->m_size<=2)
			{
				best->QuantiseBlock();
				bits+=best->HuffmanEstimateBlock(m_coeff);
				best=best->m_next;
				continue;
			}
			
			errorCurrent=best->CalcCurrentBlockError();
			
			// is last frame exists try it...
			if (best->m_isLastFrame)
			{
				errorLast=best->CalcLastBlockError();
				if (errorLast<=errorCurrent)
				{
					best->m_error=errorLast;
					best->m_useCurrentFrame=false;
				}
				else
				{
				    best->m_error=errorCurrent;
					best->m_useCurrentFrame=true;
				}
			}
			else
				best->m_error=errorCurrent;
			    	
			// put block into list
			if (best->m_error!=0)
			   m_list.Put(best);
			
			error+=best->m_error;
		    if (best->m_useCurrentFrame)
			{
			   best->QuantiseBlock();
			   bits+=best->HuffmanEstimateBlock(m_coeff);
			}
			else
			    bits++; // for using last sysmbol - may need changing

			best=best->m_next;
		}   
	}

	m_list.ClearListLinking();

    return true;

}

//////////////////////////////////////////////////////////////////

void QtreeCodec::HuffmanEncodeImage()
{
    Block *block;
 
	for (block=m_top;block;block=block->m_next)
		if (block->m_useCurrentFrame)
           block->HuffmanEncodeBlock(m_coeff);

}


//////////////////////////////////////////////////////////////////

bool QtreeCodec::DecompressY(ImageData *image,ImageData *last)
{
	bool ok=true;

	//decode qtree
	if (ok)
	   ok=DecodeQtreeImage(image,last);

	//huffman decode
	if (ok)
	   HuffmanDecodeImage();

	//unquant image
	if (ok)
	   UnquantiseImage();

	//render image
	if (ok)
	   RenderImage();

	FreeBlockList();

	return ok;

}

//////////////////////////////////////////////////////////////////

void QtreeCodec::HuffmanDecodeImage()
{
    Block *block;
 
	for (block=m_top;block;block=block->m_next)
		if (block->m_useCurrentFrame)
           block->HuffmanDecodeBlock(m_coeff);
 
}

//////////////////////////////////////////////////////////////////
//			Huffman coder Functions								//
//////////////////////////////////////////////////////////////////
// may be best in a derived version of huff ?


bool QtreeCodec::SetupPDF(char *pdfFname)
{
	FILE *fp=NULL;
	bool ok=true;
	int i;
	
	/* setup huffman coders */
	if (pdfFname)
	   sprintf(m_pdfFname,"%s",pdfFname);

	m_qtree=new Huffman;

	if (!m_qtree)
		ok=false;

	// read qtree huff tree from file 
	if (!m_compressed)
		ok=false;

	if (ok)
	{
		fp=fopen(m_pdfFname,"rb");
		if (!fp)
		   ok=false;
		if (ok)
		{
			m_qtree->SetupHuffmanTree (m_compressed,fp);   
			
			// load in huff table for basis functions			
			for (i=0;i<MAX_COEFF&&ok;i++)
			{
				//allocate space for tables 
				m_coeff[i]=new Huffman;
				if (!m_coeff[i])
					ok=false;
				// read huff tree from file 
				if (ok)
					m_coeff[i]->SetupHuffmanTree (m_compressed,fp);
			}

			// close data file 
			fclose(fp);
		}
	}

	return ok;
}

//////////////////////////////////////////////////////////////////

bool QtreeCodec::WipePDF()
{
	bool ok=true;
	int i;
	FILE *fp=NULL;

    if (m_type==wipe)
	{
        fp=fopen(m_pdfFname,"wb");
		if (!fp)
		   ok=false;

		if (ok)
		{
			// wipe qtree pdfs 
			m_qtree->WipeHuffmanPDF(fp);
			for (i=0;i<MAX_COEFF;i++)
				m_coeff[i]->WipeHuffmanPDF(fp);
			fclose(fp);
		}   
    }
	else 
		ok=false;

	return ok;
}

//////////////////////////////////////////////////////////////////

bool QtreeCodec::AppendPDF()
{
	bool ok=true;
	FILE *fp=NULL;
	int i;
	
	// huffman file options  
    if (m_type==append)
	{
        //printf("appending huffman pdf to disk\n"); 
		fp=fopen(m_pdfFname,"wb");

		if (!fp)
		   ok=false;

		if (ok)
		{
			// save qtree pdf 
			if (ok)
			   m_qtree->SaveHuffmanPDF(fp);

			// save dump pdf for each basis function 
			if (ok)
				for (i=0;i<MAX_COEFF;i++)
		   			m_coeff[i]->SaveHuffmanPDF(fp);

			fclose(fp);
                
		} 

	}
	else
		ok=false;

	return ok;

}

//////////////////////////////////////////////////////////////////
//			Quad-tree Functions									//
//////////////////////////////////////////////////////////////////

// LQDCT quadtree spliting function 
bool QtreeCodec::SplitBlock(Block *parent)
{
	bool ok=true;
    int size;
    Block *child,*last;

    // determine child size 
    size=parent->m_size/2;

    // now place children into block structures

    // Top Right 
	child=GetFreeBlock();
	if (!child)
	   ok=false;

	if (ok)
	{
		child->SetupBlock(parent->m_x+size,parent->m_y,size,parent);
		AppendBlockToList(parent,child);
		last=child;
	}

    // Bottom left 
	child=GetFreeBlock();
	if (!child)
	   ok=false;
	if (ok)
	{
		child->SetupBlock(parent->m_x,parent->m_y+size,size,parent);
		AppendBlockToList(last,child);
		last=child;
	}

    // Bottom Right 
	    // Bottom left 
	child=GetFreeBlock();
	if (!child)
	   ok=false;
	if (ok)
	{
		child->SetupBlock(parent->m_x+size,parent->m_y+size,size,parent);
		AppendBlockToList(last,child);

	}

    //Top left - reset data
	parent->m_useCurrentFrame=true;
    parent->m_jump+=(parent->m_size-size); 
    parent->m_size=size;
	parent->m_error=0;
    
	return ok;
}

//////////////////////////////////////////////////////////////////

void QtreeCodec::AppendBlockToList(Block *first,Block *second)
{
    Block *next;

    // place append block after block block 
    next=first->m_next;
    first->m_next=second;
    second->m_next=next;
 
}

//////////////////////////////////////////////////////////////////

void QtreeCodec::EncodeQtreeImage()
{
        int nitems;
        Block *block;
        
        nitems=0;
        block=m_top;
        while (block!=NULL){
              nitems++;
              block=block->m_next;
        }
  
        // encode qtree 
        block=m_top;
        while (block!=NULL)
		{
 
              if (block->m_size==m_size)
			  {
				 if (block->m_useCurrentFrame)
                    m_qtree->CompressSymbol(0);
                 else
					 m_qtree->CompressSymbol(2); // use last frame

				 block=block->m_next;
              }
              else {
                  m_qtree->CompressSymbol(1);
                  block=EncodeQtreeBlock(block,m_size/2);
              }
        }      
 
 
}

//////////////////////////////////////////////////////////////////
 
Block * QtreeCodec::EncodeQtreeBlock(Block *block,int size)
{
        int i;
 
        for (i=0;i<4;i++)
            if (block->m_size<size)
			{  // split
               m_qtree->CompressSymbol(1);
               block=EncodeQtreeBlock(block,size/2);
            }
            else 
			{	// code current block somehow
				if (block->m_useCurrentFrame)
                   m_qtree->CompressSymbol(0); // use intra frame
                else
					m_qtree->CompressSymbol(2); // use last frame
                block=block->m_next;
           }
              
        return block;
 
}

//////////////////////////////////////////////////////////////////
// Qtree decoders... 

bool QtreeCodec::DecodeQtreeImage(ImageData *image,ImageData *last)
{
	bool ok=true;
	int x,y;
	Block *block,*top;
 
	// allocate dummy block 
	top=new Block;
	block=top;
	x=0;
	y=0;
 
    for (;ok;) 
	{
        switch (m_qtree->UncompressSymbol())
		{
			case (0):
				block->m_next=GetFreeBlock();
				block->m_next->SetupBlock(x,y,m_size,image,last);
				block->m_next->m_useCurrentFrame=true;
				block=block->m_next;
				break;
			case(1):
				block=DecodeQtreeBlock(image,last,block,x,y,m_size/2);
				break;
			case(2):
				block->m_next=GetFreeBlock();
				block->m_next->SetupBlock(x,y,m_size,image,last);
				block->m_next->m_useCurrentFrame=false;
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
        if (x+m_size<image->m_width)
           x+=m_size;
        else 
		{
             x=0;
             y+=m_size;
        }
           
        if (y>=image->m_height)
		{
           block->m_next=NULL;
           break;
        }
          
    }

	// store tmp true top location
	if (ok)
       m_top=top->m_next;
    
	delete top;

	return ok;
}


//////////////////////////////////////////////////////////////////

Block *QtreeCodec::DecodeQtreeBlock
(ImageData *image,
 ImageData *last,
 Block *block,
 int x,
 int y,
 int size)
{
    int i,j;

    for (j=0;j<size*2;j+=size)
        for (i=0;i<size*2;i+=size)
			switch (m_qtree->UncompressSymbol())
			{
				case (0):
					block->m_next=GetFreeBlock();
					block->m_next->SetupBlock(x+i,y+j,size,image,last);
					block->m_next->m_useCurrentFrame=true;
					block=block->m_next;
					break;
				case(1):
					block=DecodeQtreeBlock(image,last,block,x+i,y+j,size/2);
					break;
				case(2):
					block->m_next=GetFreeBlock();
					block->m_next->SetupBlock(x+i,y+j,size,image,last);
					block->m_next->m_useCurrentFrame=false;
					block=block->m_next;
					break;
				default:
					return NULL;//panic

			}

    return block;
 
}

//////////////////////////////////////////////////////////////////
//		Misc block-image functions								//
//////////////////////////////////////////////////////////////////

void QtreeCodec::UnquantiseImage()
{
	Block *block;

	for (block=m_top;block!=NULL;block=block->m_next)
		if (block->m_useCurrentFrame)
		   block->UnquantiseBlock();

	return;
}

//////////////////////////////////////////////////////////////////

void QtreeCodec::RenderImage()
{
	Block *block;

	for (block=m_top;block!=NULL;block=block->m_next)
		if (block->m_useCurrentFrame)
		   block->RenderBlock();
		else
		    block->CopyLastFrame();

	return;
}

//////////////////////////////////////////////////////////////////
//				Free list managment functions					//
//////////////////////////////////////////////////////////////////


Block *QtreeCodec::GetFreeBlock()
{
	Block *block=NULL;

	if (!m_spareBlocks)
	{
		// make N new blocks
		int i,N=10;
		Block *last=NULL;
		
		for (i=0;i<N;i++)
		{
			block=new Block;
			if (!block)
			   return NULL;

			// make sure list is NULL terminated
			block->m_next=NULL;
			
			if (last)
			   last->m_next=block;
			else
				m_spareBlocks=block;

			// save last block
			last=block;
		}
	}

	// get block for return
	block=m_spareBlocks;
	// move spare list on
	m_spareBlocks=m_spareBlocks->m_next; 

	// null terminate new block
	block->m_next=NULL;
	block->m_sortNext=NULL;

	return block;

}

//////////////////////////////////////////////////////////////////

void QtreeCodec::FreeBlockList()
{
	// need to retrun all blocks in m_top
	// to m_spareblocks

	Block *block;

	while (m_top)
	{
		// save tmp of the top of the block list
		block=m_top;
		// move top of list on to next entery
		m_top=m_top->m_next;
		// make top of spare blocks list equal to tmp block
		block->m_next=m_spareBlocks;
		m_spareBlocks=block;
	}

	return;

}

//////////////////////////////////////////////////////////////////

void QtreeCodec::DeleteListManager()
{
	FreeAllBlocks(m_spareBlocks);
	m_spareBlocks=NULL;
	FreeAllBlocks(m_top);
	m_top=NULL;

}

//////////////////////////////////////////////////////////////////

void QtreeCodec::FreeAllBlocks(Block *top)
{

	Block *block,*next;
    block=top;
    
	while (block!=NULL)
	{
          next=block->m_next;
	      delete block;
	      /* move to next block*/
	      block=next;
    }
	return;
}

//////////////////////////////////////////////////////////////////
