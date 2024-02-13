#ifndef H_FILE_IO_H
#define H_FILE_IO_H

#include "struct.h"

/* Functions used externally */
void allocate_dump (PACKED *,int );
void allocate_packed_streams (BASIS *,HUFF *,int , int );
FILE *write_compressed_file_header (char *, int , int  rows, int );
void write_compressed_file (FILE *, IMAGE *, BASIS *, HUFF *, int );
FILE *read_compressed_file_header (char *, int  *, int  *, int  *);

void read_compressed_file (FILE *, IMAGE *, BASIS *, HUFF *, int );

#endif  /* H_BLOCKS_H*/

