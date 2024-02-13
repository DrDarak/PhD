#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "image_io.h"
#include "filters.h"
#include "mem_hand.h"
#include "UL-factor.h"

#define ZERO_PAD

typedef enum {LL,LH,HL,HH} SUBAND;
float * EstimateFilterCoeff (IMAGE *image,SUBAND suband, int scales, int n);
void clear_suband (IMAGE *image, SUBAND suband, int scales);
void apply_filter (IMAGE *image, SUBAND suband, int scales, float *a, int n);



void main(int argc, char *argv[])
{

	int columns,rows,colour;
	IMAGE *image,*image2, *orig;
	FILE *fp;
 	float *a_LH; 
 	float *a_HL; 
 	float *a_HH; 
	int n,mask,i,scales,shift;

	if (argc<3){
	   printf("%s: <image> <filter size> [blank]\n",argv[0]);
	   exit(-1);
	}

	n=atoi(argv[2]);
	/* load image */
	image=malloc_IMAGE(1,"");
	fp=read_raw_header(argv[1],&rows,&columns,&colour);
	image->columns=columns;
	image->rows=rows;
	get_pgm_file (fp,image);
	fclose(fp);

	image2=malloc_IMAGE(1,"");
        fp=read_raw_header("goldhill.y",&rows,&columns,&colour);
        image2->columns=columns;
        image2->rows=rows;  
        get_pgm_file (fp,image2);
        fclose(fp);


	/* make origal copy */
	orig=malloc_IMAGE(1,"");
	orig->columns=columns;
	orig->rows=rows;
	orig->pt=malloc_int(orig->rows*orig->columns,"");
	scales=2;
	shift=5;
	
	for (i=0;i<image->rows*image->columns;i++){
	    image->pt[i]<<=shift;
	    orig->pt[i]=image->pt[i];
 	}

	for (i=0;i<image2->rows*image2->columns;i++)
	    image2->pt[i]<<=shift;

	wavelet_filter_image(image,scales);
	wavelet_filter_image(image2,scales);

	if (argc==3){
	   a_LH=EstimateFilterCoeff(image,LH,scales,n);
   	   a_HL=EstimateFilterCoeff(image,HL,scales,n);
	   a_HH=EstimateFilterCoeff(image,HH,scales,n);

	   apply_filter (image2,LH,scales,a_LH,n);
	   apply_filter (image2,HL,scales,a_HL,n);
	   apply_filter (image2,HH,scales,a_HH,n);
	}
	else {
	     clear_suband (image,LH,scales);
	     clear_suband (image,HH,scales);
	     clear_suband (image,HL,scales);
	}

	wavelet_inv_filter_image(image2,scales);

	if (shift==0)
	   mask=0; 
	else
	    mask=1<<(shift-1);

	for (i=0;i<image2->rows*image2->columns;i++)
	    if (mask&image->pt[i]){
	       image2->pt[i]>>=shift;
	       image2->pt[i]++;
	    }
	    else 
	       image2->pt[i]>>=shift;

	write_pgm_file (image2,"new.pgm");        

	return;

}

void clear_suband
(IMAGE *image, /* the wavelet image */
 SUBAND suband,
 int scales)
{

	int i,j,x_limit,y_limit;
        int x_offset=0,y_offset=0;

      	y_limit=image->rows;
      	x_limit=image->columns;
      	for (i=0;i<scales;i++,x_limit/=2,y_limit/=2);

      	switch (suband) {
              case(LL): printf("can not work with LL\n");
                        return;
              case(LH): y_offset=y_limit;
                        break;
              case(HL): x_offset=x_limit;
                        break;
              case(HH): x_offset=x_limit;
                        y_offset=y_limit;
                        break;
        }

	for (j=0;j<y_limit*2;j++)
            for (i=0;i<x_limit*2;i++){
                image->pt[image->columns*(y_offset*2+j)+x_offset*2+i]=0;
           }

}

float * EstimateFilterCoeff
(IMAGE *image, /* the wavelet image */
 SUBAND suband,
 int scales,
 int n) /* size of filter */
{
   	int i,j,x_limit,y_limit;
	int ii,jj,k,l,ccolumns,crows;
	int x_offset=0,y_offset=0;
	int *indx,halfn,start,stop,cnt_kl,cnt_nm;
	float d;
	IMAGE *p,*s;
	float **sum_a,*pt_sum;
	float *sum_b;
	float sum,sum2;

	halfn=n/2;
      	y_limit=image->rows;
        x_limit=image->columns;
        for (i=0;i<scales;i++,x_limit/=2,y_limit/=2);

	switch (suband) {
	      case(LL): printf("can not work with LL\n"); 
			return NULL;
	      case(LH): y_offset=y_limit; 
			break;
	      case(HL): x_offset=x_limit; 
			break;
	      case(HH): x_offset=x_limit; 
	      	        y_offset=y_limit; 
			break;
	}

	p=malloc_IMAGE(1,"");
	p->columns=x_limit*2;
	p->rows=y_limit*2;
	p->pt=malloc_int(p->columns*p->rows,"");
	
	/* upsample image */
	for (i=0;i<y_limit*x_limit*4;i++)
	    p->pt[i]=0;
	        
	for (j=0;j<y_limit;j++)
	    for (i=0;i<x_limit;i++){
		p->pt[p->columns*j*2+i*2]=
			image->pt[image->columns*(y_offset+j)+x_offset+i];
#ifndef ZERO_PAD
		p->pt[p->columns*(j+1)*2+i*2+1]=
			image->pt[image->columns*(y_offset+j)+x_offset+i];
#endif
	    }

	/* make copy of original */
	s=malloc_IMAGE(1,"");
	s->columns=x_limit*2;
	s->rows=y_limit*2;
	s->pt=malloc_int(s->columns*s->rows,"");

	for (j=0;j<y_limit*2;j++)
            for (i=0;i<x_limit*2;i++)
                s->pt[s->columns*j+i]=
			image->pt[image->columns*(y_offset*2+j)+x_offset*2+i];

	/* malloc sums */
	sum_a=(float **)malloc_float(sizeof(float *)*n*n,"");
	for (i=0;i<n*n;i++)
	    sum_a[i]=(float *)malloc_float(sizeof(float)*n*n,"");
	sum_b=(float *)malloc_float(sizeof(float)*n*n,"");

	/* crop rows/columns */
	crows=s->rows-halfn;
	ccolumns=s->columns-halfn;

	stop=halfn;
	start=-halfn;
	
	/* do sumations sum_a's*/
	for (jj=start,cnt_nm=0;jj<=stop;jj++)
	    for (ii=start;ii<=stop;ii++,cnt_nm++) /* m,n loop on pauls sheet */
	        for (l=start,cnt_kl=0;l<=stop;l++)
	            for (k=start;k<=stop;k++,cnt_kl++){ /* k,l loop on pauls sheet */
	                /* sum_a  have to be in reverse order for UL */
	                pt_sum=sum_a[cnt_kl]+cnt_nm;
		        *pt_sum=0;
			/* PP sums */
	                for (j=-start;j<crows;j++)
	                    for (i=-start;i<ccolumns;i++){
			        *pt_sum+=(float)(p->pt[i+k+(j+l)*p->columns]
						*p->pt[i+ii+(j+jj)*p->columns]);
			    }
	            }

	/* SP sums */
	for (l=start,pt_sum=sum_b;l<=stop;l++)   
            for (k=start;k<=stop;k++,pt_sum++){ /* k,l loop on pauls sheet */  
		*pt_sum=0;
                for (j=-start;j<crows;j++)
                    for (i=-start;i<ccolumns;i++)
                        *pt_sum+=(float)(s->pt[i+j*s->columns]
                                       *p->pt[i+k+(j+l)*p->columns]);
	    }

	/* LU- factoriation */
        indx=(int *)malloc(sizeof(int)*(n*n));
	LU_decomp (sum_a,n*n,indx,&d);
	LU_back_substition(sum_a,n*n,indx,sum_b);

	sum=0;
	sum2=0;
	for (i=0;i<n*n;i++){
	    sum+=sum_b[i];
	    sum2+=sum_b[i]*sum_b[i];
	    printf("%f\n",sum_b[i]);

	}	
	printf("sum=%f\tsum2=%f\n",sum,sum2);

	/* blank sub band */
	for (j=0;j<p->rows;j++)
            for (i=0;i<p->columns;i++)
          	image->pt[image->columns*(y_offset*2+j)+x_offset*2+i]=0;

	/* predict suband */
	for (j=-start;j<crows;j++)
            for (i=-start;i<ccolumns;i++){
		pt_sum=sum_b;
		sum=0;
	        for (l=start;l<=stop;l++)
	            for (k=start;k<=stop;k++,pt_sum++)
	                sum+=(float)(p->pt[i+k+(j+l)*p->columns])*(*pt_sum);
          	image->pt[image->columns*(y_offset*2+j)+x_offset*2+i]=(int)rint(sum);
	   }

	for (i=0;i<n*n;i++)
	    free(sum_a[i]);
	free(sum_a);
	free_IMAGE(p);
	free_IMAGE(s);
	
	return sum_b;

}


void apply_filter
(IMAGE *image, /* the wavelet image */
 SUBAND suband,
 int scales,
 float *a,
 int n) /* size of filter */
{

        int i,j,x_limit,y_limit;
        int k,l,ccolumns,crows;
        int x_offset=0,y_offset=0;
        int halfn,start,stop;
        IMAGE *p;
        float *pt_sum;
        float sum;

        halfn=n/2;
        y_limit=image->rows;
        x_limit=image->columns;
        for (i=0;i<scales;i++,x_limit/=2,y_limit/=2);

        switch (suband) {
              case(LL): printf("can not work with LL\n");
                        return;
              case(LH): y_offset=y_limit;
                        break;
              case(HL): x_offset=x_limit;
                        break;
              case(HH): x_offset=x_limit;
                        y_offset=y_limit;
                        break;
        }

        p=malloc_IMAGE(1,"");
        p->columns=x_limit*2;
        p->rows=y_limit*2;
        p->pt=malloc_int(p->columns*p->rows,"");

        /* upsample image */
        for (i=0;i<y_limit*x_limit*4;i++)
            p->pt[i]=0;
 
        for (j=0;j<y_limit;j++)
            for (i=0;i<x_limit;i++){
                p->pt[p->columns*j*2+i*2]=
                        image->pt[image->columns*(y_offset+j)+x_offset+i];
#ifndef ZERO_PAD
                p->pt[p->columns*(j+1)*2+i*2+1]=
                        image->pt[image->columns*(y_offset+j)+x_offset+i];
#endif
	     }

       /* crop rows/columns */
        crows=p->rows-halfn;
        ccolumns=p->columns-halfn;

        stop=halfn;
        start=-halfn;

        /* blank sub band */
        for (j=0;j<p->rows;j++)
            for (i=0;i<p->columns;i++)
                image->pt[image->columns*(y_offset*2+j)+x_offset*2+i]=0;
 
        /* predict suband */
        for (j=-start;j<crows;j++)
            for (i=-start;i<ccolumns;i++){
                pt_sum=a;
                sum=0;
                for (l=start;l<=stop;l++)
                    for (k=start;k<=stop;k++,pt_sum++)
                        sum+=(float)(p->pt[i+k+(j+l)*p->columns])*(*pt_sum);
                image->pt[image->columns*(y_offset*2+j)+x_offset*2+i]=(int)rint(sum);
            }

	free_IMAGE(p);
}
