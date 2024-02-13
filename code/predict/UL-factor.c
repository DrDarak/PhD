/*######################################################*/
/*		     UL-factorisation 			*/
/*		       David Bethel			*/
/*			11/7/95				*/
/*######################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "UL-factor.h"

void LU_decomp (float **a, int n, int *indx, float *d)
{

	/* ################## */
	/*  i=rows j=columns  */
	/* ################## */

	int i,imax=0,j,k;
	float big,dum,sum,temp;
	float *vv;

	/* define space for vv */
	vv=(float *)malloc(sizeof(float)*n);

	*d=1.0;
	for (i=0;i<n;i++){
	    big=0.0;
	    for (j=0;j<n;j++)
	        if ((temp=fabs(a[i][j]))>big)
		   big=temp;
	    if (big==0.0){
	       printf("ERROR-singular matrix\n");
	       exit(-1);
	    }
	    vv[i]=1.0/big;
	}

	for (j=0;j<n;j++){ 
            for (i=0;i<j;i++){ 
		sum=a[i][j];
	        for (k=0;k<i;k++)
		    sum-=a[i][k]*a[k][j];
		a[i][j]=sum;
	    }
	    big=0.0;
	 
	    for (i=j;i<n;i++){
	        sum=a[i][j];
		for (k=0;k<j;k++)
		    sum-=a[i][k]*a[k][j];
		a[i][j]=sum;
		if ((dum=vv[i]*fabs(sum))>=big){
		   big=dum;
		   imax=i;
		}
	    }

	    if (j!=imax){
	       for (k=0;k<n;k++){
		   dum=a[imax][k];
		   a[imax][k]=a[j][k];
	           a[j][k]=dum;
	       }
	       *d=-(*d);
	       vv[imax]=vv[j];
	    }

	    indx[j]=imax;

	    if (a[j][j]==0.0)
	       a[j][j]=TINY;

	    if (j!=(n-1)){
	       dum=1.0/(a[j][j]);
	       for (i=j+1;i<n;i++)
		   a[i][j]*=dum;

	    }
	}

	free(vv);


}

void LU_back_substition(float **a, int n, int *indx, float *b)
{

	int i,ii,ip,j;
	float sum;

	/* ################## */
	/*  i=rows j=columns  */
	/* ################## */

	ii=-1;   /* setup as value not used */
	for (i=0;i<n;i++){

	    ip=indx[i];
	    sum=b[ip];
	    b[ip]=b[i];
	    if (ii>=0)
	       for (j=ii;j<i;j++)
		   sum-=a[i][j]*b[j];
	    else 
		if (sum)
		   ii=i;
	    b[i]=sum;
	}

	for (i=n-1;i>=0;i--){
	    sum=b[i];
	    for (j=i+1;j<n;j++)
		sum-=a[i][j]*b[j];
	    b[i]=sum/a[i][i];
	}

}

/*
void main(argc,argv)
int argc;
char *argv[];
{
	int i,j,n=4;
	int *indx;
	float **a;
	float *b;
	float d;

	a=(float **)malloc(sizeof(float *)*(n));
	for (i=0;i<n+1;i++)
	    a[i]=(float *)malloc(sizeof(float)*(n));

	b=(float *)malloc(sizeof(float)*(n));

	indx=(int *)malloc(sizeof(int)*(n));

	a[0][0]=4; a[0][1]=2; a[0][2]=2; a[0][3]=1; b[0]=2;
	a[1][0]=2; a[1][1]=8; a[1][2]=1; a[1][3]=2; b[1]=1;
	a[2][0]=1; a[2][1]=1; a[2][2]=3; a[2][3]=0; b[2]=2;
	a[3][0]=1; a[3][1]=0; a[3][2]=0; a[3][3]=2; b[3]=3;

	for (i=0;i<n;i++){
	    for (j=0;j<n;j++)
		printf("%f\t",a[i][j]);
	    printf("\n");
	}

	LU_decomp(a,n,indx,&d);
	for (i=0;i<n;i++)
		printf("%d\t",indx[i]);

	    printf("\n");
	    printf("\n");

	for (i=0;i<n;i++){
	    for (j=0;j<n;j++)
		printf("%f\t",a[i][j]);
	    printf("\n");
	}

 	LU_back_substition(a,n,indx,b);
	printf("\n");
	for (i=0;i<4;i++)
	    printf("%f\n",b[i]);
	

}

*/



