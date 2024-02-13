/*huffman image handler header */
#ifndef H_IMG_HUFF_H
#define H_IMG_HUFF_H

#include "struct.h"

/* Functions used externally */
void setup_huffman_coder (BASIS *,HUFF *,HUFF *,char *);
void huffman_encode_image (IMAGE *, BASIS *,HUFF *);
int huffman_encode_block (BLOCK *, BASIS *);
int huffman_estimate_block (BLOCK *, BASIS *);
void huffman_decode_image (IMAGE *, BASIS *,HUFF *);
void huffman_decode_block (BLOCK *, BASIS *);
void save_image_pdf(BASIS *,HUFF *,HUFF *,char *);
void wipe_image_pdf(BASIS *,HUFF *,HUFF *,char *);
void shutdown_huffman_coder (BASIS *,HUFF *, HUFF *);

#endif  /* H_IMG_HUFF_H*/
