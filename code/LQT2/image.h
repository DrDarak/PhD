/* Image input/ouput functions */
#ifndef H_IMAGE_H
#define H_IMAGE_H

#include <stdio.h>
// must define DIRECTX in preproceeor to get IMAGE/DIRECTX
// processes to work
#ifdef DIRECTX
       #include <CDX.h>
#endif

#ifdef OPTIMISE_SPEED
    #include <windows.h>
	#include <winbase.h>
#endif

class ImageData {
public:
	ImageData ();
	ImageData (int width,int height);
	~ImageData();

	bool AllocateImageSpace();

	int *m_pt;        /* image data  */
	int m_width;      /* image width */
	int m_lPitch;	  // length of raster line 
	int m_height;     /* image height */

} ;

class Image {
public:
    Image();
	~Image();
//	bool ReadFile(char *fname=NULL);
//	bool WriteFile(char *fname=NULL);
//  private:

	ImageData * NewImageData(ImageData *pt,int scale=0);
	bool GetPPMFile();
	bool WritePPMFile(char *fname=NULL);
	bool GetPGMFile();
	bool WritePGMFile(char *fname=NULL);
	bool ReadRawHeader(char *fname=NULL); 
    void RGBtoYUV(double *RGB, double *YUV);
	void YUVtoRGB(double *YUV,double *RGB);
	bool SetFileName(char *);
	bool CreateYUVImage();
	bool CreateYImage();
	bool ConvertYUV422ChunkToImage(unsigned char *pt,int width,int height);
	bool ConvertYUV422ChunkToImage(unsigned char *pt,int start_x,int start_y,
									int width,int height,int scale);
#ifdef DIRECTX
	bool	ReadDDSurface(CDXSurface *surface);
	bool	WriteDDSurface(CDXSurface *surface); 
	bool    WriteDDSurfaceSlow(CDXSurface *surface);
	void    YUVtoRGBFast(int *YUV,int *RGB);
#endif
	/* Data */
	FILE *m_fp;
	char m_fname[256];
	int m_colour;

	ImageData *m_Y;
	ImageData *m_U;
	ImageData *m_V;

	bool m_imageIsYUV422; // Flag to test if image is YUV
						  // or YUV422 - default false

	/* may not be same for all images */
	int m_width;    /* image width */
    int m_height;   /* image height */

} ;


#endif  /* H_IMAGE_IO_H*/
