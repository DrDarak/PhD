/* main wavelet functions */
#include "struct.h"

/* Functions used externally */

/* Internal functions */

/* External functions */
extern IMAGE *malloc_IMAGE(int ,char *); 
extern void free_IMAGE(IMAGE *);
extern WAVELET *malloc_WAVELET(int ,char *);
extern void free_WAVELET(WAVELET *);
extern int *malloc_int(int ,char *);
extern void free_int (int *pt);
extern unsigned char *malloc_unsigned_char(int ,char *);
extern void free_unsigned_char(IMAGE *);
extern void NULL_FILE_error(FILE *,char *);
extern float *malloc_float (int , char *);
extern void free_float (float *);
extern PACKED *malloc_PACKED(int ,char *); 
extern void free_PACKED(PACKED *); 
extern HUFF *malloc_HUFF (int, char *); 
extern void free_HUFF (HUFF *); 
QUANT *malloc_QUANT (int , char *);
void free_QUANT (QUANT *);

extern void allocate_dump (PACKED *,int );

extern void setup_huffman_tree (HUFF *,PACKED *, FILE *);
extern void free_huffman_tree (HUFF *);
extern void wipe_huffman_pdf (HUFF *, FILE *);
extern void save_huffman_pdf (HUFF *, FILE *);

extern void wavelet_inv_filter_image (IMAGE *,int);
extern void wavelet_filter_image (IMAGE *,int);

extern void get_ppm_file(FILE *,IMAGE *,IMAGE *,IMAGE *);
extern void write_ppm_file(IMAGE *,IMAGE *,IMAGE *,char *);
extern void get_pgm_file(FILE *,IMAGE *);
extern void write_pgm_file(IMAGE *, char *);
extern FILE *read_raw_header (char *, int *, int *,int * );

extern void solve (COEFF_PLANES *, float *,float ,float ,int);

extern void form_quantisation_tables (IMAGE *, int );
extern void quantise_coefficients (IMAGE *,int );
extern void convert_bits_to_quant (IMAGE *, float *, int *, int );
extern int ammend_quantisation (IMAGE *, int *, HUFF *, HUFF *, int);
extern void huffman_encode_image (IMAGE *, int *, HUFF *, HUFF *);
extern void huffman_decode_image (IMAGE *, int *, HUFF *, HUFF *);
extern void read_quantisation (char *, QUANT *);
extern void write_quantisation  (char *,QUANT *); 

extern void setup_blocks(IMAGE *,int);
extern void free_blocked_image (IMAGE *);
extern void reform_image (IMAGE *, int );

extern void unquantise_image (IMAGE *, int *, int );
extern int quantise_image (IMAGE *, int *, double *, int);

/* nefines */
#define myabs(x)    ( x < 0 ? -x : x)

/* macros */
