/*##############################################*/
/*				David Bethel					*/
/*		      Darak@compuserve.com				*/
/*				 LQTDCT Codec					*/
/*##############################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "image.h"
#include "codec.h"

//////////////////////////////////////////////////////////////////

Codec::Codec():
m_BPP((float)0.8),
m_fp(NULL),
m_writeResultToFile(true),
m_noUpdate(true)
{
	m_compressed= new IOBuff;
	m_image=new Image;
	m_last=new Image;
}

//////////////////////////////////////////////////////////////////

Codec::~Codec()
{
	delete m_compressed;
	delete m_image;
	delete m_last;
}

//////////////////////////////////////////////////////////////////

bool Codec::SetFileNames(char *inputFname,char *outputFname)
{
	bool ok=true;

	if (inputFname)
	   if (!strcpy(m_inputFname,inputFname))
	      ok=false;

	if (outputFname)
	   if (!strcpy(m_outputFname,outputFname))
	      ok=false;

	return ok;
}

//////////////////////////////////////////////////////////////////

void Codec::SetCompression(float bpp)
{
	m_BPP=bpp;
}


//////////////////////////////////////////////////////////////////

bool Codec::CompressImageFromFile()
{
	bool ok=true;

	// load head of PGM/PPM file
	ok=m_image->SetFileName(m_inputFname);
	
	if (ok)
	   ok=m_image->ReadRawHeader();
	
	// load main body of file
	if (ok){
	   if (m_image->m_colour)
	      ok=m_image->GetPPMFile();
	   else
		   ok=m_image->GetPGMFile();
	}

	// file for destination
	
	if (ok)
	   ok=CompressImage();

	return ok;

}

//////////////////////////////////////////////////////////////////

bool Codec::CompressImage()
{
	bool ok=true;
	float bitsY(0.0),bitsV(0.0),bitsU(0.0);

	if ((m_image->m_width)<32 || (m_image->m_height)<32)
	{
       //   printf("Image too small to compress\n");
	   ok=false;
	}

	//rewind buffer - need to be set up before using comopressors 
	m_compressed->Rewind();

	if (ok)
       ok=WriteCompressedFileHeader();

	// move stream into bit mode
	m_compressed->BitMode();

	// work out bits availible 
	bitsY=(float)(m_image->m_height*m_image->m_width)*m_BPP;
 
	// set colour compression strengths 
	if (m_image->m_colour){
       bitsU=(float)0.075*(float)bitsY; // 15% colour 
	   bitsV=(float)0.075*(float)bitsY;
       bitsY-=(bitsU+bitsV);     // keep totals bits same 
	}
 	
	if (ok)
	  ok=CompressY(m_image->m_Y,m_last->m_Y,(int)bitsY);
	
	if (ok&&m_image->m_colour)
	   ok=CompressY(m_image->m_U,m_last->m_U,(int)bitsU);

	if (ok&&m_image->m_colour)
	   ok=CompressY(m_image->m_V,m_last->m_V,(int)bitsV);

	// termate buffer
	m_compressed->TerminateBitWrite();

	// only right file to disk if enabled
	if (!m_writeResultToFile)
	   return ok;

	if (!m_outputFname)
	   ok=false;

	// open file and save it
	if (ok){
	   m_fp=fopen(m_outputFname,"wb");
	   if (m_fp){
		  ok=m_compressed->WriteBufferToFile(m_fp);
	      fclose(m_fp);
	      m_fp=NULL;
	   }
	   else
		   ok=false;
	}
	return ok;
}

//////////////////////////////////////////////////////////////////

bool Codec::DecompressImageFromFile()

{
	bool ok=true;

	// file for destination
	if (!m_inputFname)
	   ok=false;			// no file name

	if (ok)
	{
	   m_fp=fopen(m_inputFname,"rb");

	   if (m_fp)
	   {
	      // buffer MUST have been setup already and be big enough
	      ok=m_compressed->ReadBufferFromFile(m_fp);
		  fclose(m_fp);
	      m_fp=NULL;
	   }
	   else
		   ok=false;
	}
	   // decompress data
    if (ok)
	   ok=DecompressImage();
	
	return ok;

}

//////////////////////////////////////////////////////////////////
// must have image size setup before entering

bool Codec::DecompressImage()
{
	bool ok=true;
	Image *image;

	//rewind buufer 
	m_compressed->Rewind();

	// read header data
    ok=ReadCompressedFileHeader();
	// move into bit mode
	m_compressed->BitMode();

	if (ok)
	{
	   if (m_image->m_colour)
	      ok=m_image->CreateYUVImage();
	   else
	   	   ok=m_image->CreateYImage();
	}

	if (ok)
	  ok=DecompressY(m_image->m_Y,m_last->m_Y);
	 
	if (ok&&m_image->m_colour)
	   ok=DecompressY(m_image->m_U,m_last->m_U);

	if (ok&&m_image->m_colour)
	   ok=DecompressY(m_image->m_V,m_last->m_V);
	
	// swap last image with cuurent one
	image=m_image;
	if (ok &&!m_noUpdate)
	{
		m_image=m_last;
		m_last=image;
	}
	// this is not an error it just Wrtie image to file
	if (!m_writeResultToFile)
	   return ok;

	// change image file name to output destination
	if (ok)
	{
	   ok=image->SetFileName(m_outputFname);
	}

	// write out image
	if (ok)
	{
	   if (image->m_colour)
	      ok=image->WritePPMFile();
	   else
		   ok=image->WritePGMFile();
	}

	return ok;
}

//////////////////////////////////////////////////////////////////
//	Theses should be in qtree but they will be here a while -   //
//  until i sort stuff											//
//	they should be virtal but later....							//
//////////////////////////////////////////////////////////////////

bool Codec::WriteCompressedFileHeader()
{
	bool ok=true;
	char s[256];
    /* write image stats + comments */
	if (ok)
	   ok=m_compressed->WriteStringToBuffer("LDCT\n");
	if (ok)
	   ok=m_compressed->WriteStringToBuffer("# DCT compressor\n");
	if (ok)
	{
	   sprintf(s,"%d %d\n",m_image->m_width,m_image->m_height);
	   ok=m_compressed->WriteStringToBuffer(s);
    }
	if (ok)
	{
	   sprintf(s,"%d %d\n",m_image->m_colour,(int)(m_image->m_imageIsYUV422));
	   ok=m_compressed->WriteStringToBuffer(s);
	}

	return ok;
}

//////////////////////////////////////////////////////////////////

bool Codec::ReadCompressedFileHeader()
{
	bool ok=true;
	int type;
    char dummy[256];
 
	ok=m_compressed->ReadToEOLInBuffer(dummy,255);

    // sets up colour 
	if (ok)
       if (strncmp(dummy,"LDCT",4)){
          //printf("file is not " FILE_TYPE " compressed file type\n");
          ok=false;
       }
 
    // remove comment lines 
	if (ok)
       do {
	      ok=m_compressed->ReadToEOLInBuffer(dummy,255);
	   } while (strncmp(dummy,"#",1)==0 && ok);

    // read in image stats         
    if (ok)
	   sscanf(dummy,"%d %d",&(m_image->m_width),&(m_image->m_height));
    if (ok)
	   ok=m_compressed->ReadToEOLInBuffer(dummy,255);
    if (ok)
	   sscanf(dummy,"%d %d",&(m_image->m_colour),&type);

	if (type)
	   m_image->m_imageIsYUV422=true;
	else
		m_image->m_imageIsYUV422=false;
 
	return ok;
}

//////////////////////////////////////////////////////////////////
