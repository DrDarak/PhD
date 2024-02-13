#ifndef H_STRUCT_H
#define H_STRUCT_H

/* global defines */
//#define DDCT
#define MIN_SIZE 4
#define VQ_ON

#ifndef DDCT
     #define Q 4.0

	/* mutally exclusive*/
        #ifndef VQ_ON
     		//#define FRACTAL  /* ifdef -> engages fractals */
	#endif

	#ifdef FRACTAL
	       #define BFT /* uses bft version */
	       #ifdef BFT
	       	    #define FILE_TYPE "BFTR"
	       	    #define FILE_SUFFIX ".bft"
	       #else
		    //#define RD_SWITCH
	       	    #define FILE_TYPE "IFTR"
	       	    #define FILE_SUFFIX ".ift"
	       #endif
	  
	#else
	       #define FILE_TYPE "LDCT"
	       #define FILE_SUFFIX ".lqt"
	#endif
#else
	#define Q 8.0
	#define FILE_TYPE "DDCT"
	#define FILE_SUFFIX ".ddt"
#endif


/* structure definitions */

/* Block definitions */
typedef struct block_data {
        int *pt; /* pointer to loactaion in image data */
        int x,y;
	int jump; /* jumps to next line in block through image pt */
        int size; /* size of block - assumes square */
        float *c;   /* basis functions */
        int nitems; /* may not be neccessary*/
        int cf;  /* fractal coefficent */
        int vq;  /* vq index */
        float error;
        struct block_data *next;
} BLOCK;

/* image structure */
typedef struct {
        int *pt;        /* image data  */
	BLOCK *tree_top; /* pointer to the top of the linked
                                     list of image blocks*/
        int columns;    /* image width */
        int rows;       /* image height */
        int greylevels; /* max grey level */
} IMAGE;

/* node of huffman coder */
typedef struct node {

        int index;  /* number represented */
        int nitems; /* number of occurence of index */
        int symbol; /* symbol for huffman codeing*/
        int length; /* length of symbol */
	int run;    /* 0 not run 1 run */
        struct node *next; /* used by huffman coder */
        struct node *orig; /* original value */
        struct node *zero_pt; /* zero pt in tree */
        struct node *one_pt;  /* one pt in tree */


} LEAF;

/* packed data structure */
typedef struct {

        unsigned char  sym_mask;     /* puts bits to right point in integer */
        unsigned char  *symbols;     /* buffer of symbol bits */
        unsigned char *symbols_top;  /* top of symbol buffer */

	int bits; /* bits pack stream currently holds */
	int max_bits; /* max bits packed stream can hold */

} PACKED;

/* huffman stream */
typedef struct {
	/* packed stream */
	PACKED *dump;

	/* huffman data */
        LEAF *huff; /* huff data - linear*/
        LEAF *huff_top; /* top of huff tree -for coding*/
        int max,min,run; /* max/ min indexs covered + runlength */
	int context; /* last symbol to be compressed by this stream */
} HUFF;

typedef struct basis_data {
        float *pt;
        float sum;
        int size;
        float q;
        struct basis_data *next;  /* next basis vector */
        struct basis_data *lower; /* indicates lower size
                                     - only vaild for root pointers*/
        HUFF *huff;
        int min,max;  /* min/max size of quantised coefficients */
        int nitems;  /* number of basis function in lower root*/
  	int x,y; 
} BASIS;

typedef struct zoom_data {
	int x,y;
	int width,height;
	float factor;  /* zoom factor 0.25, 0.5, 1, 2, 4 */
	int flag; /* usually 0 ... 1 indicates zoom */
} ZOOM;

#endif  /* H_STRUCT_H*/
