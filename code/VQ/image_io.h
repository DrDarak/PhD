/* Image input/ouput functions */
#ifndef H_IMAGE_IO_H
#define H_IMAGE_IO_H

#include "struct.h"

/* Functions used externally */
void get_ppm_file(FILE *,IMAGE *,IMAGE *,IMAGE *);
void write_ppm_file(IMAGE *,IMAGE *,IMAGE *,char *);
void get_pgm_file(FILE *,IMAGE *);
void write_pgm_file(IMAGE *, char *);
FILE *read_raw_header (char *, int *, int *,int * ); 

#endif  /* H_IMAGE_IO_H*/
