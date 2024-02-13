#ifndef FILEIO_H
#define FILEIO_H

/* Image input/ouput functions header */
#include "struct.h"

/* Functions used externally */
void write_compressed_file(FILE *, IMAGE *, PACKED *);
void allocate_dump(PACKED *, int ); 
void read_compressed_file(FILE *, IMAGE *, PACKED *);
FILE *write_compressed_file_header(char *, int, int, int);
FILE *read_compressed_file_header(char *, int *, int *, int *);

#endif
