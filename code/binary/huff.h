/* Huffman coder header */
#ifndef H_HUFF_H
#define H_HUFF_H

#include "struct.h"

/* Functions used externally */
void setup_huffman_tree (PACKED *, FILE *);
void free_huffman_tree (PACKED *);
void wipe_huffman_pdf (PACKED *, FILE *);
void save_huffman_pdf (PACKED *, FILE *);
LEAF * construct_huff_tree (LEAF *);
int compress_symbol (PACKED *,int);
int uncompress_symbol (PACKED *);
float huffman_bpc (PACKED *);
void free_tree (LEAF *);

#endif
