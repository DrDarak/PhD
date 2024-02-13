/* optimal quantisation header */
#include "struct.h"

/* Functions used externally */
void form_quantisation_tables (IMAGE *, int );
void quantise_coefficients (IMAGE *,int );
int find_quantisation (float , COEFF_PLANES *); 
void unquantise_image (IMAGE *, int *, int );
int quantise_image (IMAGE *, int *, double *, int);
int ammend_quantisation (IMAGE *, int *, HUFF *, HUFF *, int);
int huffman_estimate_image (IMAGE *, int *, HUFF *, HUFF *, float *, float *);
void huffman_encode_image (IMAGE *, int *, HUFF *, HUFF *);
void huffman_decode_image (IMAGE *, int *, HUFF *, HUFF *);
void read_quantisation (char *, QUANT *);
void write_quantisation  (char *, QUANT *);

/* Internal functions */
inline int quantise (int ,int );  
inline void huffman_encode_block (BLOCK *, int *, HUFF *,HUFF *);
inline void huffman_decode_block (BLOCK *block, int *, HUFF *, HUFF *);

void RD_curve_by_entropy (IMAGE *, COEFF_PLANES *, int );
int max_coeff (BLOCK *, int );
void convert_bits_to_quant (IMAGE *, float *, int *, int );
int find_no_ac_symbols (IMAGE *, int *, float *, int , int );
void max_min_quant_coeff (BLOCK *, int , int *, int *);

/* External functions */
extern COEFF_PLANES *malloc_COEFF_PLANES (int , char *);
extern void free_COEFF_PLANES (COEFF_PLANES *);
extern int *malloc_int(int ,char *);
extern void free_int (int *pt);
extern float *malloc_float (int , char *);
extern void free_float (float *);
extern void NULL_FILE_error(FILE *,char *);

extern int compress_symbol (HUFF *,int);
extern int compress_run (HUFF *, int );
extern int uncompress_symbol (HUFF *);
extern int uncompress_run_symbol (HUFF *, int *);

extern void allocate_mem_coeff_planes (COEFF_PLANES *, int );

/* defines */
#define QUANT_RANGE 10

/* macros */
#define myabs(x)    ( x < 0 ? -x : x)

