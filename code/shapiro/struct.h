#ifndef STRUCT_H
#define STRUCT_H

#define END_OF_STREAM 4  /* end of stream symbol */

/* structure definitions */

/* Structure IMAGE to hold details about an image */
typedef struct {
        int *pt;        /* Pointer to image data  */
        int columns;    /* image width in pixels */
        int rows;       /* image height in pixels*/
} IMAGE;

/* Structure WAVELET to hold details about the wavelet filters */
typedef struct {
        float *c; /* Points to 0th analysis LPF coefft */
        float *u; /* Points to 0th synthesis LPF coefft */
        int cmax,cmin; /* Min and max for numbering analysis LPF coeffts */
        int umax,umin; /* Min and max for numbering synthesis LPF coeffts */
	int shift;
} WAVELET;

/* Data structure holding info used in entropy coding */
typedef struct {

        int context;

        unsigned short low_count;
        unsigned short high_count;
        unsigned short scale;

        int *totals[END_OF_STREAM+1]; /*context tables pointers    */

        unsigned short code;  /* The present input code value       */
        unsigned short low;   /* Start of the current code range    */
        unsigned short high;  /* End of the current code range      */

        long underflow_bits;  /* Number of underflow bits pending   */

} SYMBOL;

/* Data structure holding info used in entropy coding */
typedef struct {
        int bits;       /* number of bits stored */
        int n_bits;     /* max number bits - comp ratio*/

        unsigned char  sym_mask;     /* puts bits to right point in integer */
        unsigned char  *symbols;     /* buffer of symbol bits */
        unsigned char *symbols_top;  /* top of symbol buffer */

        unsigned char ref_mask;    /* puts bits to right point in integer */
        unsigned char *refine;     /* buffer of refine bits */
        unsigned char *refine_top; /* top of refine buffer */

        int t_hold;                /* original threshold */

        SYMBOL *s;
} PACKED;

#endif
