#define END_OF_STREAM 4  /* end of stream symbol */

/* structure definitions */

typedef struct {
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
	int *pt;	
	int x,y;
	int size;
	struct block_data *next;
} BLOCK;

typedef struct {
        int *pt;        /* image data  */
        int columns;    /* image width */
        int rows;       /* image height */
	BLOCK *tree_top;
	COEFF_PLANES *data; /* quantised coefficent data */
} IMAGE;

typedef struct {
        float *c;
        float *u;
        int cmax,cmin;
        int umax,umin;
	int shift;
} WAVELET;


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


