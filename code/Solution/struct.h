#ifndef STRUCT_H
#define STRUCT_H

#include "bitstrm.h"  // needed for definition in IMAGE

/* structure definitions */

typedef struct {
		float bpp; /* bits per pixel that quant table references to */
        int size;
        int *q_out; /* integer qunatisation values */
        float *q; /* stores exact values */
	    int *nitems; /* number of occurences */
} QUANT;

typedef struct {
        int nitems;
        float *e,*x;
        float *de,*dx;
        int *q;
        int max;
} COEFF_PLANES;

typedef struct  block_data {
		int *pt; /* pointer to uncompressed wavelet tree data */	
		int size; /* size of uncompressed wav tree */
		int dump_size; /* size of compressed data */
        int n_compressed_bits; /* number of bits, starting at dump_size */
		int x,y;	/* position of tree inside wavelet */
		int error;
		int transmit; /*0 for untransmitted -> 1-N (transmit lower the 
											number higher priority */
		int untransmitted; /* used to control refresh always sent after
						   N(50 ~2sec) frames untransmitted*/
		struct block_data *next; /* next tree in linked list */
} BLOCK;

class IMAGE {	 // structure with constructor
public:
		IMAGE(float bpp,int c,int r); 
		~IMAGE();
	
		int *pt;        /* image data  - 32 bit signed */
		int columns;    /* image width - pixels */
		int rows;       /* image height - pixels */
		BLOCK *tree_top; /* top of linked list to wavlet tree/blocks */
		BLOCK *transmitted; /* transmitted image , tree list */
		COEFF_PLANES *data; /* quantised coefficent data */
		SimpleBitStream dump;  /* bitstream that compressed image is stored in */
};

typedef struct {
        double *c; /* filters */
        double *u; /* filters */

        double *u_pm; // *u +- 

        double *memLow;  // Memory buffers used in the filters
        double *memHigh; // Stored here to save mallocs

        int cmax,cmin;
        int umax,umin;
	       int shift;
} WAVELET;

/*
  As things stand, the WAVELET struct can _ONLY_ be created by reading from
  a file (as the data in the file determines how large the memory allocations
  are going to be), so we can not use it to hold the memory required by
  the hard-coded wavelet.
  Because we can not modify these structs at the moment - or change the
  calling sequence for the original filter calls - struct HARDCODED_WAVELET
  has been added in as a dummy & the hard-coded wavelet function changed
  to accept on of these...

  BTW, there is no need for either wavelet structure to be declared within
  a header (nothing that needs to know the contents of the structure needs
  to be outside filters.c).
*/
typedef struct {
        double *memLow;  // Memory buffers used in the filters
        double *memHigh; // Stored here to save mallocs
} HARDCODED_WAVELET;

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

/* huffman stream */
typedef struct {
    
        /* huffman data */
        LEAF *huff; /* huff data - linear*/
        LEAF *huff_top; /* top of huff tree -for coding*/
        int max,min,run; /* max/ min indexs covered + runlength */
        int context; /* last symbol to be compressed by this stream */
} HUFF;


#endif /* STRUCT_H */
