/* memory handleing header */
#include "struct.h"

/* Functions used externally */
void allocate_dump (PACKED *,int );
void write_compressed_file (FILE *, IMAGE *, HUFF *, int );
void read_compressed_file (FILE *, IMAGE *,  HUFF *, int );


/* Internal functions */

/* External functions */
extern int *malloc_int(int ,char *);
extern void free_int (int *);

extern unsigned char *malloc_unsigned_char(int ,char *);
extern void free_unsigned_char(unsigned char *);

extern float huffman_bpc (HUFF *);

extern float *malloc_float (int , char *);
extern void free_float (float *);

extern void NULL_FILE_error(FILE *,char *);

extern HUFF *malloc_HUFF (int, char *);
extern void free_HUFF (HUFF *);

/* defines */

/* macros */
