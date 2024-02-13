#ifndef H_STRUCT_H
#define H_STRUCT_H

/* Gobal defines */
#define MIN_SIZE 3

/* structure definitions */
/* Block definitions */
typedef enum block_type  {none,black,white,vq} BLOCK_TYPE; 

typedef struct block_data {
        int *pt; /* Block data */
        int x,y;
        int size;
	int index;
	enum block_type type;
        struct block_data *next;
} BLOCK;

/* image structure */
typedef struct {
        unsigned char *pt;        /* image data  */
	BLOCK *tree_top; /* pointer to the top of the linked
                                     list of image blocks*/
        int columns;    /* image width */
        int rows;       /* image height */
} IMAGE;

/* node of huffman coder */
typedef struct node {

        int index;  /* number represented */
        int nitems; /* number of occurence of index */
        int symbol; /* symbol for huffman codeing*/
        int length; /* length of symbol */
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

        LEAF *huff; /* huff data - linear*/
        LEAF *huff_top; /* top of huff tree -for coding*/
        int max,min; /* max/ min indexs covered */
	int bits; /* bits pack stream currently holds */
	int max_bits; /* max bits packed stream can hold */

} PACKED;

#endif
