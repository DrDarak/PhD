/* memory handleing header */
#include "struct.h"

/* Functions used externally */
COEFF_PLANES *malloc_COEFF_PLANES (int , char *);
void free_COEFF_PLANES (COEFF_PLANES *);

QUANT *malloc_QUANT (int , char *);
void free_QUANT (QUANT *); 

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

PACKED *malloc_PACKED(int ,char *);
void free_PACKED(PACKED *);

LEAF *malloc_LEAF(int ,char *);
void free_LEAF(LEAF *);

BLOCK *malloc_BLOCK(int ,char *);
void free_BLOCK(BLOCK *);

HUFF *malloc_HUFF (int, char *);
void free_HUFF (HUFF *);

void NULL_FILE_error(FILE *,char *);

/* Internal functions */

/* External functions */

/* defines */

/* macros */
