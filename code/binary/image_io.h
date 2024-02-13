/* Image input/ouput functions */
#ifndef H_IMAGE_IO_H
#define H_IMAGE_IO_H

#include "struct.h"

/* Functions used externally */
int read_pgm_file (FILE *, IMAGE *, int );
void write_pgm_file(FILE *,IMAGE *,int);
FILE *read_pgm_header (char *,IMAGE * ); 
FILE *write_pgm_header (char *,IMAGE *); 

#endif
