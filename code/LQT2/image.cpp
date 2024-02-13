/*##############################################################*/
/*                      David Bethel                            */
/*                  Darak@compuserve.com				        */
/*					Image input/output							*/
/*##############################################################*/
#include <stdlib.h>
#include <string.h>
#include "Image.h"

/* macros */
#define limit(x) ((x) > 255 ? 255 : (x < 0 ? 0 :x))
#define limit32(x) ((x) > 31 ? 31 : (x < 0 ? 0 :x))
#define rint(x) (x > 0 ? (int)(x+0.5) : (int)(x-0.5) )

//////////////////////////////////////////////////////////////////
//		ImageData Functions										//
//////////////////////////////////////////////////////////////////

ImageData::ImageData ():
m_pt(NULL),
m_width(0),	
m_lPitch(0),
m_height(0)
{

}

//////////////////////////////////////////////////////////////////

ImageData::ImageData (int width,int height):
m_pt(NULL),
m_width(width),	
m_lPitch(width),
m_height(height)
{

	AllocateImageSpace();

}

//////////////////////////////////////////////////////////////////

ImageData::~ImageData()
{
	if (m_pt)
	   delete m_pt;
}

//////////////////////////////////////////////////////////////////

#define MAX_SIZE 32
bool ImageData::AllocateImageSpace()
{
	int width,height,i;

	/* rounds rows and columns to multiple of MAX_SIZE*/
	height=m_height;
	if (height%MAX_SIZE)
	   height+=(MAX_SIZE-height%MAX_SIZE);
	
	width=m_width;
	if (width%MAX_SIZE)
	   width+=(MAX_SIZE-width%MAX_SIZE);

	// save pitch of lines
	m_lPitch=width;

    if (m_pt!=NULL){
       delete m_pt;
	   m_pt=NULL;
	}

	m_pt=new int [width*height];

	if (!m_pt){
	   m_pt=NULL;
	   m_width=0;
	   m_lPitch=0;
	   m_height=0;
	   return false;
	}
	
	/* clear area */
	for (i=0;i<width*height;i++)
		m_pt[i]=0;

	return true;
}

//////////////////////////////////////////////////////////////
//		Image Functions										//
//////////////////////////////////////////////////////////////

Image::Image() 
{
	m_Y=NULL;
	m_U=NULL;
	m_V=NULL;
	m_fp=NULL;
	m_colour=0;
	m_width=0;
	m_height=0;
	m_imageIsYUV422=false;
	sprintf(m_fname,"new.pgm");

}

//////////////////////////////////////////////////////////////////

Image::~Image()
{
	if (m_Y)
	   delete m_Y;
	if (m_U)
	   delete m_U;
	if (m_V)
	   delete m_V;
}

//////////////////////////////////////////////////////////////////

bool Image::SetFileName(char *fileName)
{
	bool ok=true;

	// write over default file name
	if (fileName)
	   if (!strcpy(m_fname,fileName))
		  ok=false;

	return ok;
}

//////////////////////////////////////////////////////////////////

bool Image::ReadRawHeader(char *fname)
{

	char dummy[256];

	// write over default file name
	if (fname)
	   strcpy(m_fname,fname);

	// get file from disk 
	m_fp=fopen(m_fname,"rb");

	if (!m_fp)
	   return false;

    // remove P5/6 line 
    fgets(dummy,255,m_fp);

    //sets up colour 
    if (!strncmp(dummy,"P5",2))
       m_colour=0;
    else if (!strncmp(dummy,"P6",2))
             m_colour=1;
         else {
              //printf("file is not PGM/PPM (P5/6) file\n");
              return false;
         }
	//remove comment lines 
    do {
       fgets(dummy,255,m_fp);
	} while (strncmp(dummy,"#",1)==0);
 
    // read in image stats          
    sscanf(dummy,"%d %d",&m_width,&m_height);

	// remove 255 line 
    fgets(dummy,255,m_fp);

	return true;
}

//////////////////////////////////////////////////////////////////

ImageData * Image::NewImageData(ImageData *pt,int scale)
{
	int i;
	int width,height;
	
	// switchs image size
	if (scale){
		width=m_width/scale;
		height=m_height;
	}
	else {
		 width=m_width;
		 height=m_height;
	}

	// image data already exists ... check sizes
	if (pt){
		if (pt->m_width >= width && pt->m_height >=height){
		   pt->m_width=width;
		   pt->m_height=height;	
		   // clear area 
#ifdef OPTIMISE_SPEED
		   ZeroMemory(pt->m_pt,sizeof(int)*pt->m_lPitch*height);
#else  
		   for (i=0;i<pt->m_lPitch*height;i++)
		       pt->m_pt[i]=0;
#endif
		}
		else {
			 delete pt;
			 pt=new ImageData(width,height);
		}
	}
	else
	    pt=new ImageData(width,height);

	return pt;
}

//////////////////////////////////////////////////////////////////

bool Image::CreateYImage()
{
	bool ok=true;

	m_Y=NewImageData(m_Y);

	if (!m_Y)
	   ok=false;

	return ok;

}

//////////////////////////////////////////////////////////////////

bool Image::CreateYUVImage()
{
	bool ok=true;

	// Y
	m_Y=NewImageData(m_Y);
	if (!m_Y)
	   ok=false;

	// U
	if (ok){
	   if (m_imageIsYUV422)
		  m_U=NewImageData(m_U,2);
	   else
		   m_U=NewImageData(m_U);
	}
	if (!m_U)
	   ok=false;

	// V
	if (ok){
	   if (m_imageIsYUV422)
	      m_V=NewImageData(m_V,2);
	   else
	       m_V=NewImageData(m_V);
	}
	if (!m_V)
	   ok=false;

	return ok;

}

//////////////////////////////////////////////////////////////////

bool Image::GetPPMFile()
{
	int i,j;
	unsigned char tmp[3];
    double RGB[3];
    double YUV[3];

	// check image is colour
	if (!m_colour)
	   return false;

	// create new images
	if (!CreateYUVImage())
	   return false;

	// read in image data -> file is 3 times as large as image (RGB)
	for (j=0;j<m_height;j++)
        for (i=0;i<m_width;i++){
            // read in Red / Blue / Green 
	        fread(tmp,sizeof(unsigned char),3,m_fp);
    
			RGB[0]=(double)tmp[0];	// R
			RGB[1]=(double)tmp[1];	// G
			RGB[2]=(double)tmp[2];	// B
		
			RGBtoYUV(RGB,YUV);

            // load into image files 
            m_Y->m_pt[m_Y->m_lPitch*j+i]=limit((int)rint(YUV[0])); //Y
            if (m_imageIsYUV422){
			   if (!(i%2)){
			      m_U->m_pt[m_U->m_lPitch*j+(i/2)]=limit((int)rint(YUV[1]+128));  //U
                  m_V->m_pt[m_V->m_lPitch*j+(i/2)]=limit((int)rint(YUV[2]+128));  //V
			   }
			}
			else { 
				 m_U->m_pt[m_U->m_lPitch*j+i]=limit((int)rint(YUV[1]+128));  //U
                 m_V->m_pt[m_V->m_lPitch*j+i]=limit((int)rint(YUV[2]+128));  //V
			}
		}
 
	fclose(m_fp);
	return true;
}

//////////////////////////////////////////////////////////////////

void Image::RGBtoYUV(double *RGB, double *YUV)
{
 
    // convert to YUV 
	//  		R				G				B
    YUV[0]= 0.2990*RGB[0]+0.5870*RGB[1]+0.1140*RGB[2]; // Y
    YUV[1]=-0.1686*RGB[0]-0.3311*RGB[1]+0.4997*RGB[2]; // U
    YUV[2]= 0.4998*RGB[0]-0.4185*RGB[1]-0.0813*RGB[2]; // V

}

//////////////////////////////////////////////////////////////////

bool Image::WritePPMFile(char *fname)
{
 
    double RGB[3];
    double YUV[3];
	int i,j,cnt;
	unsigned char tmp[3];
         
	// write over default file name
	if (fname)
	   sprintf(m_fname,"%s",fname);

    m_fp=fopen(m_fname,"wb");
	if (!m_fp)
	   return false;

    // write image stats + comments 
    fputs("P6",m_fp);
    fprintf(m_fp,"\n# Created by Image.cpp (daveb 97)#");
    fprintf(m_fp,"\n%d %d",m_width,m_height);
    fprintf(m_fp,"\n255\n");  /* 255 graylevels */
 
    // write image data 
	cnt=0;
    for (j=0;j<m_height;j++)
        for (i=0;i<m_width;i++){
            // load up Y/U/V 
            YUV[0]=(double)m_Y->m_pt[m_Y->m_lPitch*j+i];	   //Y
		    if (m_imageIsYUV422){
			   if (!(i%2)){
   				  YUV[1]=(double)(m_U->m_pt[m_U->m_lPitch*j+(i/2)]-128); //U
                  YUV[2]=(double)(m_V->m_pt[m_V->m_lPitch*j+(i/2)]-128); //V
			   }
			}
			else { 
				 YUV[1]=(double)(m_U->m_pt[m_U->m_lPitch*j+i]-128); //U
                 YUV[2]=(double)(m_V->m_pt[m_V->m_lPitch*j+i]-128); //V
			}

			YUVtoRGB(YUV,RGB);

            // save to file 
            tmp[0]=(unsigned char)limit(rint(RGB[0])); //R
            tmp[1]=(unsigned char)limit(rint(RGB[1])); //G
            tmp[2]=(unsigned char)limit(rint(RGB[2])); //B

		    fwrite(tmp,sizeof(unsigned char),3,m_fp);
        }
 
	fclose(m_fp);

	return true;

}

//////////////////////////////////////////////////////////////////

void Image::YUVtoRGB(double *YUV,double *RGB)
{
	// convert to RGB 
    RGB[0]=YUV[0]+1.402581*YUV[2];				    // R
    RGB[1]=YUV[0]-0.344369*YUV[1]-0.714407*YUV[2];	// G
    RGB[2]=YUV[0]+1.773043*YUV[1];				    // B

}

//////////////////////////////////////////////////////////////////

bool Image::GetPGMFile()
{

    int i,j;
	int *pt;


	if (m_colour)
	   return false;

	// create space for image
	if (!CreateYImage())
	   return false;
	
	/* read in image data */
    pt=m_Y->m_pt;
	for (j=0;j<m_height;j++,pt+=(m_Y->m_lPitch-m_width))
		for (i=0;i<m_width;i++)
            *pt++=(int)getc(m_fp);
 
	fclose (m_fp);

	return true;

}

//////////////////////////////////////////////////////////////////

bool Image::WritePGMFile(char *fname)
{
	int i,j,*pt;
 
	// write over default file name
	if (fname)
	   sprintf(m_fname,"%s",fname);

    m_fp=fopen(m_fname,"wb");
	if (!m_fp)
	   return false;

    // write image stats + comments 
    fputs("P5",m_fp);
    fprintf(m_fp,"\n# Created by Image.cpp daveb 97#");
    fprintf(m_fp,"\n%d %d",m_width,m_height);
    fprintf(m_fp,"\n255\n");    /* 255 graylevels */
 
    //write image data 
	pt=m_Y->m_pt;
	for (j=0;j<m_height;j++,pt+=(m_Y->m_lPitch-m_width))
		for (i=0;i<m_width;i++){
			*pt=limit(*pt);
            putc((unsigned char)(*pt++),m_fp);
		}
    fclose(m_fp);

	return true;
}

//////////////////////////////////////////////////////////////////

bool Image::ConvertYUV422ChunkToImage(unsigned char *pt,int width,int height)
{
	bool ok=true;
	int i,j;

	if (!pt)
	   ok=false;

	if (width && height){
       m_width=width;
	   m_height=height;
	}
	else
		ok=false;

	if (ok){
	   m_imageIsYUV422=true;
	   m_colour=1;
	}

	// create new images
	if (ok)
	   ok=CreateYUVImage();

	if (ok){
	   for (j=0;j<m_height;j++)
           for (i=0;i<m_width;i+=2){
               // load into image files 
			   m_Y->m_pt[m_Y->m_lPitch*j+i]=*pt++;      //Y0 
			   m_U->m_pt[m_U->m_lPitch*j+(i/2)]=*pt++;  //U
			   m_Y->m_pt[m_Y->m_lPitch*j+i+1]=*pt++;    //Y1
   			   m_V->m_pt[m_V->m_lPitch*j+(i/2)]=*pt++;  //V
            
           }
	}


	return ok;
}

//////////////////////////////////////////////////////////////////

bool Image::ConvertYUV422ChunkToImage
(unsigned char *pt,
 int x_start,
 int y_start,
 int width,
 int height,
 int scale) // 1= normal 2 = half
{
	bool ok=true;
	int i,j;


	if (!pt)
	   ok=false;

	if (scale!=1 && scale!=2)
	   ok=false;

	if (width && height&&ok){
       width/=scale;
	   height/=scale;
	}
	else
		ok=false;

	if (ok){
	   m_imageIsYUV422=true;
	   m_colour=1;
	}

	// create new images
	if (!m_Y || !m_U || !m_V)
	   ok=false;

	if (ok){
	   for (j=y_start;j<y_start+height;j++)
	   {
		   for (i=x_start;i<x_start+width;i+=2)
		   {
			   // load into image files 
			   if (scale==1)
			   {
					m_Y->m_pt[m_Y->m_lPitch*j+i]=*pt++;      //Y0 
					m_U->m_pt[m_U->m_lPitch*j+(i/2)]=*pt++;  //U
					m_Y->m_pt[m_Y->m_lPitch*j+i+1]=*pt++;    //Y1
   					m_V->m_pt[m_V->m_lPitch*j+(i/2)]=*pt++;  //V
			   }
			   if (scale==2)
			   {
				  m_Y->m_pt[m_Y->m_lPitch*j+i]=*pt++;      //Y0 
				  m_U->m_pt[m_U->m_lPitch*j+(i/2)]=*pt++;  //U
			      pt++;    
   				  m_V->m_pt[m_V->m_lPitch*j+(i/2)]=*pt++;  //V
				  m_Y->m_pt[m_Y->m_lPitch*j+i+1]=*pt++;    //Y1
				  pt+=3;
			   }
		   }
		   if (scale==2)
			  pt+=width*4;
	   }
	}


	return ok;
}

//////////////////////////////////////////////////////////////////

#ifdef DIRECTX

// directX image translators in 24 bits ONLY!
bool Image::ReadDDSurface(CDXSurface *surface)
{
	bool ok=true;
	DWORD i,j;
	double RGB[3],YUV[3];
	DWORD lPitch;

	// lock directX surface for use
	surface->Lock();

	BYTE *Bitmap = (BYTE*)surface->m_DDSD.lpSurface;

	if (!Bitmap)
	   ok=false;

	if (ok){
	   m_height=(int)(surface->m_DDSD.dwHeight);
	   m_width=(int)(surface->m_DDSD.dwWidth);

	   m_Y=NewImageData(m_Y);
	   m_U=NewImageData(m_U);
	   m_V=NewImageData(m_V);
	}

	// new failor
	if (!(m_Y || m_U || m_V))
	   ok=false;

	// Load data from surface
	if (ok){
	   lPitch=surface->m_DDSD.lPitch;
	   for (j=0;j<(surface->m_DDSD.dwHeight);j++)
	       for (i=0;i<(surface->m_DDSD.dwWidth*3);i+=3){
			   RGB[2]=(double)Bitmap[j*lPitch+i];
			   RGB[1]=(double)Bitmap[j*lPitch+i+1];
			   RGB[0]=(double)Bitmap[j*lPitch+i+2];

			   RGBtoYUV(RGB,YUV);

			   m_Y->m_pt[m_Y->m_lPitch*j+i/3]=(int)YUV[0];
			   m_U->m_pt[m_U->m_lPitch*j+i/3]=(int)YUV[1]+128;
			   m_V->m_pt[m_V->m_lPitch*j+i/3]=(int)YUV[2]+128;
		   }
	}
	// restore surface 
	surface->UnLock();

	return ok;

}

//////////////////////////////////////////////////////////////////

bool Image::WriteDDSurface(CDXSurface *surface)
{
	bool ok=true;
	DWORD i,j;
	int RGB[3],YUV[3];
	DWORD lPitch,width,height;
	int r,g,b;
	int *Yptr,*Uptr,*Vptr;
	long offset=0;

	// lock directX surface for use
	surface->Lock();
	
	BYTE *Bitmap = (BYTE*)surface->m_DDSD.lpSurface;

	if (!Bitmap)
	   ok=false;

	// new failor
	if (!(m_Y || m_U || m_V))
	   ok=false;
	
	// Load data from surface
	if (ok){
	   lPitch=surface->m_DDSD.lPitch;
	   if ((DWORD)m_width<surface->m_DDSD.dwWidth)
	      width=(DWORD)m_width;
	   else
		   width=surface->m_DDSD.dwWidth;
	
	   if ((DWORD)m_height<surface->m_DDSD.dwHeight)
	      height=(DWORD)m_height;
	   else
		   height=surface->m_DDSD.dwHeight;
	}

	Yptr=m_Y->m_pt;
	Uptr=m_U->m_pt;
	Vptr=m_V->m_pt;

	if (ok)
	   for (j=0;j<height;j++,offset+=(lPitch-2*width))
		   for (i=0;i<width*3;i+=3){
			
			   YUV[0]=(*Yptr++);

			   if (m_imageIsYUV422){
			      if (!(i%2)){
			         YUV[1]=(*Uptr++)-128;
			         YUV[2]=(*Vptr++)-128;
				  }
			   }
			   else {
			         YUV[1]=(*Uptr++)-128;
			         YUV[2]=(*Vptr++)-128;
			   }
			   YUVtoRGBFast(YUV,RGB);

//			   Bitmap[j*lPitch+i]=(BYTE)limit(rint(RGB[2]));
//			   Bitmap[j*lPitch+i+1]=(BYTE)limit(rint(RGB[1]));
//			   Bitmap[j*lPitch+i+2]=(BYTE)limit(rint(RGB[0]));

			   b=(BYTE)limit32(RGB[2]);
			   g=(BYTE)limit32(RGB[1]);
			   r=(BYTE)limit32(RGB[0]);


			   Bitmap[offset]=g & 7;
				Bitmap[offset]=Bitmap[offset] <<5;
				Bitmap[offset]+=b;

				Bitmap[offset + 1]=r << 2;
				Bitmap[offset + 1]=Bitmap[offset + 1] + ((g & 24) >> 3);
				offset+=2;

		   }
	 
	// unlock surface 
	surface->UnLock();

	return ok;
}

//////////////////////////////////////////////////////////////////

void Image::YUVtoRGBFast(int *YUV,int *RGB)
{
	// convert to RGB 
    RGB[0]=((YUV[0]<<8)+359*YUV[2])>>11;				    // R
    RGB[1]=((YUV[0]<<8)-88*YUV[1]-183*YUV[2]) >>11;			// G
    RGB[2]=((YUV[0]<<7)+YUV[1]*227) >> 10;				    // B

}

//////////////////////////////////////////////////////////////////

bool Image::WriteDDSurfaceSlow(CDXSurface *surface)
{
	bool ok=true;
	DWORD i,j;
    double RGB[3],YUV[3];
	DWORD lPitch,width,height;
	int r,g,b;
	int *Yptr,*Uptr,*Vptr;
	long offset=0;

	// lock directX surface for use
	surface->Lock();
	
	BYTE *Bitmap = (BYTE*)surface->m_DDSD.lpSurface;

	if (!Bitmap)
	   ok=false;

	// new failor
	if (!(m_Y || m_U || m_V))
	   ok=false;
	
	// Load data from surface
	if (ok){
	   lPitch=surface->m_DDSD.lPitch;
	   if ((DWORD)m_width<surface->m_DDSD.dwWidth)
	      width=(DWORD)m_width;
	   else
		   width=surface->m_DDSD.dwWidth;
	
	   if ((DWORD)m_height<surface->m_DDSD.dwHeight)
	      height=(DWORD)m_height;
	   else
		   height=surface->m_DDSD.dwHeight;
	}

	Yptr=m_Y->m_pt;
	Uptr=m_U->m_pt;
	Vptr=m_V->m_pt;

	if (ok)
	   for (j=0;j<height;j++,offset+=(lPitch-2*width))
		   for (i=0;i<width*3;i+=3)
		   {
			   YUV[0]=(double)(*Yptr++);
			   if (m_imageIsYUV422){
			      if (!(i%2)){
				     YUV[1]=(double)((*Uptr++)-128);
		             YUV[2]=(double)((*Vptr++)-128);
				  }
			   }
			   else {
			         YUV[1]=(double)((*Uptr++)-128);
			         YUV[2]=(double)((*Vptr++)-128);
			   }
			   YUVtoRGB(YUV,RGB);

			   b=(BYTE)limit(rint(RGB[2]));
			   g=(BYTE)limit(rint(RGB[1]));
			   r=(BYTE)limit(rint(RGB[0]));

			   b=b>>3;
	           g=g>>3;
	           r=r>>3;

			   Bitmap[offset]=g & 7;
				Bitmap[offset]=Bitmap[offset] <<5;
				Bitmap[offset]+=b;

				Bitmap[offset + 1]=r << 2;
				Bitmap[offset + 1]=Bitmap[offset + 1] + ((g & 24) >> 3);
				offset+=2;

		   }
	 
	// unlock surface 
	surface->UnLock();

	return ok;
}

//////////////////////////////////////////////////////////////////



#endif
