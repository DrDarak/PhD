/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                     wavelet filters                          */
/*                      reworked 3/7/96                         */
/*                     re-reworked Barry Sherlock 28 Nov 96     */
/*##############################################################*/

/* Present program works with edge reflection for odd symmetrical*/
/* wavelets.  Different reflection is required for even lengths.*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "filters.h"

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))
/* modulo -- m%n not defined by ANSI C for negative m*/
#define mymod(M,N) ( ((M)<0) ? (N)-1 - (-(M)-1)%(N) : (M)%(N) ) 
#define MOD_REFLECT_11(M,N,ans) ans=mymod((M),(N)+(N)-2); if(ans>=(N))ans=(N)+(N)-ans-2;
#define MOD_REFLECT_21(M,N,ans) ans=mymod((M),(N)+(N)-1); if(ans>=(N))ans=(N)+(N)-ans-2;
#define MOD_REFLECT_12(M,N,ans) ans=mymod((M),(N)+(N)-1); if(ans>=(N))ans=(N)+(N)-ans-1;
#define MOD_REFLECT_22(M,N,S,ans) ans=mymod((M),(N)+(N));S=1; if(ans>=(N)){ans=(N)+(N)-ans-1;S= -1;}
/* Arithmetic shift right -- here because x>>y not ANSI for negative x*/
#define ASR1(x) ((x)>>1)

void setup_wavelet
(char *fname_wave,
 WAVELET *c)
{
    int i;
    FILE *fp;
    int nitems,shift;
    float *ccmem,*cumem;

	/* read interger wavelet from disk */
        fp = fopen(fname_wave,"r");
	NULL_FILE_error(fp,"setup wavelet");

	/* scan in scaling */
	fscanf(fp,"%d\n",&shift);
	c->shift=shift;

        nitems=fscanf(fp,"%d\t%d\n",&(c->cmin),&(c->cmax));
        if (nitems !=2){
           printf("fscanf reads %d items instead of 2\n",nitems);
           exit(1);
        }

        ccmem=malloc_float(c->cmax-c->cmin+1,"setup wavelet c");
        c->c=ccmem-c->cmin;

        for (i=c->cmin;i<=c->cmax;i++)
            fscanf(fp,"%f\n",&(c->c[i]));
	
        nitems=fscanf(fp,"%d\t%d\n",&c->umin,&c->umax);
        if (nitems !=2){
           printf("fscanf reads %d items instead of 2\n",nitems);
           exit(1);
        }


        cumem=malloc_float(c->umax-c->umin+1,"setup wavelet u");
        c->u = cumem-c->umin;

        for (i=c->umin;i<=c->umax;i++) 
            fscanf(fp,"%f\n",&(c->u[i]));

        fclose(fp);

}

void filter_x
(IMAGE *image,
 WAVELET *c,
 int x_lim,
 int y_lim)
{
        int *sig, *mem;
        int shift,x,y,i;
        float lotemp,hitemp;
	int temp1, index;
	int minc, maxc, minx, maxx, minOK, maxOK;

        minc = MIN(c->cmin,1 - c->umax);
	maxc = MAX(c->cmax,1 - c->umin);
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
                lotemp = 0;
                hitemp = 0;
 
                for (i=c->cmin; i<=c->cmax; i++) {
                    lotemp += c->c[i]*sig[i+x+x];
				}
                for (i=1-c->umax; i<=1-c->umin; i++) {
                    hitemp += (i&1 ? c->u[-i+1] : -c->u[-i+1])*sig[i+x+x];
				}
                image->pt[temp1+x] = (int)lotemp;                  
                image->pt[temp1+x+shift] = (int)hitemp;
            }
        }
 
        free_int(mem);
}


void filter_y
(IMAGE *image,
 WAVELET *c,
 int x_lim,
 int y_lim)
{
        int *sig, *mem;
        int shift,x,y,i;
        float lotemp,hitemp;
	int index;
	int minc, maxc, miny, maxy, minOK, maxOK;

        minc = MIN(c->cmin,1 - c->umax);
	maxc = MAX(c->cmax,1 - c->umin);
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
                lotemp = 0;
                hitemp = 0;
 
                for (i=c->cmin; i<=c->cmax; i++) {
                    lotemp += c->c[i]*sig[i+y+y];
				}
                for (i=1-c->umax; i<=1-c->umin; i++) {
                    hitemp += (i&1 ? c->u[-i+1] : -c->u[-i+1])*sig[i+y+y];
				}
                image->pt[image->columns*y+x] = (int)lotemp;                  
                image->pt[image->columns*(y+shift)+x] = (int)hitemp;
            }
        }
 
        free_int(mem);
}


void inv_filter_x
(IMAGE *image,
 WAVELET *c,
 int x_lim,
 int y_lim)
{
        int x,y,k,shift;
        double tmp1, tmp2;
        int *siglo, *sighi, *memlo, *memhi;
        int sign, xlo, xhi;
        int kumin,kumax,kcmin,kcmax;
		int maxx, minx, minOK, maxOK;

        shift=x_lim/2;

	    maxx = MAX(ASR1(x_lim-1-c->umin),ASR1(c->cmax+x_lim-2));
		minx = MIN(ASR1(1-c->umax),ASR1(c->cmin));
		minOK = MIN(maxx+1,MAX(minx,0));
		maxOK = MAX(minx-1,MIN(maxx,shift-1));
 
        memlo=malloc_int(maxx-minx+1,"inv filter x sig (memlo)");
		memhi=malloc_int(maxx-minx+1,"inv filter x sig (memhi)");
		siglo=memlo-minx;
		sighi=memhi-minx;

        for (y=0;y<y_lim;y++){
            sign = +1;

            // load up signal 
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
			

            for (x=0; x<x_lim; x++){
                sign = -sign;
 
                kumin=ASR1(1+x-c->umax);
                kumax=ASR1(x-c->umin);
                tmp1=0;
                for (k=kumin; k<=kumax; k++)
                    tmp1+=c->u[x-k-k]*siglo[k];
 
                kcmin=ASR1(c->cmin+x);
                kcmax=ASR1(c->cmax+x-1);
				tmp2=0;
                for (k=kcmin;k<=kcmax;k++)
                    tmp2+=c->c[(k+k-x)+1] * sighi[k];
     
                // load tmp varible back into image 
                image->pt[image->columns*y+x]=(int)(tmp1+sign*tmp2);
            }
        }
        free_int(memlo);
		free_int(memhi);
}

void inv_filter_y
(IMAGE *image,
 WAVELET *c,
 int x_lim,
 int y_lim)
{
        int x,y,k,shift;
        double tmp1, tmp2;
        int *siglo, *sighi, *memlo, *memhi;
        int sign, ylo, yhi;
        int kumin,kumax,kcmin,kcmax;
		int maxy, miny, minOK, maxOK;

        shift=y_lim/2;

	    maxy = MAX(ASR1(y_lim-1-c->umin),ASR1(c->cmax+y_lim-2));
		miny = MIN(ASR1(1-c->umax),ASR1(c->cmin));
		minOK = MIN(maxy+1,MAX(miny,0));
		maxOK = MAX(miny-1,MIN(maxy,shift-1));
 
        memlo=malloc_int(maxy-miny+1,"inv filter y sig (memlo)");
		memhi=malloc_int(maxy-miny+1,"inv filter y sig (memhi)");
		siglo=memlo-miny;
		sighi=memhi-miny;

        for (x=0;x<x_lim;x++){
            sign = +1;

            // load up signal 
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
			

            for (y=0; y<y_lim; y++){
                sign = -sign;
 
                kumin=ASR1(1+y- c->umax);
                kumax=ASR1(y-c->umin);
                tmp1=0;
                for (k=kumin; k<=kumax; k++)
                    tmp1+=c->u[y-k-k]*siglo[k];
 
                kcmin=ASR1(c->cmin+y);
                kcmax=ASR1(c->cmax+y-1);
				tmp2=0;
                for (k=kcmin;k<=kcmax;k++)
                    tmp2+=c->c[(k+k-y)+1] * sighi[k];
     
                // load tmp varible back into image 
                image->pt[image->columns*y+x]=(int)(tmp1+sign*tmp2);
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
	float temp;
	float *flimg;

    xmax=image->columns;
    ymax=image->rows;

	flimg = malloc_float ((xmax * ymax), "lkhgkj");
	
	for (i=0; i<(xmax*ymax); i++)
        flimg[i] = (float)image->pt[i];

        for (j=0;j<ymax;j++)
            for (i=0;i<xmax;i++){
                temp = (flimg[image->columns*j+i]) * 0.7071;
             }
	xmax/=2;

        for (k=1;k<=scales;k++){
            for (j=0;j<ymax;j++)
                for (i=0;i<xmax;i++)
                    (flimg[image->columns*j+i])/=2;
            ymax/=2;
            xmax/=2;
        }

	ymax*=2;
	xmax*=2;

        for (j=0;j<image->rows;j++)
            for (i=0;i<image->columns;i++){
	        if (!(j < ymax && i < xmax))
		    (flimg[image->columns*j+i])+=128.0;
            }
	for (i=0; i<(xmax * ymax); i++)
             image->pt[i] = (int)rint(flimg[i]);
}

void wavelet_inv_filter_image
(IMAGE *image,
 WAVELET *c,
 int scales)
{
 
        int i,x_limit,y_limit;
        /* set up top scale limits */
        y_limit=image->rows;
        x_limit=image->columns;

        for (i=1;i<scales;i++) {
			x_limit/=2;
			y_limit/=2;
			}

        /* Perform 2D inverse wavelet transform for N scales */
        for (i=0;i<scales;i++){
            inv_filter_x(image,c,x_limit,y_limit);
            inv_filter_y(image,c,x_limit,y_limit);
            x_limit*=2;
            y_limit*=2;
        }
		
 
}
 
void wavelet_filter_image
(IMAGE *image,
 WAVELET *c,
 int scales)
{
 
    int i,x_limit,y_limit;

    /* Perform 2D wavelet transform for N scales */   
    x_limit=image->columns;
    y_limit=image->rows;

        for (i=0;i<scales;i++){
            filter_x(image,c,x_limit,y_limit);
            filter_y(image,c,x_limit,y_limit);
            x_limit/=2;
            y_limit/=2;
        }
}





