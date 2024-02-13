/*######################################################*/
/*		     UL-factorisation 			*/
/*		       David Bethel			*/
/*			11/7/95				*/
/*######################################################*/
#define  TINY 1.0E-20
#define myabs(x)    ( x < 0 ? -x : x)


void LU_decomp (float **a, int n, int *indx, float *d);
void LU_back_substition(float **a, int n, int *indx, float *b);

