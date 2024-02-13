/* Image input/ouput functions */
#include "struct.h"

/* Functions used externally */
void wavelet_inv_filter_image (IMAGE *,int);
void wavelet_filter_image (IMAGE *,int);

/* Internal functions */
int mod_symm_even (int , int , int *);
void filter_x(IMAGE *, int , int );
void filter_y(IMAGE *, int , int );
void inv_filter_x(IMAGE *,int ,int );
void inv_filter_y(IMAGE *,int ,int );
void make_image_writeable(IMAGE *,int );

/* External functions */
extern int *malloc_int(int ,char *);
extern float *malloc_float(int ,char *);
extern void free_int (int *pt);
extern void NULL_FILE_error(FILE *,char *);

/* defines */

/* macros */
