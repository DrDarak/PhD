/* File input/output heder header */
#ifndef H_FILE_IO_H
#define H_FILE_IO_H

#include "struct.h"

/* Functions used externally */
void allocate_dump (PACKED *,int );
void write_compressed_file (FILE *, IMAGE *, PACKED *, int );
int read_compressed_file_header(FILE *,IMAGE *);
void  read_compressed_file (FILE *, PACKED *);

#endif
