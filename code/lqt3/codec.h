#ifndef H_CODEC_H
#define H_CODEC_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#ifdef WIN32
	#include <windows.h>
#else
	#define DWORD unsigned int
#endif
#ifndef __cplusplus
	#define bool unsigned char
	#define true 1
	#define false 0
#endif

///////////////////////////////////////////////////////////////////////////////////////

#define MAX_COEFF 6
#define Q 3.0
#define MAX_SIZE 32
#ifndef MYABS
	#define MYABS
#define myabs(x)    ( x < 0 ? -x : x)
#endif
#ifndef RINT
	#define RINT
#define rint(x) (x > 0 ? (int)(x+0.5) : (int)(x-0.5) )
#endif

#define AbsLimit(x) ( x < 0 ? (-x >255 ? 255:-x) : (x >255 ? 255 : x))
#define Limit(x) ((x) > 255 ? 255 : (x < 0 ? 0 :x))

typedef enum block_type 
{
	currentFrameBlock	=	0,
	lastFrameBlock		=	2,
	backGroundBlock		=	3,	
} BLOCK_TYPE;

///////////////////////////////////////////////////////////////////////////////////////

typedef struct _Dump
{
	unsigned char m_pt[128000]; // max size
	unsigned char *m_packPtr;
	unsigned int m_activeBytes;
	int m_bits;
	int m_size;	// data in m_pt
} Dump;

///////////////////////////////////////////////////////////////////////////////////////

Dump *Dump_Create();
void Dump_Destroy(Dump **inst);
void Dump_Rewind(Dump *inst);
void Dump_BitMode(Dump *inst);
void Dump_TerminateBitWrite(Dump *inst);
void Dump_TerminateBitRead(Dump *inst);

///////////////////////////////////////////////////////////////////////////////////////

#define MAX_HUFFMAN_SIZE 65536
#define MAX_HUFFMAN_COEFF 1024

///////////////////////////////////////////////////////////////////////////////////////

typedef struct _Huffman 
{
    int m_max; // max coeff
	int m_min; // min coeff
	short m_decode[MAX_HUFFMAN_SIZE];
	unsigned int m_codeSymbols[MAX_HUFFMAN_COEFF];
	unsigned int *m_code;
	Dump *m_dump;
}Huffman ;

///////////////////////////////////////////////////////////////////////////////////////

#define EstimateCompressSymbol(symbol) m_code[symbol] & 0xffff
Huffman *Huffman_Create();
void Huffman_Destroy(Huffman **inst);
bool Huffman_Setup(Huffman *inst,Dump *dump,unsigned int *codes,int min,int max);
void Huffman_CompressSymbol(Huffman *huff,int symbol);
int Huffman_UncompressSymbol(Huffman *huff);

///////////////////////////////////////////////////////////////////////////////////////
// Block definitions 
///////////////////////////////////////////////////////////////////////////////////////

typedef struct _Block {

	// member data
	unsigned char *m_currentFrame; // pointer to loactaion in image data 
	unsigned char *m_lastFrame; // pointer to location of image data from last frame
	unsigned char *m_backGroundFrame; // pointer to location of image data from background frame
	int m_x;	// x postion inside image
	int m_y;	// y position in image
	int m_jump; // jumps to next line in block through image pt 
    int m_size; // size of block - assumes square 
    float m_c[MAX_COEFF];   // DCT coefficients
    int m_error; // Squared Error sum for block;

	//flags
	bool m_isLastFrame;		// used to indicate that last frame exists
	int m_blockType; // used to pick which frame is been used

	// linked list pointers
    struct _Block *m_next;
	struct _Block *m_sortNext;

} Block;


///////////////////////////////////////////////////////////////////////////////////////

Block *	Block_Create();
void	Block_Destroy(Block **inst);
bool	Block_SetupBlock(Block *inst,int x,int y,int size,unsigned char *current,
					  unsigned char *last,unsigned char *backGround,int pitch);
bool	Block_InternalSetupBlock(Block *inst,int x,int y,int size,Block *parent);
int		Block_CalcLastBlockError(Block *inst);
int		Block_CalcBackGroundBlockError(Block *inst);
int		Block_CalcCurrentBlockError(Block *inst);
bool	Block_RenderBlock(Block *inst);
bool	Block_CopyLastFrame(Block *inst);
bool	Block_CopyBackGroundFrame(Block *inst);
bool	Block_CodeBlock(Block *inst);
int		Block_ChoseMethod(Block *inst,Huffman **coeff,Huffman *qtree,bool forceCode);
void	Block_QuantiseBlock(Block *inst);
void	Block_UnquantiseBlock(Block *inst);
int		Block_HuffmanEncodeBlock(Block *inst,Huffman **coeff);
int		Block_HuffmanEstimateBlock(Block *inst,Huffman **coeff);
void	Block_HuffmanDecodeBlock(Block *inst,Huffman **coeff);

void	Block_Render2x2m4(Block *inst);
void	Block_Render4x4m6(Block *inst);
void	Block_Render8x8m6(Block *inst);
void	Block_Render16x16m6(Block *inst);
void	Block_Render32x32m6(Block *inst);

void	Block_Code2x2m4(Block *inst);
void	Block_Code4x4m6(Block *inst);
void	Block_Code8x8m6(Block *inst);
void	Block_Code16x16m6(Block *inst);
void	Block_Code32x32m6(Block *inst);

///////////////////////////////////////////////////////////////////////////////////////

typedef struct _BlockList 
{
    Block *m_start;
	int m_nItems; 

} BlockList;

BlockList *	BlockList_Create();
void		BlockList_Destroy(BlockList **inst);
bool		BlockList_Put(BlockList *inst,Block *in);
int			BlockList_Max(BlockList *inst);
Block *		BlockList_Get(BlockList *inst);
bool		BlockList_MergeLists(BlockList *inst,BlockList *in); 
bool		BlockList_ClearListLinking(BlockList *inst);

///////////////////////////////////////////////////////////////////////////////////////

typedef struct _BuffList 
{
	// sorted buffer for incoming information
	BlockList *m_buffer;
	int m_maxBufferSize;
	// List of sorted information
	BlockList *m_list;
	bool m_getNew;
} BuffList;

///////////////////////////////////////////////////////////////////////////////////////

bool		BuffList_ClearListLinking(BuffList *inst);
Block *		BuffList_Get(BuffList *inst);
bool		BuffList_Put(BuffList *inst,Block *in);
void		BuffList_Destroy(BuffList **inst);
BuffList *	BuffList_Create(int N);

///////////////////////////////////////////////////////////////////////////////////////
//	video data
///////////////////////////////////////////////////////////////////////////////////////

#ifndef VIDEODATA
#define VIDEODATA

typedef struct _SystemTime
{
	unsigned int m_relativeTime; // in frames @ 50fps from ref point
	int m_frames;
	int m_seconds;
	int m_mins;
	int m_hours;
	int m_days;
	int m_months;
	int m_years;

	char m_time[12];
	char m_date[11];

} SystemTime;

/////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_VIDEO_TEXT 5
typedef struct _VideoData
{
	unsigned char *m_Y;
	unsigned char *m_U;
	unsigned char *m_V;
	int m_width;
	int m_height;
	int m_pitch;
	int m_camera;
	SystemTime m_time; // in frames @ 50fps from ref point

	// text
	bool m_hasTextTime;
	int m_timeX;
	int m_timeY;
	bool m_hasTextDate;
	int m_dateX;
	int m_dateY;

	bool m_hasText;
	int m_x[MAX_VIDEO_TEXT];
	int m_y[MAX_VIDEO_TEXT];
	char m_text[MAX_VIDEO_TEXT][64];

	// memory chunck all above is stored in
	unsigned char *m_basePtr;
	unsigned int m_size;

} VideoData;

#endif
// create video data
#define BG_BLOCKSIZE  8
#define BG_SHIFT	  3			// above in shift form
#define	BG_BLOCKSIZE2 64		// above squared

typedef struct _Qtree 
{

    VideoData *m_image;
	VideoData *m_last;
	VideoData *m_backGround;
	VideoData *m_backGroundMask;

	float			m_BPP;
	float			m_FFBPP;
	unsigned int	m_frameSize;
	float			m_MSEThreshold;
	float			m_rateThreshold;
	int				m_noRuns; // number of times the compressor has been run
	bool			m_noUpdate; // usually true
	bool			m_readyToCompress;
	bool			m_postProcess;

	Dump *m_compressed; // store of compressed data
	Dump *m_recoded; 

	int m_size;
    Huffman *m_qtree;
	Huffman *m_coeff[MAX_COEFF];
	Block *m_top; // pointer to the top of the block list

    
	// free list managment
	Block *m_spareBlocks;
	int m_numBlocks;

	// list sorting
	BuffList *m_list;


} Qtree;

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

void	Qtree_AddImage(Qtree *inst,VideoData *image);
bool	Qtree_PrepImages(Qtree *inst);
void	Qtree_BackGroundComparison(Qtree *inst);
bool	Qtree_Compressor(Qtree *inst);
bool	Qtree_CompressY(Qtree *inst,unsigned char *image,unsigned char *last,
						unsigned char *backGround,int width,int height,int pitch,int bits);
int		Qtree_SetupBlocks (Qtree *inst,unsigned char *image,unsigned char *lastImage,
						unsigned char *backGround,int width,int height,int pitch);
bool	Qtree_FitBlocksToImage (Qtree *inst,int budget,float RDThreshold,bool recode);
void	Qtree_HuffmanEncodeImage(Qtree *inst);
bool	Qtree_DecompressY(Qtree *inst,unsigned char *image,unsigned char *last,
						unsigned char *backGround,int width,int height,int pitch);
void	Qtree_Recode(Qtree *inst);
void	Qtree_HuffmanDecodeImage(Qtree *inst);
bool	Qtree_SplitBlock(Qtree *inst,Block *parent);
void	Qtree_EncodeQtreeImage(Qtree *inst);
Block * Qtree_EncodeQtreeBlock(Huffman *qtree,Block *block,int size);
bool	Qtree_DecodeQtreeImage(Qtree *inst,unsigned char  *image,unsigned char *last,
						unsigned char *backGround,int width,int height,int pitch);
Block *	Qtree_DecodeQtreeBlock(Qtree *inst,unsigned char *image,unsigned char *last,
						unsigned char *backGround,Block *block,int x,int y,int size,int pitch);
Block *	Qtree_DecodeMinSizeQtreeBlocks(Qtree *inst,unsigned char *image,unsigned char *last,
						unsigned char *backGround,Block *block,int x,int y,int size,int pitch);
void	Qtree_UnquantiseImage(Qtree *inst);
void	Qtree_RenderImage(Qtree *inst);
Block *	Qtree_GetFreeBlock(Qtree *inst);
void	Qtree_FreeBlockList(Qtree *inst);
void	Qtree_DeleteListManager(Qtree *inst);
void	Qtree_FreeAllBlocks(Block *top);
unsigned int StringToInt(char *s,int length);
bool	Qtree_WriteCompressedFileHeader(Qtree *inst);
bool	Qtree_ReadCompressedFileHeader(Qtree *inst);
bool    Qtree_PostProcess(Qtree *inst,unsigned char *image,int width,int height,int pitch);

///////////////////////////////////////////////////////////////////////////////////////
//	block focus
///////////////////////////////////////////////////////////////////////////////////////

typedef struct _Focus
{
	int m_x;
	int m_y;
	int m_size;
	int m_scale;
    unsigned int m_inst;
}Focus;

Focus *	Focus_FindInst(unsigned int inst);
Focus *	Focus_Create(unsigned int inst);
void	Focus_Destroy(unsigned int inst);
void	Qtree_StartFocus(Qtree *inst);
void	Qtree_SetFocus(Qtree *inst,int size,int scale);
void	Qtree_NextFocus(Qtree *inst);

///////////////////////////////////////////////////////////////////////////////////////
// JPEG headers
///////////////////////////////////////////////////////////////////////////////////////

// JPEG marker codes
typedef enum {	
  M_SOF0  = 0xc0,
  M_SOF1  = 0xc1,
  M_SOF2  = 0xc2,
  M_SOF3  = 0xc3,
  
  M_SOF5  = 0xc5,
  M_SOF6  = 0xc6,
  M_SOF7  = 0xc7,
  
  M_JPG   = 0xc8,
  M_SOF9  = 0xc9,
  M_SOF10 = 0xca,
  M_SOF11 = 0xcb,
  
  M_SOF13 = 0xcd,
  M_SOF14 = 0xce,
  M_SOF15 = 0xcf,
  
  M_DHT   = 0xc4,
  
  M_DAC   = 0xcc,
  
  M_RST0  = 0xd0,
  M_RST1  = 0xd1,
  M_RST2  = 0xd2,
  M_RST3  = 0xd3,
  M_RST4  = 0xd4,
  M_RST5  = 0xd5,
  M_RST6  = 0xd6,
  M_RST7  = 0xd7,
  
  M_SOI   = 0xd8,
  M_EOI   = 0xd9,
  M_SOS   = 0xda,
  M_DQT   = 0xdb,
  M_DNL   = 0xdc,
  M_DRI   = 0xdd,
  M_DHP   = 0xde,
  M_EXP   = 0xdf,
  
  M_APP0  = 0xe0,
  M_APP1  = 0xe1,
  M_APP2  = 0xe2,
  M_APP3  = 0xe3,
  M_APP4  = 0xe4,
  M_APP5  = 0xe5,
  M_APP6  = 0xe6,
  M_APP7  = 0xe7,
  M_APP8  = 0xe8,
  M_APP9  = 0xe9,
  M_APP10 = 0xea,
  M_APP11 = 0xeb,
  M_APP12 = 0xec,
  M_APP13 = 0xed,
  M_APP14 = 0xee,
  M_APP15 = 0xef,
  
  M_JPG0  = 0xf0,
  M_JPG13 = 0xfd,
  M_COM   = 0xfe,
  
  M_TEM   = 0x01,
  
  M_ERROR = 0x100
} JPEG_MARKER;


/////////////////////////////////////////////////////////////////////////////////////////////

#define DCTSIZE2	64
#define DCTSIZE		8
#define CODE_SIZE   65536
#define ENCODE_SIZE   32768

typedef struct _JpegPlaneInfo
{
	int m_lastDCvalue;		// last DC value
	unsigned int m_EOB;
	int m_quant[DCTSIZE2];	// current Q table
	unsigned int m_DCCode[CODE_SIZE];
	unsigned int m_ACCode[CODE_SIZE];

} JpegPlaneInfo;


/////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _JpegInfo
{
	int m_quality;			   // current quality
	unsigned char m_topPtr[128000];
	unsigned char *m_packPtr;  // actual position in packed data
	unsigned int m_bits;	   // overflow	
	int m_dataSize;			   // size of data
	unsigned int m_activeBytes;	   // used to stored bytes
	int m_compSize;
	
	unsigned short m_lines;
	unsigned short m_samplesPerLine;

	JpegPlaneInfo m_Y;
	JpegPlaneInfo m_U;
	JpegPlaneInfo m_V;

} JpegInfo;

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char FindMarker(JpegInfo *jpeg);
void LoadJpeg(JpegInfo *jpeg,char *fileName);
bool ReadStartOfFrame(JpegInfo *jpeg);
bool ReadStartOfScan(JpegInfo *jpeg);
bool ReadQuantTable(JpegInfo *jpeg);
bool ReadHuffTable(JpegInfo *jpeg);
unsigned int CreateHuffmanTables(unsigned int *oHuffCode,unsigned char *bits,unsigned char *val,bool compressor,bool dc);
bool DecodeBlock(JpegInfo *jpeg,int *data,int type);
void ScaleQuant(int *quant,bool compress);
bool ReadScanData(JpegInfo *jpeg,VideoData *buf);
void NewVideoData(JpegInfo *jpeg,VideoData **video);
void FastIDCT(unsigned char *Y,int jump,int *data);
void FastDCT (unsigned char * Y,int jump,int *data,int *quant);
void QuantQualityScale(int quality,int *quant);
bool WriteStartOfScan(JpegInfo *jpeg);
bool WriteStartOfFrame(JpegInfo *jpeg);
bool WriteHuffTable(JpegInfo *jpeg,unsigned char type);
bool WriteQuantTable(JpegInfo *jpeg,unsigned char type);
bool WriteCompressedData(JpegInfo *jpeg,VideoData *video);
bool EncodeBlock(JpegInfo *jpeg,int *dataPtr,int type);

#define MCUY	0
#define MCUU	1
#define MCUV	2
#define PASS1_BITS  3
#define CONST_BITS  8
//compress
#define FIX_0_382683433  ((int)   98)		// FIX(0.382683433) 
#define FIX_0_541196100  ((int)  139)		// FIX(0.541196100) 
#define FIX_0_707106781  ((int)  181)		// FIX(0.707106781) 
#define FIX_1_306562965  ((int)  334)		// FIX(1.306562965) 

// decompress
#define FIX_1_082392200  ((int)  277)		// FIX(1.082392200) 
#define FIX_1_414213562  ((int)  362)		// FIX(1.414213562) 
#define FIX_1_847759065  ((int)  473)		// FIX(1.847759065) 
#define FIX_2_613125930  ((int)  669)		// FIX(2.613125930) 

#define MULTIPLY(var,const)  (((var) * (const))>>CONST_BITS)
#define IDESCALE(x,n)  ((int) ((x) + (1 << ((n)-1))) >> n)

//#define QUANT_DIVIDE		// uses crappy divides for quantiastion


///////////////////////////////////////////////////////////////////////////////////////
// external c functions
///////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif

VideoData *VideoData_Create(int pitch, int width, int height,bool cache,bool onlyY);
void VideoData_Destroy(VideoData **inst);
void VideoData_Copy(VideoData **destPtr,VideoData *src);
void VideoData_Set(VideoData *inst,unsigned char value);
bool SetupSystemTime(SystemTime *inst);
void DeReferenceSystemTime(SystemTime *inst,SystemTime *ref);

Qtree *	Qtree_Create();
void	Qtree_Destroy(Qtree **inst);
void	Qtree_BackGroundRecode(Qtree *inst);
void	Qtree_ClearBackGround(Qtree *inst);
void	Qtree_SetCompression(Qtree *inst,float bpp);
void	Qtree_SetFrameSize(Qtree *inst,unsigned int size);
void	Qtree_SetMSEThreshold(Qtree *inst,float mse);	
void	Qtree_SetRateThreshold(Qtree *inst,float rate);
bool	Qtree_CompressImage(Qtree *inst,VideoData *image);
bool	Qtree_DecompressImage(Qtree *inst,unsigned char *pt,int size);
void	Qtree_DecompressData(Qtree *inst,unsigned char *pt,int size);

JpegInfo *CreateJpegInfo();
bool JpegEncodeSetup(JpegInfo *jpeg,int quality);
bool JpegDecode(JpegInfo *jpeg,VideoData **video);
bool JpegEncode(JpegInfo *jpeg,VideoData *video);

#ifdef __cplusplus
}
#endif


#endif