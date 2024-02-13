#ifndef H_QTREE_CODEC_H
#define H_QTREE_CODEC_H


///////////////////////////////////////////////////////////////
//					Quad tree codec							 //
//						David Bethel						 //
//					Darak@compuserve.com					 //
///////////////////////////////////////////////////////////////
#include "codec.h"
#include "iobuff.h"
#include "image.h"
#include "blocks.h"
#include "list.h"
#include "huff.h"

typedef enum huff_type {normal,wipe,append} HUFF_TYPE; 

class QtreeCodec: public Codec {
public:

	QtreeCodec();
	~QtreeCodec();

	bool SetupPDF(char *pdfFname=NULL);
	bool SetFileNames(char *inputFname,char *outputFname,char *pdfFname);

	bool CompressY(ImageData *image,ImageData *last,int bits);
	
	int  SetupBlocks(ImageData *image,ImageData *lastImage=NULL);
	bool FitBlocksToImage (int budget);
	void HuffmanEncodeImage();

	bool DecompressY(ImageData *image,ImageData *last);

	void HuffmanDecodeImage();

	bool AppendPDF();
	bool WipePDF();

	int m_size;
    Huffman *m_qtree;
	Huffman *m_coeff[MAX_COEFF];
	Block *m_top; // pointer to the top of the block list

	char m_pdfFname[256];
    
	HUFF_TYPE m_type;

	// quadtree type files
	bool SplitBlock(Block *parent);
	void AppendBlockToList(Block *first,Block *second);
	void EncodeQtreeImage();
	Block *EncodeQtreeBlock(Block *block,int size);
	bool DecodeQtreeImage(ImageData *image,ImageData *last);
	Block *DecodeQtreeBlock(ImageData *image,ImageData *last,
		         Block *block,int x,int y,int size);
	void UnquantiseImage();
	void RenderImage();

	// free list managment
	Block *m_spareBlocks;
	void DeleteListManager();
	void FreeBlockList();
	Block *GetFreeBlock();
	void FreeAllBlocks(Block *top);

	// list sorting
	BuffList m_list;

};

#endif