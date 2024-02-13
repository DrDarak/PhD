/* solution header */
#include "struct.h"

/* Functions used externally */
void allocate_mem_coeff_planes (COEFF_PLANES *, int );
void solve (COEFF_PLANES *, float *,float ,float ,int);        
void calculate_gradient (COEFF_PLANES *,int );
void smooth_gradient (COEFF_PLANES *);
float find_compression (COEFF_PLANES *,float );
float find_error (COEFF_PLANES *,float );
float find_gradient (COEFF_PLANES *,float );
/* Internal functions */


/* External functions */
extern int *malloc_int(int ,char *);
extern void free_int (int *pt);
extern float *malloc_float (int , char *);
extern void free_float (float *);

/* defines */
#define COMP_STEP 0.5

/* macros */
#define myabs(x)    ( x < 0 ? -x : x)

