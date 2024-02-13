/* Image input/ouput functions */
#include "struct.h"

/* Functions used externally */
void get_ppm_file(FILE *,IMAGE *,IMAGE *,IMAGE *);
void write_ppm_file(IMAGE *,IMAGE *,IMAGE *,char *);
void get_pgm_file(FILE *,IMAGE *);
void write_pgm_file(IMAGE *, char *);
FILE *read_raw_header (char *, int *, int *,int * ); 


/* Internal functions */

/* External functions */
extern void NULL_FILE_error(FILE *,char *);
extern int *malloc_int(int ,char *);
extern void free_int(int *);


/* defines */

/* macros */
#define limit(x) ((x) > 255 ? 255 : (x < 0 ? 0 :x))
