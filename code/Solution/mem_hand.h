#ifndef MEM_HAND_H
#define MEM_HAND_H

/* memory handleing header */
#include "struct.h"

/*
  Routines defined within mem_hand.c:
*/
IMAGE *malloc_IMAGE(int size, char *loc);
void free_IMAGE(IMAGE *pt);

QUANT *malloc_QUANT(int size, char *loc);
void free_QUANT(QUANT *pt);

HUFF *malloc_HUFF(int size, char *loc);
void free_HUFF(HUFF *pt);

LEAF *malloc_LEAF(int size, char *loc);
void free_LEAF(LEAF *pt);

COEFF_PLANES *malloc_COEFF_PLANES(int size, char *loc);
void free_COEFF_PLANES(COEFF_PLANES *pt);

BLOCK *malloc_BLOCK(int size, char *loc);
void free_BLOCK(BLOCK *pt);

WAVELET *malloc_WAVELET(int size, char *loc);
void free_WAVELET(WAVELET *pt);

int *malloc_int(int size, char *loc);
void free_int(int *pt);

unsigned char *malloc_unsigned_char(int size, char *loc);
void free_unsigned_char(unsigned char *pt);

float *malloc_float(int size, char *loc);
void free_float(float *pt);

double *malloc_double(int size, char *loc);
void free_double(double *pt);

void NULL_FILE_error(FILE *pt, char *loc);

#endif /* MEM_HAND_H */
