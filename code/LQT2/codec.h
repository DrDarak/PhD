#ifndef H_CODEC_H
#define H_CODEC_H


///////////////////////////////////////////////////////////////
//					Primary codec header					 //
//						David Bethel						 //
//						October 97							 //
//					Darak@compuserve.com					 //
//															 //
//Defines:	DIRECTX for use with directx apps				 //
//			OPTIMISE_SPEED install speed optimisations... may//
//						   cause some problems				 //
///////////////////////////////////////////////////////////////

#include "iobuff.h"
#include "image.h"

class Codec {
public:

	Codec();
	~Codec();

	void SetCompression(float bpp);
	bool SetFileNames(char *inputFname,char *outputFname);

	bool CompressImageFromFile();
	bool CompressImage();
	virtual	bool CompressY(ImageData *image,ImageData *last,int bits)=NULL;

	
	bool DecompressImageFromFile();
	bool DecompressImage();
	virtual bool DecompressY(ImageData *image,ImageData *last)=NULL;

	bool ReadCompressedFileHeader();
	bool WriteCompressedFileHeader();

    Image *m_image;
	Image *m_last;

	char m_inputFname[256];
	char m_outputFname[256];

	float m_BPP;
	int m_noRuns; // number of times the compressor has been run
	bool m_writeResultToFile;
	bool m_noUpdate; // usually true


	IOBuff *m_compressed; // store of compressed data
    
	FILE *m_fp; //output file (needs doing in IOBUFF)

};

#endif