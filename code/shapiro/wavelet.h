/* main wavelet functions */
#include "struct.h"

/* Functions used externally */

/* Internal functions */
void wavelet_compress(IMAGE *,WAVELET *,PACKED *,int );

/* External functions */
extern void initialize_arithmetic_encoder(SYMBOL *);
extern void initialize_arithmetic_decoder(PACKED *);

extern void zero_tree_code_image(IMAGE *,PACKED *,int );
extern void zero_tree_decode_image (IMAGE *,PACKED *,int );
extern void clear_markers(IMAGE *);

extern void setup_wavelet (char *, WAVELET *);
extern void wavelet_inv_filter_image (IMAGE *, WAVELET *,int );
extern void wavelet_filter_image (IMAGE *, WAVELET *,int );

extern void get_pgm_file(IMAGE *, char *);
extern void write_pgm_file(IMAGE *, char *);
extern void get_ppm_file(IMAGE *,IMAGE *,IMAGE *,char *);
extern void write_ppm_file(IMAGE *,IMAGE *,IMAGE *,char *);

extern IMAGE *malloc_IMAGE(int ,char *); 
extern void free_IMAGE(IMAGE *);
extern WAVELET *malloc_WAVELET(int ,char *);
extern void free_WAVELET(WAVELET *);
extern PACKED *malloc_PACKED(int ,char *);
extern void free_PACKED(PACKED *);
extern SYMBOL *malloc_SYMBOL(int ,char *);
extern void free_SYMBOL(SYMBOL *);
extern int *malloc_int(int ,char *);
extern void free_int (int *pt);
extern unsigned char *malloc_unsigned_char(int ,char *);
extern void free_unsigned_char(IMAGE *);
extern void NULL_FILE_error(FILE *,char *);

extern void write_compressed_file(FILE *, IMAGE *, PACKED *);
extern void allocate_dump(PACKED *, int ); 
extern void read_compressed_file(FILE *, IMAGE *, PACKED *);

/* nefines */

/* macros */
