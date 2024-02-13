/* Huffman coder header */
#ifndef H_HUFF_H
#define H_HUFF_H

#include "struct.h"

/* Functions used externally */
void setup_huffman_tree (HUFF *,PACKED *, FILE *);
void free_huffman_tree (HUFF *);
void wipe_huffman_pdf (HUFF *, FILE *);
void save_huffman_pdf (HUFF *, FILE *);
LEAF * construct_huff_tree (LEAF *);
int compress_symbol (HUFF *,int);
int estimate_compress_symbol (HUFF *, int);
int compress_run (HUFF *, int );
int estimate_compress_run (HUFF *, int);
int uncompress_symbol (HUFF *);
float huffman_bpc (HUFF *);
int uncompress_run_symbol (HUFF *,int *); 

#endif  /* H_HUFF_H*/
