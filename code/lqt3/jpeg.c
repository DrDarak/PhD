/////////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////////

#include "codec.h"

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	JPEG Tables
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

unsigned char bitsDCLuminance[17] =
{ 0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 }; // base 0
unsigned char  valDCLuminance[12] =
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

////////////////////////////////////////////////////////////////////////////////

unsigned char  bitsACLuminance[17] =
{ 0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };// base 0
unsigned char  valACLuminance[162] =
{ 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
  0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
  0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
  0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
  0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
  0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
  0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
  0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
  0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
  0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
  0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
  0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
  0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
  0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
  0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
  0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
  0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
  0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
  0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
  0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
  0xf9, 0xfa };

////////////////////////////////////////////////////////////////////////////////

unsigned char  bitsDCChrominance[17] =
{ 0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 }; // 0-base
unsigned char  valDCChrominance[12] =
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

////////////////////////////////////////////////////////////////////////////////

unsigned char  bitsACChrominance[17] =
{  0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 }; // 0-base 
unsigned char  valACChrominance[162] =
{ 0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
  0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
  0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
  0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
  0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
  0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
  0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
  0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
  0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
  0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
  0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
  0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
  0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
  0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
  0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
  0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
  0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
  0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
  0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
  0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
  0xf9, 0xfa };

/////////////////////////////////////////////////////////////////////////////////////////////

const unsigned char ZAG[65] = {
  0, 1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63, 64 
};

/////////////////////////////////////////////////////////////////////////////////////////////

static const int ZIG[DCTSIZE2] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

/////////////////////////////////////////////////////////////////////////////////////////////
// This is the sample quantization table given in the JPEG spec section K.1,
// but expressed in zigzag order (as are all of our quant. tables).
// The spec says that the values given produce "good" quality, and
// when divided by 2, "very good" quality.
  
const unsigned int stdYQuant[DCTSIZE2] = 
{
    16,  11,  12,  14,  12,  10,  16,  14,
    13,  14,  18,  17,  16,  19,  24,  40,
    26,  24,  22,  22,  24,  49,  35,  37,
    29,  40,  58,  51,  61,  60,  57,  51,
    56,  55,  64,  72,  92,  78,  64,  68,
    87,  69,  55,  56,  80, 109,  81,  87,
    95,  98, 103, 104, 103,  62,  77, 113,
    121, 112, 100, 120,  92, 101, 103,  99
};

/////////////////////////////////////////////////////////////////////////////////////////////

const unsigned int stdUVQuant[DCTSIZE2] = 
{
    17,  18,  18,  24,  21,  24,  47,  26,
    26,  47,  99,  66,  56,  66,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99
};

/////////////////////////////////////////////////////////////////////////////////////////////

void QuantQualityScale(int quality,int *quant)
{
	int temp,i;
	// Safety limit on quality factor.  Convert 0 to 1 to avoid zero divide. 
	if (quality <= 0) quality = 1;
	if (quality > 100) quality = 100;
	
	if (quality < 50)
		quality = 5000 / quality;
	else
		quality = 200 - quality*2;

	for (i = 0; i < DCTSIZE2; i++) 
	{
		temp = (quant[i] * quality + 50) / 100L;
		if (temp <= 0L) temp = 1L;
		if (temp > 255L) temp = 255L;
		quant[i]=temp;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//		Maintain JpegInfo
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

JpegInfo *CreateJpegInfo()
{
	JpegInfo *jpeg=NULL;
	jpeg=(JpegInfo *)malloc(sizeof(JpegInfo));
	memset(jpeg,0,sizeof(JpegInfo));
	return jpeg;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LoadJpeg(JpegInfo *jpeg,char *fileName)
{
#ifdef WIN32
	DWORD read;
	HANDLE hFile;
	// Save alarm frame
	hFile=CreateFile(fileName,
					GENERIC_READ,
					0,
					NULL,
					OPEN_ALWAYS,
					0,
					NULL);

	if (hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD size=GetFileSize(hFile,NULL);
		jpeg->m_packPtr=jpeg->m_topPtr;
		jpeg->m_dataSize=size;
		ReadFile(hFile,jpeg->m_topPtr,size,&read,NULL);
		CloseHandle(hFile);
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool JpegClear(JpegInfo *jpeg)
{
	//clear all but leave  data
	jpeg->m_packPtr=jpeg->m_topPtr;
	jpeg->m_bits=0;	  	
	jpeg->m_activeBytes=0;	  
	jpeg->m_lines=0;
	jpeg->m_samplesPerLine=0;
	memset(&jpeg->m_Y,0,sizeof(JpegPlaneInfo));
	memset(&jpeg->m_U,0,sizeof(JpegPlaneInfo));
	memset(&jpeg->m_V,0,sizeof(JpegPlaneInfo));
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void NewVideoData(JpegInfo *jpeg,VideoData **video)
{
	int i;
	VideoData *clear=NULL;
	if (*video)
	{	
		if ((*video)->m_width!=jpeg->m_samplesPerLine ||
			(*video)->m_height!=jpeg->m_lines)
		{
			clear=*video;
			*video=NULL;
		}
	}	

	if (!*video)
	{
		*video=VideoData_Create(jpeg->m_samplesPerLine,jpeg->m_samplesPerLine,
							jpeg->m_lines,false,false);
		if (clear)
		{
			// copy stuff from clear over
			(*video)->m_camera=clear->m_camera;
			(*video)->m_time=clear->m_time;
			(*video)->m_hasTextTime=clear->m_hasTextTime;
			(*video)->m_timeX=clear->m_timeX;
			(*video)->m_timeY=clear->m_timeY;
			(*video)->m_hasTextDate=clear->m_hasTextDate;
			(*video)->m_dateX=clear->m_dateX;
			(*video)->m_dateY=clear->m_dateY;
			(*video)->m_hasText=clear->m_hasText;
			for (i=0;i<MAX_VIDEO_TEXT;i++)
			{
				(*video)->m_x[i]=clear->m_x[i];
				(*video)->m_y[i]=clear->m_y[i];
				memcpy((*video)->m_text[i],clear->m_text[i],64);
			}
			VideoData_Destroy(&clear);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
#define IABS(x) ( (x) <0 ? -(x) :(x)) 
#define signExtend11(x) ((x)&0x400 ? ((x) | 0xfffff800) : ((x) & 0x7ff))
#define signExtend12(x) ((x)&0x800 ? ((x) | 0xfffff000) : ((x) & 0xfff))

unsigned int CreateHuffmanTables
(unsigned int *oHuffCode,
 unsigned char *bits,
 unsigned char *val,
 bool compressor,
 bool dc)
{
	int p,i,j,l,lastp,si;
	char size[257],huffSize[257];
	unsigned int huffCode[257],code[257];
	unsigned int hCode,hSize,EOB=0;
	int r,tmpCode,tmpCode2,nbits;


	// Figure C.1: make table of Huffman code length for each symbol 
	// Note that this is in code-length order. 

	p = 0;
	for (l = 1; l <= 16; l++)
	{
		for (i = 1; i <= (int) bits[l]; i++)
			huffSize[p++] = (char) l;
	}
	huffSize[p] = 0;
	lastp = p;

	// Figure C.2: generate the codes themselves 
	// Note that this is in code-length order. 

	hCode = 0;
	si = huffSize[0];
	p = 0;
	while (huffSize[p]) 
	{
		while (((int) huffSize[p]) == si) 
		{
			huffCode[p++] = hCode &((1<<si)-1);	// ensure data is si bits
			hCode++;
		}
		hCode <<= 1;
		si++;
	}

	// Figure C.3: generate encoding tables 
	// These are code and size indexed by symbol value 

	if (compressor)
	{
		for (p = 0; p < lastp; p++) 
		{
			code[val[p]] = huffCode[p];
			size[val[p]] = huffSize[p];
		}

		// form up the decode tables for dc and ac codes
		if (dc)
		{
			memset(oHuffCode,0,sizeof(unsigned int)*CODE_SIZE);
			for (i=0;i<CODE_SIZE/16;i++)
			{
				tmpCode2=signExtend12(i&0xfff);
				tmpCode=IABS(tmpCode2);
				tmpCode2=tmpCode2-(tmpCode2<0);
				if (!tmpCode)
				{
					hSize=size[tmpCode];
					oHuffCode[i]= hSize & 0x1f;
					oHuffCode[i]|= (code[tmpCode] <<(32-hSize));
					continue;
				}

				nbits = 1;		// there must be at least one 1 bit 
				do 
				{
					if (tmpCode>>=1)
						nbits++;
				} while (tmpCode);		
				tmpCode2&=((1<<nbits)-1);
				hSize=size[nbits] + nbits;

				oHuffCode[i]= hSize & 0x1f;
				oHuffCode[i]|= ((code[nbits] << nbits) | tmpCode2)<< (32-hSize);
			}
		}
		else
		{
			EOB=size[0] & 0x1f;
			EOB|=(code[0] << (32-size[0])) ;
			memset(oHuffCode,0,sizeof(unsigned int)*CODE_SIZE);
			for (i=1;i<CODE_SIZE/2;i++)
			{
				tmpCode2=signExtend11(i&0x7ff);
				tmpCode=IABS(tmpCode2);
				tmpCode2=tmpCode2-(tmpCode2<0);
				r=i>>11;

				if (!tmpCode)
				{
					tmpCode=(r<<4);
					hSize=size[tmpCode];
					oHuffCode[i]= hSize & 0x1f;
					oHuffCode[i]|= (code[tmpCode] <<(32-hSize));
					continue;
				}

				nbits = 1;		// there must be at least one 1 bit 
				do 
				{
					if (tmpCode>>=1)
						nbits++;
				} while (tmpCode);		
			
				tmpCode=(r<<4) + nbits;
				tmpCode2&=((1<<nbits)-1);
				hSize=size[tmpCode] + nbits;

				oHuffCode[i]= hSize & 0x1f;
				oHuffCode[i]|= ((code[tmpCode] << nbits) | tmpCode2)<< (32-hSize);
			}
		}
	}
	else
	{
		unsigned int pCode,pLength;
		for (i=0;i<lastp;i++)
		{
			hCode=huffCode[i]<<(16-huffSize[i]);
			pLength=1<<(16-huffSize[i]);
			for (j=0;j<pLength;j++)
			{
				pCode=hCode|j;
				oHuffCode[pCode]=val[i];
				oHuffCode[pCode]|=huffSize[i]<<8;
			}
		}
	}

	return EOB;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//		Jpeg Decode Routines
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

bool JpegDecode(JpegInfo *jpeg,VideoData **video)
{
	unsigned char marker;
	// clear 
	JpegClear(jpeg);	
	while (marker=FindMarker(jpeg))
	{
		
		switch(marker)
		{
			
			case (M_SOF0):	ReadStartOfFrame(jpeg);		break;
			case (M_DHT):	ReadHuffTable(jpeg);		break;
			case (M_SOI):	break;
			case (M_EOI):	return true;	
			case (M_SOS):	if (ReadStartOfScan(jpeg))
							{
								NewVideoData(jpeg,video);
								ReadScanData(jpeg,*video);	
							}
							break;		
			case (M_DQT):	ReadQuantTable(jpeg);	break;
			
		}
		
	}
	
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char FindMarker(JpegInfo *jpeg)
{
	for (;jpeg->m_packPtr-jpeg->m_topPtr<jpeg->m_dataSize;jpeg->m_packPtr++)
		if (*jpeg->m_packPtr==0xff && *(jpeg->m_packPtr+1)!=0x00)
		{
			jpeg->m_packPtr++;
			break;
		}
	
	if (jpeg->m_packPtr-jpeg->m_topPtr>=jpeg->m_dataSize)
		return 0;

	return jpeg->m_packPtr[0];
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ReadStartOfFrame(JpegInfo *jpeg)
{
	if (*jpeg->m_packPtr++!=M_SOF0)	// only does base line
		return false;

	jpeg->m_packPtr+=2; 			// skip length

	if (*jpeg->m_packPtr++!=8)		// precision must be 8
		return false;
	
	jpeg->m_lines=*jpeg->m_packPtr++;
	jpeg->m_lines<<=8;
	jpeg->m_lines|=*jpeg->m_packPtr++;
	jpeg->m_samplesPerLine=*jpeg->m_packPtr++;
	jpeg->m_samplesPerLine<<=8;
	jpeg->m_samplesPerLine|=*jpeg->m_packPtr++;

	if (*jpeg->m_packPtr++!=3)			// only 3 componets 
		return false;

	if (*jpeg->m_packPtr++!=1)			// component labling 1	
		return false;
	if (*jpeg->m_packPtr++!=0x22)		// 22 scaling Y 	
		return false;
	if (*jpeg->m_packPtr++!=0x0)		// Q table 0 	
		return false;

	if (*jpeg->m_packPtr++!=2)			// component labling 2
		return false;
	if (*jpeg->m_packPtr++!=0x11)		// 11 scaling U
		return false;	
	if (*jpeg->m_packPtr++!=0x01)		// Q table 1 	
		return false;

	if (*jpeg->m_packPtr++!=3)			// component labling 
		return false;
	if (*jpeg->m_packPtr++!=0x11)		// 11 scaling V
		return false;
	if (*jpeg->m_packPtr++!=0x01)		// Q table 1	
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ReadStartOfScan(JpegInfo *jpeg)
{
	if (*jpeg->m_packPtr++!=M_SOS)	// only does base line
		return false;

	jpeg->m_packPtr+=2; 			// skip length

	if (*jpeg->m_packPtr++!=3)		// number of component
		return false;

	if (*jpeg->m_packPtr++!=1)		// Y
		return false;
	if (*jpeg->m_packPtr++!=0x00)	
		return false;

	if (*jpeg->m_packPtr++!=2)		// U
		return false;
	if (*jpeg->m_packPtr++!=0x11)	
		return false;

	if (*jpeg->m_packPtr++!=3)		// V
		return false;
	if (*jpeg->m_packPtr++!=0x11)	
		return false;

	// sequenqual only
	if (*jpeg->m_packPtr++!=0)	
		return false;
	if (*jpeg->m_packPtr++!=63)	
		return false;
	if (*jpeg->m_packPtr++!=0x00)	
		return false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ReadQuantTable(JpegInfo *jpeg)
{
	int i;
	if (*jpeg->m_packPtr++!=M_DQT)	// only does base line
		return false;
	jpeg->m_packPtr+=2; 			// skip length
	if (*jpeg->m_packPtr==0x00)
	{
		jpeg->m_packPtr++;
		for (i=0;i<64;i++)
		{
			if (*jpeg->m_packPtr!=0xff)
				jpeg->m_Y.m_quant[i]=*jpeg->m_packPtr++;
			else
				jpeg->m_Y.m_quant[i]=1;
		}
		ScaleQuant(jpeg->m_Y.m_quant,false);
		return true;
	}
	if (*jpeg->m_packPtr++==0x01)	
	{
		int i;
		for (i=0;i<64;i++)
		{
			if (*jpeg->m_packPtr!=0xff)
				jpeg->m_U.m_quant[i]=*jpeg->m_packPtr++;
			else
				jpeg->m_U.m_quant[i]=1;
		}
		ScaleQuant(jpeg->m_U.m_quant,false);
		memcpy(jpeg->m_V.m_quant,jpeg->m_U.m_quant,DCTSIZE*sizeof(int));
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ScaleQuant(int *quant,bool compress)
{
	int temp,temp2;
	double ftmp,scalei,scalej;
	int i;
	static const unsigned short aanscales[DCTSIZE2] = 
	{
	  // precomputed values scaled up by 14 bits 
	  16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
	  22725, 31521, 29692, 26722, 22725, 17855, 12299,  6270,
	  21407, 29692, 27969, 25172, 21407, 16819, 11585,  5906,
	  19266, 26722, 25172, 22654, 19266, 15137, 10426,  5315,
	  16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
	  12873, 17855, 16819, 15137, 12873, 10114,  6967,  3552,
	   8867, 12299, 11585, 10426,  8867,  6967,  4799,  2446,
	   4520,  6270,  5906,  5315,  4520,  3552,  2446,  1247
	};

	if (!compress)
	{
		for (i = 0; i < DCTSIZE2; i++) 
			quant[i]=IDESCALE((quant[i]*aanscales[ZAG[i]]),2);
		return;
	}

#ifdef QUANT_DIVIDE
	for (i = 0; i < DCTSIZE2; i++) 
		quant[i]=IDESCALE((quant[i]*aanscales[i]),11);
	return;
#else
			
	for (i=0;i<DCTSIZE2;i++)
	{
		scalei=1.0;
		scalej=1.0;
		// luminance
		temp = quant[i];
		
		if (i%8)	scalei=cos(3.141592654*(float)(i-8*(i/8))/16.0)*sqrt(2.0);	
		if (i/8)	scalej=cos(3.141592654*(float)(i/8)/16.0)*sqrt(2.0);

		ftmp=(float)(1<<14)*(scalei*scalej);
		temp2=(unsigned int)(ftmp+0.5);
		temp2*=temp;

		temp2= (temp2 + (1<<9))>>10; // shift back with 4 bits of int precious
		if (temp2)
		{
			ftmp= (float)(1<<16) / (float) temp2;
			temp= (int)(ftmp+0.5);
		}
		else
			temp=0xffff;
		if (temp<=0)	temp=1;
		if (temp > 0xffff) temp = 0xffff;
		quant[i]=temp; 
	}

	return;
#endif	

}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ReadHuffTable(JpegInfo *jpeg)
{
	unsigned char bits[17];
	unsigned char vals[257];
	unsigned char type;
	int length,i;
	memset(bits,0,17);
	memset(vals,0,257);

	if (*jpeg->m_packPtr++!=M_DHT)	// find huffman
		return false;

	length=*jpeg->m_packPtr++;
	length<<=8;
 	length|=*jpeg->m_packPtr++;
	length-=19;
	if (length>=257)
		return false;
	type=*jpeg->m_packPtr++;
	for (i=1;i<=16;i++)
		bits[i]=*jpeg->m_packPtr++;
	for (i=0;i<length;i++)
		vals[i]=*jpeg->m_packPtr++;

	if (type&0xf0)
	{
		if (type&0xf)
		{
			CreateHuffmanTables(jpeg->m_U.m_ACCode,bits,vals,false,false);
			CreateHuffmanTables(jpeg->m_V.m_ACCode,bits,vals,false,false);
		}
		else
			CreateHuffmanTables(jpeg->m_Y.m_ACCode,bits,vals,false,false);
	}
	else
	{
		if (type&0xf)
		{
			CreateHuffmanTables(jpeg->m_U.m_DCCode,bits,vals,false,true);
			CreateHuffmanTables(jpeg->m_V.m_DCCode,bits,vals,false,true);
		}
		else
			CreateHuffmanTables(jpeg->m_Y.m_DCCode,bits,vals,false,true);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool ReadScanData(JpegInfo *jpeg,VideoData *buf)
{		
	int i=0,jj,j,offset;
	int data[DCTSIZE2];
	unsigned char *Y,*U,*V;
	unsigned char *stuffed=jpeg->m_packPtr;
	unsigned char *unstuffed=jpeg->m_packPtr;

	// preprocess scan data and destuff it
	i=jpeg->m_packPtr-jpeg->m_topPtr;
	// remove stuffing for now
	for (;i<jpeg->m_dataSize;i++)
	{
		*unstuffed++=*stuffed;
		if (*stuffed++==0xff)
		{	
			if (*stuffed==0x00)
				stuffed++;
		}
	}

	Y=buf->m_Y;
	offset=8*buf->m_pitch;
	U=buf->m_U;
	V=buf->m_V;

	for (j=0;j<jpeg->m_lines;j+=16)
	{
		for (i=0;i<jpeg->m_samplesPerLine;i+=16)
		{
			DecodeBlock(jpeg,data,MCUY);
			FastIDCT(Y+i,buf->m_pitch,data); 

			DecodeBlock(jpeg,data,MCUY);
			FastIDCT(Y+i+8,buf->m_pitch,data); 

			DecodeBlock(jpeg,data,MCUY);
			FastIDCT(Y+i+offset,buf->m_pitch,data); 

			DecodeBlock(jpeg,data,MCUY);
			FastIDCT(Y+i+offset+8,buf->m_pitch,data); 

			DecodeBlock(jpeg,data,MCUU);
			FastIDCT(U+i/2,buf->m_pitch,data); 
					
			DecodeBlock(jpeg,data,MCUV);
			FastIDCT(V+i/2,buf->m_pitch,data); 

		}
		for (jj=0;jj<8;jj++)
		{
			memcpy(U+buf->m_pitch/2,U,buf->m_pitch/2);
			memcpy(V+buf->m_pitch/2,V,buf->m_pitch/2);
			U+=buf->m_pitch;
			V+=buf->m_pitch;
		}
		Y+=offset*2;
	}

	return true;	
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool DecodeBlock(JpegInfo *jpeg,int *data,int type)
{
	unsigned int *DCCode;
	int k,next,r;
	int *quant;
	unsigned int activeBytes=0;
	unsigned int index,n,select,ltmp;
	unsigned int bits;
	unsigned char *dataPtr;
	unsigned int *highLookUp;
	unsigned int s,mask,sign,shift;
	int *lastDCvalue,q;

	switch(type)
	{
		case (MCUY):
			lastDCvalue=&jpeg->m_Y.m_lastDCvalue;
			quant=jpeg->m_Y.m_quant;	
			DCCode=jpeg->m_Y.m_DCCode;
			highLookUp=jpeg->m_Y.m_ACCode;
			break;
		case (MCUU):
			lastDCvalue=&jpeg->m_U.m_lastDCvalue;
			quant=jpeg->m_U.m_quant;	
			DCCode=jpeg->m_U.m_DCCode;
			highLookUp=jpeg->m_U.m_ACCode;
			break;
		case (MCUV):
			lastDCvalue=&jpeg->m_V.m_lastDCvalue;
			quant=jpeg->m_V.m_quant;	
			DCCode=jpeg->m_V.m_DCCode;
			highLookUp=jpeg->m_V.m_ACCode;
			break;
	}
	memset(data,0,DCTSIZE2*sizeof(unsigned int));
	
	// copy in pack ptrs
	bits=jpeg->m_bits;
	dataPtr=jpeg->m_packPtr;
	
	// form up first coeff
	shift=8-bits;
	activeBytes=dataPtr[2];
	activeBytes|= (dataPtr[1] <<8) | (dataPtr[0]<<16);
	activeBytes>>=shift;

	select=activeBytes & 0xffff;
	ltmp=DCCode[select];	// unsigned short

	s=ltmp & 0xff;
	n=(ltmp>>8) & 0xff;

	bits+=n;
	dataPtr+=bits>>3;
	bits&=0x7;

	// decode 
	if (s) 
	{
		bits+=s;
		shift=24-bits;
		sign=1<<(s-1);
		mask=(1<<s)-1;
		
		activeBytes=dataPtr[2];
		activeBytes|= (dataPtr[1] <<8) | (dataPtr[0]<<16);
		activeBytes>>=shift;
		activeBytes&=mask;

		// store pointersd for return
		dataPtr+=(bits>>3);
		bits&=0x7;

		if ((sign&activeBytes)==0)
		{
			activeBytes|=~mask;
			activeBytes++;
		}		
		//Convert DC difference to actual value, update last_dc_val
		activeBytes+=*lastDCvalue;
		*lastDCvalue=activeBytes;
		// Output the DC coefficient (assumes ZAG[0] = 0) 
		data[0] =activeBytes * quant[0];

	}
	else
		data[0]=*lastDCvalue* quant[0];
	data[0]+=1<<11;
	data[0]>>=12;


	// Section F.2.2.2: decode the AC coefficients 
	// Since zeroes are skipped, output area must be cleared beforehand 
	next=1;
	s=0;
	k=1;
	r=0;

//////////////////////////////////////////////////////////////////////

decodeLoop:
		shift=8-bits;
		activeBytes=dataPtr[2];
		activeBytes|= (dataPtr[1] <<8) | (dataPtr[0]<<16);
		activeBytes>>=shift;

		select=activeBytes & 0xffff;
		ltmp=highLookUp[select];	// unsigned short

		index=ltmp & 0xff;
		n=(ltmp>>8) & 0xff;

		bits+=n;
		dataPtr+=bits>>3;
		bits&=0x7;

		// decode 
		s=index & 0xf;
		r=index >> 4;
		k += r;
		next = ZAG[k];
		q=(int)quant[k];
		k++;
		
		if (s==0 && r!=15) 
			goto decodeLoopExit;
			
		if (s==0)
			goto decodeLoop;

		bits+=s;
		shift=24-bits;
		sign=1<<(s-1);
		mask=(1<<s)-1;
		
		activeBytes=dataPtr[2];
		activeBytes|= (dataPtr[1] <<8) | (dataPtr[0]<<16);
		activeBytes>>=shift;
		activeBytes&=mask;

		// store pointersd for return
		dataPtr+=(bits>>3);
		bits&=0x7;

		if ((sign&activeBytes)==0)
		{
			activeBytes|=~mask;
			activeBytes++;
		}		

		data[next]= activeBytes * q ;
		data[next]+=1<<11;
		data[next]>>=12;

		if (k>=64) 
			goto decodeLoopExit;

goto decodeLoop;
		
//////////////////////////////////////////////////////////////////////

decodeLoopExit:

	jpeg->m_packPtr=dataPtr;
	jpeg->m_bits=bits;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//    JPEG encode
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

bool JpegEncodeSetup(JpegInfo *jpeg,int quality)
{
	jpeg->m_quality=quality;
	// setup Y plane
	CreateHuffmanTables(jpeg->m_Y.m_DCCode,bitsDCLuminance,valDCLuminance,true,true);
	jpeg->m_Y.m_EOB=CreateHuffmanTables(jpeg->m_Y.m_ACCode,bitsACLuminance,valACLuminance,true,false);
	CreateHuffmanTables(jpeg->m_U.m_DCCode,bitsDCChrominance,valDCChrominance,true,true);
	jpeg->m_U.m_EOB=CreateHuffmanTables(jpeg->m_U.m_ACCode,bitsACChrominance,valACChrominance,true,false);
	CreateHuffmanTables(jpeg->m_V.m_DCCode,bitsDCChrominance,valDCChrominance,true,true);
	jpeg->m_V.m_EOB=CreateHuffmanTables(jpeg->m_V.m_ACCode,bitsACChrominance,valACChrominance,true,false);

	memcpy(jpeg->m_Y.m_quant,stdYQuant,DCTSIZE2*sizeof(int));
	QuantQualityScale(jpeg->m_quality,jpeg->m_Y.m_quant);
	ScaleQuant(jpeg->m_Y.m_quant,true);

	memcpy(jpeg->m_U.m_quant,stdUVQuant,DCTSIZE2*sizeof(int));
	QuantQualityScale(jpeg->m_quality,jpeg->m_U.m_quant);
	ScaleQuant(jpeg->m_U.m_quant,true);
	memcpy(jpeg->m_V.m_quant,jpeg->m_U.m_quant,DCTSIZE2*sizeof(int));
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool JpegEncode(JpegInfo *jpeg,VideoData *video)
{
	if (!video)
		return false;
	jpeg->m_samplesPerLine=video->m_width;
	jpeg->m_lines=video->m_height;
	// attach buffer for jpeg
	jpeg->m_packPtr=jpeg->m_topPtr;

	// Start of IMG
	*jpeg->m_packPtr++=0xff;
	*jpeg->m_packPtr++=M_SOI;

	// APP marker
	*jpeg->m_packPtr++=0xff;
	*jpeg->m_packPtr++=0xe0;
	*jpeg->m_packPtr++=0;
	*jpeg->m_packPtr++=0x10;
	*jpeg->m_packPtr++='J';
	*jpeg->m_packPtr++='F';
	*jpeg->m_packPtr++='I';
	*jpeg->m_packPtr++='F';
	*jpeg->m_packPtr++=0;
	*jpeg->m_packPtr++=1;
	*jpeg->m_packPtr++=1;
	*jpeg->m_packPtr++=1;
	*jpeg->m_packPtr++=0;
	*jpeg->m_packPtr++=0;
	*jpeg->m_packPtr++=0;
	*jpeg->m_packPtr++=0;
	*jpeg->m_packPtr++=0;
	*jpeg->m_packPtr++=0;

	// DQT
	WriteQuantTable(jpeg,0x0);
	WriteQuantTable(jpeg,0x1);	

	// Start of frame
	WriteStartOfFrame(jpeg);

	// DHT
	WriteHuffTable(jpeg,0x00);
	WriteHuffTable(jpeg,0x10);
	WriteHuffTable(jpeg,0x01);
	WriteHuffTable(jpeg,0x11);

	// Start of Scan
	WriteStartOfScan(jpeg);

	// compressed data
	WriteCompressedData(jpeg,video);

	// End of Image
	*jpeg->m_packPtr++=0xff;
	*jpeg->m_packPtr++=M_EOI;
	// size
	jpeg->m_compSize=jpeg->m_packPtr-jpeg->m_topPtr;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool WriteStartOfFrame(JpegInfo *jpeg)
{
	*jpeg->m_packPtr++=0xff;
	*jpeg->m_packPtr++=M_SOF0;
	*jpeg->m_packPtr++=0;		
	*jpeg->m_packPtr++=17;								// length
	*jpeg->m_packPtr++=8;								// precission
	*jpeg->m_packPtr++=(jpeg->m_lines>>8)&0xff;			// height
	*jpeg->m_packPtr++=jpeg->m_lines & 0xff;
	*jpeg->m_packPtr++=(jpeg->m_samplesPerLine>>8)&0xff;// width
	*jpeg->m_packPtr++=jpeg->m_samplesPerLine & 0xff;
	*jpeg->m_packPtr++=3;								//  3 componets 
	*jpeg->m_packPtr++=1;								// component labling 1	
	*jpeg->m_packPtr++=0x22;							// 22 scaling Y 	
	*jpeg->m_packPtr++=0x0;								// Q table 0 	
	*jpeg->m_packPtr++=2;								// component labling 2
	*jpeg->m_packPtr++=0x11;							// 11 scaling U
	*jpeg->m_packPtr++=0x01;							// Q table 1 	
	*jpeg->m_packPtr++=3;								// component labling 
	*jpeg->m_packPtr++=0x11;							// 11 scaling V
	*jpeg->m_packPtr++=0x01;							// Q table 1	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool WriteStartOfScan(JpegInfo *jpeg)
{
	*jpeg->m_packPtr++=0xff;
	*jpeg->m_packPtr++=M_SOS;	// only does base line
	*jpeg->m_packPtr++=0;		
	*jpeg->m_packPtr++=12;		// length
	*jpeg->m_packPtr++=3;		// number of components
	*jpeg->m_packPtr++=1;		// Y
	*jpeg->m_packPtr++=0x00;	
	*jpeg->m_packPtr++=2;		// U
	*jpeg->m_packPtr++=0x11;	
	*jpeg->m_packPtr++=3;		// V
	*jpeg->m_packPtr++=0x11;	
	*jpeg->m_packPtr++=0;	
	*jpeg->m_packPtr++=63;	
	*jpeg->m_packPtr++=0x00;	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool WriteHuffTable(JpegInfo *jpeg,unsigned char type)
{
	unsigned char *bits=NULL;
	unsigned char *vals=NULL;
	int length,i;

	*jpeg->m_packPtr++=0xff;
	*jpeg->m_packPtr++=M_DHT;
	if (type&0xf0)
	{
		if (type&0xf)
		{
			bits=bitsACChrominance;
			vals=valACChrominance;
		}
		else
		{		
			bits=bitsACLuminance;
			vals=valACLuminance;
		}
	}
	else
	{
		if (type&0xf)
		{			
			bits=bitsDCChrominance;
			vals=valDCChrominance;
		}
		else
		{
			bits=bitsDCLuminance;
			vals=valDCLuminance;
		}
	}
	
	if (!bits || !vals)
		return false;

	length=0;
	for (i=1;i<=16;i++)
		length+=bits[i];
	length+=19;

	*jpeg->m_packPtr++=(length>>8) & 0xff;
	*jpeg->m_packPtr++=length & 0xff;	
	*jpeg->m_packPtr++=type;
	for (i=1;i<=16;i++)
		*jpeg->m_packPtr++=bits[i];
	for (i=0;i<length-19;i++)
		*jpeg->m_packPtr++=vals[i];

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool WriteQuantTable(JpegInfo *jpeg,unsigned char type)
{
	int quant[DCTSIZE2];
	int i;
	*jpeg->m_packPtr++=0xff;
	*jpeg->m_packPtr++=M_DQT;
	*jpeg->m_packPtr++=0;
	*jpeg->m_packPtr++=DCTSIZE2+3;	
	*jpeg->m_packPtr++=type;	

	if (type==0x00)
		memcpy(quant,stdYQuant,DCTSIZE2*sizeof(int));
	if (type==0x01)	
		memcpy(quant,stdUVQuant,DCTSIZE2*sizeof(int));

	QuantQualityScale(jpeg->m_quality,quant);
	for (i=0;i<64;i++)
		*jpeg->m_packPtr++=(unsigned char)quant[ZAG[i]];

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool WriteCompressedData(JpegInfo *jpeg,VideoData *video)
{
	unsigned char *unstuffed,*stuffed;
	unsigned char *Y,*U,*V;
	int offset;
	int data[DCTSIZE2+2],i=0,j=0;
	data[64]=0xffffff; // endstop for huffman
	data[65]=0xffffff; // endstop for huffman
	
	jpeg->m_bits=0;
	jpeg->m_activeBytes=0;
	stuffed=jpeg->m_packPtr;
	jpeg->m_packPtr+=32784; //8k gap for stuffing - may be a problem
	unstuffed=jpeg->m_packPtr;

	Y=video->m_Y;
	offset=8*video->m_pitch;
	U=video->m_U;
	V=video->m_V;
	jpeg->m_Y.m_lastDCvalue=0;
	jpeg->m_U.m_lastDCvalue=0;
	jpeg->m_V.m_lastDCvalue=0;

	for (j=0;j<jpeg->m_lines;j+=16)
	{
		for (i=0;i<jpeg->m_samplesPerLine;i+=16)		
		{
			FastDCT (Y+i,video->m_pitch,data,jpeg->m_Y.m_quant);
			EncodeBlock(jpeg,data,MCUY);

			FastDCT (Y+i+8,video->m_pitch,data,jpeg->m_Y.m_quant);
			EncodeBlock(jpeg,data,MCUY);

			FastDCT (Y+i+offset,video->m_pitch,data,jpeg->m_Y.m_quant);
			EncodeBlock(jpeg,data,MCUY);

			FastDCT (Y+i+8+offset,video->m_pitch,data,jpeg->m_Y.m_quant);
			EncodeBlock(jpeg,data,MCUY);

			FastDCT (U+i/2,video->m_pitch,data,jpeg->m_U.m_quant);
			EncodeBlock(jpeg,data,MCUU);
					
			FastDCT (V+i/2,video->m_pitch,data,jpeg->m_V.m_quant);
			EncodeBlock(jpeg,data,MCUV);
		}
		U+=offset;
		V+=offset;
		Y+=offset*2;
	}

	// terminate bit write
	if (jpeg->m_bits)
	{
		jpeg->m_packPtr[0]=jpeg->m_activeBytes>>24;
		jpeg->m_packPtr++;
	}

	for (;unstuffed<jpeg->m_packPtr;)
	{	
		*stuffed=*unstuffed++;
		if (*stuffed++==0xff)
			*stuffed++=0x00;
	}
	jpeg->m_packPtr=stuffed;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

#define	UBYTESEL(x,sel)	( (unsigned char) ( ((x) >> (sel*8)) & 0xff))
#define	byteSelector(x,sel)	( (unsigned char) ( ((x) >> (sel)) & 0xff))

// trimedia method packing method 
/*
#define PackHuffData()							\
	packPtr[0]=UBYTESEL(activeBytes,3);			\
	packPtr[1]=UBYTESEL(activeBytes,2);			\
	packPtr[2]=UBYTESEL(activeBytes,1);			\
	packPtr[3]=UBYTESEL(activeBytes,0);			\
	activeBytes=(activeBytes<< shift); 			\
	if (shift==32)	activeBytes=0;				\
	if (bits>32)	activeBytes|=(data<<25);	\
	packPtr+=bits>>3 	// leave last ; off

*/
#define PackHuffData()											\
	if (shift>=8)	packPtr[0]=byteSelector(activeBytes,24);	\
	if (shift>=16)	packPtr[1]=byteSelector(activeBytes,16);	\
	if (shift>=24)	packPtr[2]=byteSelector(activeBytes,8);		\
	if (shift>=32)	packPtr[3]=byteSelector(activeBytes,0);		\
	activeBytes=(activeBytes<< shift); 						\
	if (shift==32)	activeBytes=0;							\
	if (bits>32)	activeBytes|=(data<<25);				\
	packPtr+=bits>>3 	// leave last ; off

#define WriteHuffData()							\
	bits&=0x7;									\
	data=ltmp&mask;								\
	n=ltmp&0x1f;								\
	activeBytes|=(data>>bits);					\
	bits+=n;									\
	shift=bits&0x38 	// leave last ; off

bool EncodeBlock(JpegInfo *jpeg,int *dataPtr,int type)
{
	unsigned char *packPtr=jpeg->m_packPtr;
	unsigned int activeBytes=jpeg->m_activeBytes;
	unsigned int *ACCode;
	unsigned int *DCCode;
	unsigned int bits=jpeg->m_bits;
	unsigned int shift,ltmp;
	unsigned int next0;
	unsigned int temp0;
	unsigned int data=0,n=0;
	unsigned int jump,r;
	int nbits,temp,temp2,i;
	unsigned int EOB=0;
	const unsigned int mask=0xffffffc0;
	const unsigned int jumpMask=0xffffff;
	const unsigned int codeMask=0x7ff;
	const unsigned int runSize=2048;
	int *lastDC=0;

	switch(type)
	{
		case (MCUY):
			lastDC=&jpeg->m_Y.m_lastDCvalue;
			DCCode=jpeg->m_Y.m_DCCode;
			ACCode=jpeg->m_Y.m_ACCode;
			EOB=jpeg->m_Y.m_EOB;
			break;
		case (MCUU):
			lastDC=&jpeg->m_U.m_lastDCvalue;
			DCCode=jpeg->m_U.m_DCCode;
			ACCode=jpeg->m_U.m_ACCode;
			EOB=jpeg->m_U.m_EOB;
			break;
		case (MCUV):
			lastDC=&jpeg->m_V.m_lastDCvalue;
			DCCode=jpeg->m_V.m_DCCode;
			ACCode=jpeg->m_V.m_ACCode;
			EOB=jpeg->m_V.m_EOB;
			break;
	}

	//Encode the DC coefficient difference per section F.1.2.1
	temp0=dataPtr[0]- *lastDC;
	*lastDC=dataPtr[0];

	// huf code DC
	i=temp0 & 0xfff;
	ltmp=DCCode[i];
	WriteHuffData();

	// Encode the AC coefficients per section F.1.2.2  
	dataPtr++;
	next0=dataPtr[0];
	dataPtr++;
	r=0;
loop:

	temp0 = next0 & codeMask;
	jump=next0;
	next0=*dataPtr;
	dataPtr++;
	r+=runSize;

	if (jump==0)
		goto loop;

	if (jump==jumpMask)
		goto exitLoop;

	r-=runSize;

	if (r>15*runSize)
		goto overRunLoop;

	// Emit Huffman symbol for run length / number of bits 
	i = r  + temp0;
	ltmp=ACCode[i];
	PackHuffData();

	// write bytes to active space
	WriteHuffData();
	//runs
	r=0;
	goto loop;


//////////////////////////////////////////////////////////////////////

overRunLoop:
	// if run length > 15, must emit special run-length-16 codes (0xF0)
	while (r > 15*runSize) 
	{
		ltmp=ACCode[runSize*15]; // data to compress
		PackHuffData();
		r -= runSize*16;
		WriteHuffData();
	}	

    next0=jump;
	dataPtr--;
	goto loop;

//////////////////////////////////////////////////////////////////////

exitLoop:
	// finish off packing
	PackHuffData();
	r-=runSize;
	// If the last coef(s) were zero, emit an end-of-block code 
	if (r)
	{
		ltmp=EOB;
		WriteHuffData();
		PackHuffData();
	}

	// put varibles back in class
	bits&=0x7;
	jpeg->m_packPtr=packPtr;
	jpeg->m_activeBytes=activeBytes;
	jpeg->m_bits=bits;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//    DCTs forwards and inverse
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#define limit(x) ((x) > 255 ? 255 : (x < 0 ? 0 :x))

void FastIDCT(unsigned char *Y,int jump,int *data) 
{
	int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	int tmp10, tmp11, tmp12, tmp13;
	int z5, z10, z11, z12, z13;
	int *inptr;
	unsigned char *outptr;
	int *wsptr;
	int ctr;
	int workspace[DCTSIZE2];

	// Pass 1: process columns from input, store into work array. 
	inptr = data;
	wsptr = workspace;

	for (ctr = DCTSIZE; ctr > 0; ctr--) 
	{
		// Even part 
		tmp0 = inptr[DCTSIZE*0];
		tmp1 = inptr[DCTSIZE*2];
		tmp2 = inptr[DCTSIZE*4];
		tmp3 = inptr[DCTSIZE*6];

		tmp10 = tmp0 + tmp2;
		tmp11 = tmp0 - tmp2;

		tmp13 = tmp1 + tmp3;	
		tmp12 = MULTIPLY(tmp1 - tmp3, FIX_1_414213562) - tmp13; 

		tmp0 = tmp10 + tmp13;	
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		// Odd part 
		tmp4 = inptr[DCTSIZE*1];
		tmp5 = inptr[DCTSIZE*3];
		tmp6 = inptr[DCTSIZE*5];
		tmp7 = inptr[DCTSIZE*7];

		z13 = tmp6 + tmp5;		// phase 6 
		z10 = tmp6 - tmp5;
		z11 = tmp4 + tmp7;
		z12 = tmp4 - tmp7;

		tmp7 = z11 + z13;	
		tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562); 

		z5 = MULTIPLY(z10 + z12, FIX_1_847759065); 
		tmp10 = MULTIPLY(z12, FIX_1_082392200) - z5; 
		tmp12 = MULTIPLY(z10, - FIX_2_613125930) + z5;

		tmp6 = tmp12 - tmp7;	
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;

		wsptr[DCTSIZE*0] = (int) (tmp0 + tmp7);
		wsptr[DCTSIZE*7] = (int) (tmp0 - tmp7);
		wsptr[DCTSIZE*1] = (int) (tmp1 + tmp6);
		wsptr[DCTSIZE*6] = (int) (tmp1 - tmp6);
		wsptr[DCTSIZE*2] = (int) (tmp2 + tmp5);
		wsptr[DCTSIZE*5] = (int) (tmp2 - tmp5);
		wsptr[DCTSIZE*4] = (int) (tmp3 + tmp4);
		wsptr[DCTSIZE*3] = (int) (tmp3 - tmp4);

		inptr++;	
		wsptr++;
	}

	wsptr = workspace;
	outptr = Y;
	for (ctr = 0; ctr < DCTSIZE; ctr++) 
	{
		// Even part 

		tmp10 = (wsptr[0] + wsptr[4]);
		tmp11 = (wsptr[0] - wsptr[4]);

		tmp13 = (wsptr[2] + wsptr[6]);
		tmp12 = MULTIPLY(wsptr[2] - wsptr[6], FIX_1_414213562)
			- tmp13;

		tmp0 = tmp10 + tmp13;
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		// Odd part 
		z13 = wsptr[5] + wsptr[3];
		z10 = wsptr[5] - wsptr[3];
		z11 = wsptr[1] + wsptr[7];
		z12 = wsptr[1] - wsptr[7];

		tmp7 = z11 + z13;		
		tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562); 

		z5 = MULTIPLY(z10 + z12, FIX_1_847759065); 
		tmp10 = MULTIPLY(z12, FIX_1_082392200) - z5;
		tmp12 = MULTIPLY(z10, - FIX_2_613125930) + z5; 

		tmp6 = tmp12 - tmp7;
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;

		// output
		//////////////////////////////

		////////////////////////////////

		outptr[0] = limit(IDESCALE(tmp0 + tmp7, PASS1_BITS)+128);
		outptr[7] = limit(IDESCALE(tmp0 - tmp7, PASS1_BITS)+128);
		outptr[1] = limit(IDESCALE(tmp1 + tmp6, PASS1_BITS)+128);
		outptr[6] =	limit(IDESCALE(tmp1 - tmp6, PASS1_BITS)+128);
		outptr[2] = limit(IDESCALE(tmp2 + tmp5, PASS1_BITS)+128);
		outptr[5] = limit(IDESCALE(tmp2 - tmp5, PASS1_BITS)+128);
		outptr[4] = limit(IDESCALE(tmp3 + tmp4, PASS1_BITS)+128);
		outptr[3] = limit(IDESCALE(tmp3 - tmp4, PASS1_BITS)+128);

		wsptr+=DCTSIZE;
		outptr+=jump;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void FastDCT (unsigned char * Y,int jump,int *data,int *quant)
{
	int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	int tmp10, tmp11, tmp12, tmp13;
	int z1, z2, z3, z4, z5, z11, z13;
	int *dataptr;
	int ctr,i;
	int workSpace[DCTSIZE2];

	dataptr=workSpace;
	// Pass 1: process rows. 
	for (ctr = DCTSIZE-1; ctr >= 0; ctr--) 
	{
		tmp0 = Y[0] + Y[7] -256;
		tmp7 = Y[0] - Y[7];
		tmp1 = Y[1] + Y[6] -256;
		tmp6 = Y[1] - Y[6];
		tmp2 = Y[2] + Y[5] -256;
		tmp5 = Y[2] - Y[5];
		tmp3 = Y[3] + Y[4] -256;
		tmp4 = Y[3] - Y[4];

		//even
		tmp10 = tmp0 + tmp3;	
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;

		dataptr[0] = tmp10 + tmp11; 
		dataptr[4] = tmp10 - tmp11;

		z1 = MULTIPLY(tmp12 + tmp13, FIX_0_707106781); 
		dataptr[2] = tmp13 + z1;	
		dataptr[6] = tmp13 - z1;

		// Odd part 

		tmp10 = tmp4 + tmp5;	
		tmp11 = tmp5 + tmp6;
		tmp12 = tmp6 + tmp7;

		z5 = MULTIPLY(tmp10 - tmp12, FIX_0_382683433); 
		z2 = MULTIPLY(tmp10, FIX_0_541196100) + z5; 
		z4 = MULTIPLY(tmp12, FIX_1_306562965) + z5; 
		z3 = MULTIPLY(tmp11, FIX_0_707106781); 

		z11 = tmp7 + z3;		
		z13 = tmp7 - z3;

		dataptr[5] = z13 + z2;
		dataptr[3] = z13 - z2;
		dataptr[1] = z11 + z4;
		dataptr[7] = z11 - z4;

		dataptr += DCTSIZE;
		Y+=jump;
	}

	//Pass 2: process columns. 
	dataptr = workSpace;
	for (ctr = DCTSIZE-1; ctr >= 0; ctr--) 
	{
		tmp0 = dataptr[DCTSIZE*0] + dataptr[DCTSIZE*7];
		tmp7 = dataptr[DCTSIZE*0] - dataptr[DCTSIZE*7];
		tmp1 = dataptr[DCTSIZE*1] + dataptr[DCTSIZE*6];
		tmp6 = dataptr[DCTSIZE*1] - dataptr[DCTSIZE*6];
		tmp2 = dataptr[DCTSIZE*2] + dataptr[DCTSIZE*5];
		tmp5 = dataptr[DCTSIZE*2] - dataptr[DCTSIZE*5];
		tmp3 = dataptr[DCTSIZE*3] + dataptr[DCTSIZE*4];
		tmp4 = dataptr[DCTSIZE*3] - dataptr[DCTSIZE*4];

		// Even part 

		tmp10 = tmp0 + tmp3;	
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;

		dataptr[DCTSIZE*0] = tmp10 + tmp11; 
		dataptr[DCTSIZE*4] = tmp10 - tmp11;

		z1 = MULTIPLY(tmp12 + tmp13, FIX_0_707106781);
		dataptr[DCTSIZE*2] = tmp13 + z1; 
		dataptr[DCTSIZE*6] = tmp13 - z1;

		// Odd part 

		tmp10 = tmp4 + tmp5;
		tmp11 = tmp5 + tmp6;
		tmp12 = tmp6 + tmp7;

		z5 = MULTIPLY(tmp10 - tmp12, FIX_0_382683433);
		z2 = MULTIPLY(tmp10, FIX_0_541196100) + z5;
		z4 = MULTIPLY(tmp12, FIX_1_306562965) + z5;
		z3 = MULTIPLY(tmp11, FIX_0_707106781);

		z11 = tmp7 + z3;	
		z13 = tmp7 - z3;

		dataptr[DCTSIZE*5] = z13 + z2; 
		dataptr[DCTSIZE*3] = z13 - z2;
		dataptr[DCTSIZE*1] = z11 + z4;
		dataptr[DCTSIZE*7] = z11 - z4;

		dataptr++;		
	}

	
	for (i=0;i<DCTSIZE2;i++)
#ifdef QUANT_DIVIDE	
		data[ZIG[i]]=workSpace[i]/quant[i];
#else
		data[ZIG[i]]=(workSpace[i]*quant[i] + (1<<14))>>15;
#endif


}

/////////////////////////////////////////////////////////////////////////////////////////////
