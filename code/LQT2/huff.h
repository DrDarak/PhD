/* Huffman coder header */
#ifndef H_HUFF_H
#define H_HUFF_H

#include "IOBuff.h"

// defines / macros 
#define MAX_RUN 16
#define myabs(x)    ( x < 0 ? -x : x)

/* node of huffman coder */
class Leaf {
public:
	Leaf();
	~Leaf();
	Leaf(int nitems,int index);
	void SetupLeaf(int nitems,int index);

	int m_index;  // number represented 
	int m_nitems; // number of occurence of index 
	int m_symbol; // symbol for huffman codeing
	int m_length; // length of symbol
	int m_run;    // 0 not run 1 run 
	Leaf *m_next; // used by huffman coder 
	Leaf *m_orig; // original value 
	Leaf *m_zeroPt; // zero pt in tree 
	Leaf *m_onePt;  // one pt in tree 
};

// huffman stream 
class Huffman 
{
public:
	Huffman();
	~Huffman();

	// Functions 
	bool SetupHuffmanTree(IOBuff *dump,FILE *fp);
	bool WipeHuffmanPDF(FILE *fp);
	bool SaveHuffmanPDF(FILE *fp);
	bool TraverseTree(Leaf *huffPt,int symbol,int length);
	float HuffmanBPC();
	int CompressSymbol(int symbol);
	int EstimateCompressSymbol(int symbol);
	int EstimateCompressRun(int symbol);
	int UncompressSymbol();
	int UncompressRunSymbol(int &run);
	int CompressRun(int symbol);
	
	// packed stream 
	IOBuff *m_dump;

	// huffman data 
	Leaf *m_huff;      // huff data - linear
    Leaf *m_huffTop;  // top of huff tree -for coding
    int m_max; // max coeff
	int m_min; // min coeff
	int m_run; // runlength - max run
	int m_context; // last symbol to be compressed by this stream 
	typedef enum pdf_type {normal,blank} PDF_TYPE;

protected:
	void FreeTree (Leaf *huff);
	bool ConstructHuffTree();
	bool DeleteElement(Leaf *del_pt);
	Leaf * SearchList();


};

#endif  /* H_HUFF_H*/
