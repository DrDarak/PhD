#ifndef OPTIM_Q_H
#define OPTIM_Q_H
/* optimal quantisation header */ 
#include "struct.h"
#include "bitstrm.h"

/*
  Routines defined within optim_q.c:
*/
void read_quantisation(char *fname, QUANT *quant);
void huffman_encode_image(IMAGE *image,
                          int *q_out,
                          HUFF *dc,
                          HUFF *ac);
void huffman_decode_image(IMAGE *image,
                          int *q_out,
                          HUFF *dc,
                          HUFF *ac);
void huffman_decode_block(BLOCK *block,
                          int *q_out, 
                          HUFF *dc, 
                          HUFF *ac,
						  BitStream &dump);
int find_jump_position(int x);
int check_for_jump(BLOCK *block,int *q_out,int x);

#endif /* OPTIM_Q_H */
