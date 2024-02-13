/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                     wavelet filters                          */
/*                      reworked 3/7/96                         */
/*##############################################################*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "filters.h"
#include "mem_hand.h"

/* Internal functions */
int mod_symm_even (int , int , int *);
void filter_x(IMAGE *, int , int );
void filter_y(IMAGE *, int , int );
void inv_filter_x(IMAGE *,int ,int );
void inv_filter_y(IMAGE *,int ,int );
void make_image_writeable(IMAGE *,int );



#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

/* modulo -- m%n not defined by ANSI C for negative m */
#define mymod(M,N) ( ((M)<0) ? (N)-1 - (-(M)-1)%(N) : (M)%(N) ) 

#define MOD_REFLECT_11(M,N,ans) ans=mymod((M),(N)+(N)-2); if(ans>=(N))ans=(N)+(N)-ans-2;
#define MOD_REFLECT_21(M,N,ans) ans=mymod((M),(N)+(N)-1); if(ans>=(N))ans=(N)+(N)-ans-2;
#define MOD_REFLECT_12(M,N,ans) ans=mymod((M),(N)+(N)-1); if(ans>=(N))ans=(N)+(N)-ans-1;
#define MOD_REFLECT_22(M,N,S,ans) ans=mymod((M),(N)+(N));S=1; if(ans>=(N)){ans=(N)+(N)-ans-1;S= -1;}

/* Arithmetic shift right -- here because x>>y not ANSI for negative x */
#define ASR1(x) ((x)>>1)

/* wavelet ranges */
#define CMAX  4
#define CMIN -4
#define UMAX  3
#define UMIN -3

/* wavelet co-efficients */
#define C0  0.85269867900889
#define C1  0.37740285561283
#define C2 -0.11062440441844
#define C3 -0.02384946501956
#define C4  0.03782845550726
#define U0  0.78848561640637
#define U1  0.41809227322204
#define U2 -0.04068941760920
#define U3 -0.06453888262876


/* this function performs a wavlet filter in the x direction */


void filter_x
(IMAGE *image,
 int x_lim,
 int y_lim)
{
        int *sig, *mem;
        int shift,x,y;
        float lotemp,hitemp;
	int temp1, index, twox;
	int minc, maxc, minx, maxx, minOK, maxOK;

        minc = MIN(CMIN, 1-UMAX);
	maxc = MAX(CMAX, 1-(UMIN));
	minx = minc;
	maxx = maxc+x_lim-2;
	minOK = MIN(maxx+1,MAX(minx,0));
	maxOK = MAX(minx-1,MIN(maxx,x_lim-1));

	shift = x_lim>>1;
		
        mem = malloc_int(maxx-minx+1,"filter x sig");
	sig = mem-minx;

        for (y=0; y < y_lim; y++){
	  temp1 = image->columns*y;

	  /*load in signal in 3 pieces */
	  for (x=minx; x < minOK; x++) {
	    MOD_REFLECT_11(x, x_lim, index);
	    sig[x]=image->pt[temp1+index];
	  }
	  for (x=minOK; x <= maxOK; x++)
	    sig[x]=image->pt[temp1+x];
	  
	  for (x=maxOK+1; x <= maxx; x++) {
	    MOD_REFLECT_11(x, x_lim, index);
	    sig[x]=image->pt[temp1+index];
	  }


	  /* process signal */
	  for (x=0; x < shift; x++){
	    int *ptr1,*ptr2;
	    twox=x+x;

	    ptr1 = sig+twox;
	    ptr2 = ptr1-1;
	    lotemp  = C0*(*ptr1++);
	    lotemp += C1*(*ptr1++ + *ptr2--);
	    lotemp += C2*(*ptr1++ + *ptr2--);
	    lotemp += C3*(*ptr1++ + *ptr2--);
	    lotemp += C4*(*ptr1 + *ptr2);

	    ptr2=sig+twox;
	    ptr1=ptr2+1;  /* Points at middle */
	    hitemp = U0*(*ptr1++);
	    hitemp -= U1*(*ptr1++ + *ptr2--);
	    hitemp += U2*(*ptr1++ + *ptr2--);
	    hitemp -= U3*(*ptr1 + *ptr2);
 
	    image->pt[temp1+x] = (int)lotemp;                  
	    image->pt[temp1+x+shift] = (int)hitemp;
	  }
        }
	
        free_int(mem);
}


/* this function performs a wavelet filter in the y direction */


void filter_y
(IMAGE *image,
 int x_lim,
 int y_lim)
{
        int *sig, *mem;
        int shift,x,y;
        float lotemp,hitemp;
	int index, twoy;
	int minc, maxc, miny, maxy, minOK, maxOK;

        minc = MIN(CMIN, 1-UMAX);
	maxc = MAX(CMAX, 1-(UMIN));
	miny = minc;
	maxy = maxc+y_lim-2;
	minOK = MIN(maxy+1,MAX(miny,0));
	maxOK = MAX(miny-1,MIN(maxy,y_lim-1));
		
	shift = y_lim>>1;
 
        mem = malloc_int(maxy-miny+1,"filter y sig");
	sig = mem-miny;

        for (x=0; x < x_lim; x++){

	  /*load in signal in 3 pieces */
	  for (y=miny; y < minOK; y++) {
	    MOD_REFLECT_11(y, y_lim, index);
	    sig[y]=image->pt[image->columns*index+x];
	  }
	  for (y=minOK; y <= maxOK; y++)
	    sig[y]=image->pt[image->columns*y+x];

	  for (y=maxOK+1; y <= maxy; y++) {
	    MOD_REFLECT_11(y, y_lim, index);
	    sig[y]=image->pt[image->columns*index+x];
	  }


	  /* process signal */
	  for (y=0; y < shift; y++){


	    int *ptr1,*ptr2;
	    twoy=y+y;
	    
	    ptr1 = sig+twoy;
	    ptr2 = ptr1-1;
	    lotemp  = C0*(*ptr1++);
	    lotemp += C1*(*ptr1++ + *ptr2--);
	    lotemp += C2*(*ptr1++ + *ptr2--);
	    lotemp += C3*(*ptr1++ + *ptr2--);
	    lotemp += C4*(*ptr1 + *ptr2);

	    ptr2=sig+twoy;
	    ptr1=ptr2+1;  /* Points at middle */
	    hitemp = U0*(*ptr1++);
	    hitemp -= U1*(*ptr1++ + *ptr2--);
	    hitemp += U2*(*ptr1++ + *ptr2--);
	    hitemp -= U3*(*ptr1 + *ptr2);
 
	    image->pt[image->columns*y+x] = (int)lotemp;                  
	    image->pt[image->columns*(y+shift)+x] = (int)hitemp;
	  }
        }
	
        free_int(mem);
}



/* this function performs an inverse wavelet filter in the x direction */


void inv_filter_x
(IMAGE *image,
 int x_lim,
 int y_lim)
{
        int x,y,shift;
        int *siglo, *sighi, *memlo, *memhi;
        int sign, xlo, xhi;
	int maxx, minx, minOK, maxOK;

        shift=x_lim/2;

	maxx = MAX(ASR1(x_lim-1-(UMIN)),ASR1(CMAX+x_lim-2));
	minx = MIN(ASR1(1-UMAX),ASR1(CMIN));
	minOK = MIN(maxx+1,MAX(minx,0));
	maxOK = MAX(minx-1,MIN(maxx,shift-1));
 
        memlo=malloc_int(maxx-minx+1,"inv filter x sig (memlo)");
	memhi=malloc_int(maxx-minx+1,"inv filter x sig (memhi)");
	siglo=memlo-minx;
	sighi=memhi-minx;

        for (y=0;y<y_lim;y++){
	  sign = +1;
	  
	  /* load up signal */ 
	  for (x=minx; x<minOK; x++) {
	    MOD_REFLECT_12(x, shift, xlo);
	    MOD_REFLECT_21(x, shift, xhi);
	    siglo[x]=image->pt[image->columns*y+xlo];
	    sighi[x]=image->pt[image->columns*y+xhi+shift];
	  }
	  for (x=minOK; x<=maxOK; x++){
	    siglo[x]=image->pt[image->columns*y+x];
	    sighi[x]=image->pt[image->columns*y+x+shift];
	  }
	  for (x=maxOK+1; x<=maxx; x++) {
	    MOD_REFLECT_12(x, shift, xlo);
	    MOD_REFLECT_21(x, shift, xhi);
	    siglo[x]=image->pt[image->columns*y+xlo];
	    sighi[x]=image->pt[image->columns*y+xhi+shift];
	  }
			

	  for (x=0; x<shift; x++){

	    int *ptr1, *ptr2;
	    double temp1,temp2;

	    ptr1 = siglo+x;
	    ptr2 = ptr1-1;
	    temp1= U0*(*ptr1++);
	    temp1 += U2*(*ptr1 + *ptr2);
	    
	    ptr1 = sighi+x;
	    ptr2 = ptr1-1;
	    temp1 -= C1*(*ptr1++ + *ptr2--);
	    temp1 -= C3*(*ptr1 + *ptr2);

	    image->pt[image->columns*y + x+x] = (int) temp1;
	    
	    ptr1 = siglo+x+1;
	    ptr2 = ptr1-1;
	    temp2 = U1*(*ptr1++ + *ptr2--);
	    temp2 += U3*(*ptr1 + *ptr2);
	    
	    ptr1 = sighi+x;
	    ptr2 = ptr1-1;
	    temp2 += C0*(*ptr1++);
	    temp2 += C2*(*ptr1++ + *ptr2--);
	    temp2 += C4*(*ptr1 + *ptr2);
	    
	    image->pt[image->columns*y + x+x+1] = (int) temp2;				
	  }
        }
        free_int(memlo);
	free_int(memhi);
}


/* this function performs an inverse wavelet filter in the y direction */


void inv_filter_y
(IMAGE *image,
 int x_lim,
 int y_lim)
{
        int x,y,shift;
        int *siglo, *sighi, *memlo, *memhi;
        int sign, ylo, yhi;
	int maxy, miny, minOK, maxOK;

        shift=y_lim/2;

	maxy = MAX(ASR1(y_lim-1-(UMIN)),ASR1(CMAX+y_lim-2));
	miny = MIN(ASR1(1-UMAX),ASR1(CMIN));
	minOK = MIN(maxy+1,MAX(miny,0));
	maxOK = MAX(miny-1,MIN(maxy,shift-1));
	
        memlo=malloc_int(maxy-miny+1,"inv filter y sig (memlo)");
	memhi=malloc_int(maxy-miny+1,"inv filter y sig (memhi)");
	siglo=memlo-miny;
	sighi=memhi-miny;
	
        for (x=0;x<x_lim;x++){
	  sign = +1;

	  /* load up signal */
          for (y=miny; y<minOK; y++) {
	    MOD_REFLECT_12(y, shift, ylo);
	    MOD_REFLECT_21(y, shift, yhi);
	    siglo[y]=image->pt[image->columns*ylo+x];
	    sighi[y]=image->pt[image->columns*(yhi+shift)+x];
	  }
	  for (y=minOK; y<=maxOK; y++){
	    siglo[y]=image->pt[image->columns*y+x];
	    sighi[y]=image->pt[image->columns*(y+shift)+x];
	  }
	  for (y=maxOK+1; y<=maxy; y++) {
	    MOD_REFLECT_12(y, shift, ylo);
	    MOD_REFLECT_21(y, shift, yhi);
	    siglo[y]=image->pt[image->columns*ylo+x];
	    sighi[y]=image->pt[image->columns*(yhi+shift)+x];
	  }
			

	  for (y=0; y<shift; y++){
	    int *ptr1, *ptr2;
	    double temp1,temp2;

	    ptr1 = siglo+y;
	    ptr2 = ptr1-1;
	    temp1= U0*(*ptr1++);
	    temp1 += U2*(*ptr1 + *ptr2);
 
	    ptr1 = sighi+y;
	    ptr2 = ptr1-1;
	    temp1 -= C1*(*ptr1++ + *ptr2--);
	    temp1 -= C3*(*ptr1 + *ptr2);

	    image->pt[image->columns*(y+y) + x] = (int) temp1;

	    ptr1 = siglo+y+1;
	    ptr2 = ptr1-1;
	    temp2 = U1*(*ptr1++ + *ptr2--);
	    temp2 += U3*(*ptr1 + *ptr2);

	    ptr1 = sighi+y;
	    ptr2 = ptr1-1;
	    temp2 += C0*(*ptr1++);
	    temp2 += C2*(*ptr1++ + *ptr2--);
	    temp2 += C4*(*ptr1 + *ptr2);
	    
	    image->pt[image->columns*(y+y+1) + x] = (int) temp2;				
	  }
        }
        free_int(memlo);
	free_int(memhi);
}


void make_image_writeable
(IMAGE *image,
 int scales)
{
 
        int i,j,k,ymax,xmax;
 
        xmax=image->columns;
        ymax=image->rows;
 
             
        for (k=1;k<=scales;k++){
            for (j=0;j<ymax;j++)
                for (i=xmax/2;i<xmax;i++){
                    (image->pt[image->columns*j+i])>>=k;
                    (image->pt[image->columns*j+i])+=128;
                }
            for (j=ymax/2;j<ymax;j++)
                for (i=0;i<xmax/2;i++){
                    (image->pt[image->columns*j+i])>>=k;
                    (image->pt[image->columns*j+i])+=128;
                }
            ymax/=2;
            xmax/=2;
        }
        for (j=0;j<ymax;j++)
            for (i=0;i<xmax;i++){
                (image->pt[image->columns*j+i])>>=scales;
            }
 
             
}



/* this function controls the inverse filtering of an image. */
/* it alternatley calls inverse filter x and inverse filter y */
/* until the appropriate number of transforms have been performed */
/* this number is stored in scales */

void wavelet_inv_filter_image
(IMAGE *image,
 int scales)
{
 
        int i,x_limit,y_limit;
 
        /* set up top scale limites */
        y_limit=image->rows;
        x_limit=image->columns;
        for (i=1;i<scales;i++,x_limit/=2,y_limit/=2);
 
        /* Perform 2D inverse wavelet transform for N scales */
        for (i=0;i<scales;i++){
            /*printf ("Wavelet transform: x limit - %3d, y limit - %3d \n",
                     x_limit, y_limit); */
            inv_filter_x(image,x_limit,y_limit);
            inv_filter_y(image,x_limit,y_limit);
            x_limit*=2;
            y_limit*=2;
        }
 
}



/* this function controls the forward filtering of an image. */
/* it alternatley calls filter x and filter y */
/* until the appropriate number of transforms have been performed */
/* this number is stored in scales */
 
void wavelet_filter_image
(IMAGE *image,
 int scales)
{
 
        int i,x_limit,y_limit;
 
        /* Perform 2D wavelet transform for N scales */
        x_limit=image->columns;
        y_limit=image->rows;
        for (i=0;i<scales;i++){
            /*printf ("Wavelet transform: x limit - %3d, y limit - %3d \n",
                     x_limit, y_limit); */
            filter_x(image,x_limit,y_limit);
            filter_y(image,x_limit,y_limit);
            x_limit/=2;
            y_limit/=2;
        }
}




