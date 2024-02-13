#ifndef HUFF_H
#define HUFF_H
/* Huffman coder header */
#include "struct.h"

/* Routines defined within huff.c: */
void setup_huffman_tree(HUFF *huff,
                        FILE *fp);

void wipe_huffman_pdf(HUFF *huff,
                      FILE *fp);
void save_huffman_pdf(HUFF *huff,
                      FILE *fp);

void free_huffman_tree(HUFF *huff);

int compress_symbol(HUFF *huff_stream,
					BitStream &dump,
                    int symbol);
int compress_run(HUFF *huff_stream,
				 BitStream &dump,
                 int symbol);
int uncompress_symbol(HUFF *huff_stream,
					  BitStream &dump);
int uncompress_run_symbol(HUFF *huff_stream,
						  BitStream &dump,
                          int &run);
int estimate_compress_run(HUFF *huff_stream,
						  int symbol);
int estimate_compress_symbol(HUFF *huff_stream,
							  int &symbol);


#endif /* HUFF_H */
