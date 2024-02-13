#ifndef STRUCT_H
#define STRUCT_H

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

#endif
