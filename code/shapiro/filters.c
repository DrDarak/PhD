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
void filter_x(IMAGE *, WAVELET *, int , int );
void filter_y(IMAGE *, WAVELET *, int , int );
void inv_filter_x(IMAGE *,WAVELET *,int ,int );
void inv_filter_y(IMAGE *,WAVELET *,int ,int );
void make_image_writeable(IMAGE *,int );

/* macros */
#define sym_mod(x,y) ((x) < 0 ? -(x)-1 : ((x)>=y ? (y+y-(x)-1) : (x)))
#define sym_mod_sign(x,y) ((x) < 0 ? -1 : ((x)>=y ? -1 : 1))


/* this function reads the wavelet in form the disk */
/* and stores its values */

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
	
	/* check the the correct number of ranges are present */
        nitems=fscanf(fp,"%d\t%d\n",&(c->cmin),&(c->cmax));
        if (nitems !=2){
           printf("fscanf reads %d items instead of 2\n",nitems);
           exit(1);
        }
	
	/* malloc space for wavelet co-efficients */
        ccmem=malloc_float(c->cmax-c->cmin+1,"setup wavelet c");
        c->c=ccmem-c->cmin;

	/* read in wavelet co-efficients */
        for (i=c->cmin;i<=c->cmax;i++)
            fscanf(fp,"%f\n",&(c->c[i]));
	
	/* check the the correct number of ranges are present */
        nitems=fscanf(fp,"%d\t%d\n",&c->umin,&c->umax);
        if (nitems !=2){
           printf("fscanf reads %d items instead of 2\n",nitems);
           exit(1);
        }

	/* malloc space for wavelet co-efficients */
        cumem=malloc_float(c->umax-c->umin+1,"setup wavelet u");
        c->u = cumem-c->umin;

	/* read in wavelet co-efficients */
        for (i=c->umin;i<=c->umax;i++)
            fscanf(fp,"%f\n",&(c->u[i]));

        fclose(fp);

}


/* this function performs a wavlet filter in the x direction */

void filter_x
(IMAGE *image,
 WAVELET *c,
 int x_lim,
 int y_lim)
{
        int *sig;
        int shift,x,y,i;
        float lotemp,hitemp;
 
        shift=x_lim>>1;
 
        sig=malloc_int(x_lim,"filter x sig");
 
        for (y=0;y<y_lim;y++){
            /* load up signal */
            for (x=0;x<x_lim;x++)
                sig[x]=image->pt[image->columns*y+x];
            /* process signal */
            for (x=0;x<(x_lim>>1);x++){
                lotemp = 0;
                hitemp = 0;
 
                for (i=c->cmin; i<=c->cmax; i++)
                    lotemp+=c->c[i]*sig[sym_mod(i+2*x,x_lim)];

                for (i=1-c->umax; i<=1-c->umin; i++)
                    hitemp+=(i&1 ? (1):(-1)) * c->u[-i+1]*sig[sym_mod(i+2*x,x_lim)];

                image->pt[image->columns*y+x] = (int)lotemp;                  
                image->pt[image->columns*y+x+shift] = (int)hitemp;
            }
        }
 
        free_int(sig);
 
}


/* this function performs a wavel;et filter in the y direction */

void filter_y
(IMAGE *image,
 WAVELET *c,
 int x_lim,
 int y_lim)
{
        int *sig;
        int    shift,x,y,i;
        float lotemp,hitemp;
 
        shift=y_lim>>1;

        sig=malloc_int(y_lim,"filter y sig");
 
        for (x=0;x<x_lim;x++){
            /* load up signal */
            for (y=0;y<y_lim;y++)
                sig[y]=image->pt[image->columns*y+x];
            /* process signal */
            for (y=0;y<(y_lim>>1);y++){
                lotemp = 0;
                hitemp = 0;
 
                for (i=c->cmin; i<=c->cmax; i++)
                    lotemp+=c->c[i]*sig[sym_mod(i+2*y,y_lim)];
		
                for (i=1-c->umax; i<=1-c->umin; i++)
                    hitemp+=(i&1 ? (1):(-1)) * c->u[-i+1]*sig[sym_mod(i+2*y,y_lim)];
		
                image->pt[image->columns*y+x] = (int)lotemp;                      
                image->pt[image->columns*(y+shift)+x] = (int)hitemp;
            }
        }
 
        free_int(sig);
 
}


/* this function performs an inverse wavelet filter in the x direction */

void inv_filter_x
(IMAGE *image,
 WAVELET *c,
 int x_lim,
 int y_lim)
{
        int x,y,k,shift;
        float tmp;
        int *sig;
        int sign,s2;
        int kumin,kumax,kcmin,kcmax;
 
        shift=x_lim/2;
 
        sig=malloc_int(x_lim,"inv filter x sig");
 
        for (y=0;y<y_lim;y++){
            sign = +1;
            /* load up signal */
            for (x=0;x<x_lim;x++)
                sig[x]=image->pt[image->columns*y+x];
            for (x=0; x<x_lim; x++){
                sign = -sign;
 
                kumin=(x-c->umax+1)>>1;
                kumax=(x-c->umin)>>1;
                tmp=0;
                for (k=kumin; k<=kumax; k++)
                    tmp+=c->u[x-2*k]*sig[sym_mod(k,shift)];
 
                kcmin=(c->cmin+x)>>1;
                kcmax=(c->cmax+x-1)>>1;
                for (k=kcmin;k<=kcmax;k++){
                    s2=sym_mod_sign(k,shift)*sign;
                    tmp+=s2* c->c[2*k-x+1] * sig[shift+sym_mod(k,shift)];
                }
                /* load tmp varible back into image */
                image->pt[image->columns*y+x]=(int)tmp;
            }
        }
        free_int(sig);
}



/* this function performs an inverse wavelet filter in the y direction */

void inv_filter_y
(IMAGE *image,
 WAVELET *c,
 int x_lim, 
 int y_lim)
{
        int x,y,k,shift;
        float tmp;
        int *sig;
        int sign,s2;
        int kumin,kumax,kcmin,kcmax;
 
        shift=y_lim/2;
 
        sig=malloc_int(y_lim,"inv filter y sig");
 
        for (x=0;x<x_lim;x++){
            sign = +1;
            /* load up signal */
            for (y=0;y<y_lim;y++)
                sig[y]=image->pt[image->columns*y+x];
            for (y=0; y<y_lim; y++){
                sign = -sign;
 
                kumin=(y-c->umax+1)>>1;
                kumax=(y-c->umin)>>1;
                tmp=0;
                for (k=kumin; k<=kumax; k++)
                    tmp+=c->u[y-2*k]*sig[sym_mod(k,shift)];
 
                kcmin=(c->cmin+y)>>1;
                kcmax=(c->cmax+y-1)>>1;
                for (k=kcmin;k<=kcmax;k++){
                    s2=sym_mod_sign(k,shift)*sign;
                    tmp+=s2* c->c[2*k-y+1] * sig[shift+sym_mod(k,shift)];
                }
                /* load tmp varible back into image */
                image->pt[image->columns*y+x]=(int)tmp;
            }
        }
        free_int(sig);
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
 WAVELET *c,
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
            inv_filter_x(image,c,x_limit,y_limit);
            inv_filter_y(image,c,x_limit,y_limit);
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
 WAVELET *c,
 int scales)
{
 
        int i,x_limit,y_limit;
 
        /* Perform 2D wavelet transform for N scales */
        x_limit=image->columns;
        y_limit=image->rows;
        for (i=0;i<scales;i++){
            /*printf ("Wavelet transform: x limit - %3d, y limit - %3d \n",
                     x_limit, y_limit); */
            filter_x(image,c,x_limit,y_limit);
            filter_y(image,c,x_limit,y_limit);
            x_limit/=2;
            y_limit/=2;
        }
}




