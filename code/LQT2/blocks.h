#ifndef H_BLOCKS_H
#define H_BLOCKS_H

#include "image.h"
#include "huff.h"

#define MAX_COEFF 6
#define Q 2.0
#define MAX_SIZE 32
#define myabs(x)    ( x < 0 ? -x : x)
#define rint(x) (x > 0 ? (int)(x+0.5) : (int)(x-0.5) )


// Block definitions 
class Block {
public:

	Block(); // constructor / deconstructor
	~Block(); 

	bool SetupBlock(int x,int y,int size,ImageData *current,
					ImageData *last=NULL);
	bool SetupBlock(int x,int y,int size,Block *parent);

	bool RenderBlock(); // renders block with approprate rendering alg
	bool CodeBlock();	// codes block wit appropiate encoder

	int CalcLastBlockError(); // calculates Error of current frame to 
							  // last frame block
	int CalcCurrentBlockError(); // calculates Error of current frame
								 // to coded current frame;
	void UnquantiseBlock();	// unquantises blocks
	void QuantiseBlock();	// quantises blocks
	bool CopyLastFrame();

	// member data
	int *m_currentFrame; // pointer to loactaion in image data 
	int *m_lastFrame; // pointer to location of image data from last frame
	int m_x;	// x postion inside image
	int m_y;	// y position in image
	int m_jump; // jumps to next line in block through image pt 
    int m_size; // size of block - assumes square 
    float m_c[MAX_COEFF];   // DCT coefficients
    int m_error; // Squared Error sum for block;

	//flags
	bool m_isLastFrame;		// used to indicate that last frame exists
	bool m_useCurrentFrame; // used to pick which frame is been 
								 // used. Normally current
	// linked list pointers
    Block *m_next;
	Block *m_sortNext;

	int HuffmanEncodeBlock(Huffman **coeff);
	int HuffmanEstimateBlock(Huffman **coeff);
	void HuffmanDecodeBlock(Huffman **coeff);

	static const float c_q[MAX_SIZE+1];

protected:
	// protected member functions
	void Render2x2m4();
	void Render4x4m6();
	void Render8x8m6();
	void Render16x16m6();
	void Render32x32m6();	

	void Code2x2m4();
	void Code4x4m6();
	void Code8x8m6();
	void Code16x16m6();
	void Code32x32m6();

} ;

#endif  /* H_BLOCKS_H*/
