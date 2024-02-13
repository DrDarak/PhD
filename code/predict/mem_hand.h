#ifndef MEMHAND_H
#define MEMHAND_H

/* memory handleing header */
#include "struct.h"

/* Functions used externally */
IMAGE *malloc_IMAGE(int ,char *); 
void free_IMAGE(IMAGE *);
WAVELET *malloc_WAVELET(int ,char *);
void free_WAVELET(WAVELET *);
int *malloc_int(int ,char *);
void free_int (int *pt);
unsigned char *malloc_unsigned_char(int ,char *);
void free_unsigned_char(unsigned char *);
float *malloc_float (int , char *);
void free_float (float *);
void NULL_FILE_error(FILE *,char *);

#endif
