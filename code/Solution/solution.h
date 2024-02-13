/* solution header */
#include "struct.h"

/*
  Routines defined within solution.c:
*/
void allocate_mem_coeff_planes(COEFF_PLANES *pt, int nitems);
void solve(COEFF_PLANES *data,
           float *x_out,	/* optimum induvidual compressions */
           float xt,	/* total compression availible */
           float accuracy, 	/* how close approximation needs to be to xt */
           int n);		/* number of coeffcient planes */

