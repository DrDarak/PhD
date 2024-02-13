/* memory handleing header */
#ifndef H_MEM_HAND_H
#define H_MEM_HAND_H

#include "struct.h"

/* Functions used externally */
IMAGE *malloc_IMAGE(int ,char *); 
void free_IMAGE(IMAGE *);

BASIS *malloc_BASIS (int , char *);
void free_BASIS (BASIS *);

BLOCK *malloc_BLOCK (int , char *);
void free_BLOCK (BLOCK *);

PACKED *malloc_PACKED(int ,char *);
void free_PACKED(PACKED *);

LEAF *malloc_LEAF(int ,char *);
void free_LEAF(LEAF *);

ZOOM *malloc_ZOOM (int , char *); 
void free_ZOOM (ZOOM *);

HUFF *malloc_HUFF (int, char *); 
void free_HUFF (HUFF *);   

int *malloc_int(int ,char *);
void free_int (int *);

unsigned char *malloc_unsigned_char(int ,char *);
void free_unsigned_char(unsigned char *);

float *malloc_float (int , char *);
void free_float (float *);

void NULL_FILE_error(FILE *,char *);

#endif  /* H_MEM_HAND_H*/
