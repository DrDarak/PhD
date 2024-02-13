/* Huffman coder header */
#include "struct.h"

/* Functions used externally */
void setup_huffman_tree (HUFF *,PACKED *, FILE *);
void free_huffman_tree (HUFF *);
void wipe_huffman_pdf (HUFF *, FILE *);
void save_huffman_pdf (HUFF *, FILE *);
LEAF * construct_huff_tree (LEAF *);
int compress_symbol (HUFF *,int);
int compress_run (HUFF *, int );
int uncompress_symbol (HUFF *);
int uncompress_run_symbol (HUFF *, int *);

float huffman_bpc (HUFF *);

/* Internal functions */
inline int input_bit (PACKED *);
void output_bit (PACKED *, int);
void free_tree (LEAF *);
void traverse_tree (LEAF *, int , int );
void setup_leaf (LEAF *, int , int );
LEAF * allocate_leaf_mem (int , int );
LEAF * delete_element (LEAF *, LEAF *);
LEAF * search_list (LEAF *);
int uncompress_stream (PACKED *, LEAF *);


/* External functions */
extern LEAF *malloc_LEAF(int ,char *);
extern void free_LEAF(LEAF *);
extern void NULL_FILE_error(FILE *,char *);

/* defines */
#define MAX_RUN 16

/* macros */
#define myabs(x)    ( x < 0 ? -x : x)


