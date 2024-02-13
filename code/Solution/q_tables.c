/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                  optimal block quantiser                     */
/*                   spring 96 +6/12/96                         */
/*##############################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "miscw.h"
#include "struct.h"
#include "mem_hand.h"
#include "solution.h"
#include "huff.h"
#include "bitstrm.h"

#include "optim_q.h"
#include "q_tables.h"

#ifdef __cplusplus
#  include <memleak.h> /* must be the last .h file included */
#endif

#define QUANT_RANGE 10		 /* number of quant levels passed to solution */
#define RD_CONDENSE_SPEED 10 /* higher number .. slower condence min:2 */
#define Q_STEP 6			 /* quant values +- quant value * Q_STEP ... changes 
								condence speed min:2 */

#if defined(__cplusplus)
inline
#endif
int quantise(int c, int q)
{
 return(q > 0 ? c/q : 0);
}

/****************************************************************/
/*
  Routines defined & only used within this module
*/
void RD_curve_by_entropy(IMAGE *image, COEFF_PLANES *pt, int coeff);
int huffman_estimate_image(IMAGE *image,
                           int   *q_out,
                           HUFF  *dc,
                           HUFF  *ac,
                           float *bits,
                           float *error);

int max_coeff(BLOCK *block, int coeff);
int find_quantisation(float x, COEFF_PLANES *pt);
void unquantise_image(IMAGE *image, int *q_out, int size);
int quantise_image(IMAGE *image,
                   int *q_out,
                   double *entropy,
                   int size);
int find_no_ac_symbols(IMAGE *image,
                       int *q_out,
                       float *x_out,
                       int size,
                       int low_high); /* 0 - low    1 - high */
void max_min_quant_coeff(BLOCK *block,
                         int coeff,
                         int *min,
                         int *max);
/****************************************************************/

void write_quantisation
(char *fname,
 QUANT *quant)
{	

	int i;
	FILE *fp;

	/* open and write quant file */
	fp=fopen(fname,"wb");
	NULL_FILE_error(fp,"write quant");
	fprintf(fp,"%f\n",quant->bpp);
	for (i=0;i<quant->size;i++){
	    fprintf(fp,"%d\t%d\n",(int)rint(quant->q[i]),quant->nitems[i]);
	}
	fclose(fp);

}

void form_quantisation_tables
(IMAGE *image,
 int size)
{
	int i,j,max,nitems,last,k;
	float factor;
	int q[QUANT_RANGE];
	COEFF_PLANES *pt;
	
	/* allocate memoery for quantisation data */
        image->data=malloc_COEFF_PLANES(size,"form quant tables");	
	
	/* loop through all coefficent sets */ 
	for (i=0;i<size;i++){

	    /* find max coefficient in set */
 	    max=max_coeff(image->tree_top,i);
	    

	    /* calcualte quant scaling factor */
	    factor=(float)(log((double)max)/log(2));
	    factor/=(float)QUANT_RANGE;
	
	    /* if the coefficent > quantisation range the work out quant step */
	    if (max>QUANT_RANGE)
	       for (j=QUANT_RANGE;j>0;j--) 
	           q[QUANT_RANGE-j]=(int)ceil(pow(2,factor*(float)j));
	    else
	        for (j=QUANT_RANGE;j>0;j--) 
	            q[QUANT_RANGE-j]=(int)j;

	    /* search through items and check reportion etc*/
	    last=max;
	    nitems=1;
	    for (j=0;j<QUANT_RANGE;j++)
	        if (q[j]>=last || q[j]<1)
		   q[j]=-1;
		else{ 
		     nitems++;
		     last=q[j];
		}

	    /* check if stream is terminated with no quantisation */
	    if (last>1)
	       nitems++;

	    /* allocate memory for coeff plane */
	    pt=(image->data)+i;
            allocate_mem_coeff_planes(pt,nitems);

	    /* load approprate items into list */
	    pt->q[0]=max+1;  /* max quant is set to max level +1 to ensure 0 entropy */
	    k=1;
	    for (j=0;j<QUANT_RANGE;j++){
		if (q[j]!=-1)
		   pt->q[k++]=q[j];
	    }
	    if (last>1)
	       pt->q[k]=1;
		
	    /* load max in coeff struct */
            pt->max=max;

	}

}

void quantise_coefficients
(IMAGE *image,
 int size)
{

	int i;

	/* now apply these quantisation tables to the data */
        for (i=0;i<size;i++)
            RD_curve_by_entropy(image,image->data+i,i);

}

static void RD_curve_by_entropy
(IMAGE *image,
 COEFF_PLANES *pt,
 int coeff)
{
	int i,j,max;
	double entropy,last_entropy;
	int error,q,diff;
	int *bin_bottom,*bins,nitems;
	BLOCK *block;
	
	max=pt->max;

	/* since min quantisation is 1 => nitems is 2*max (accounting for -/+)*/
	bin_bottom=malloc_int(2*max+1,"quantise coeff");
	bins=bin_bottom+max;

	/* scan through quantisations */
	i=0;
	last_entropy=-1;
	while (i<pt->nitems){
	    /* define top of block list */
	    block=image->tree_top;

	    /* clear bins */
	    for (j=0;j<2*max+1;j++)
	        bin_bottom[j]=0;

	    /* clear error */
	    error=0;

	    /* load up quantiastion value */
	    q=pt->q[i];

	    /* scan through block list */
	    nitems=0;
	    while (block!=NULL){
	          diff=quantise(block->pt[coeff],q); 
	          bins[diff]++; 			
	          diff=(block->pt[coeff])-diff*q;
		      diff*=diff;
	          error+=diff;
		      nitems++;
	          block=block->next; 
	    }
	    
	    /* calculate bits / pixel */
	    entropy=0;
	    for (j=-max;j<=max;j++)
	        if (bins[j]!=0)
                   entropy-=(double)bins[j]*log10((double)bins[j]/(double)nitems);                 

	    /* ensure no repeated points in x */
	    if (last_entropy>=entropy){
	       /* shift quantisation values down */
	       for (j=i;j<pt->nitems-1;j++)
	     	   pt->q[j]=pt->q[j+1]; 	
	       /* decrease size of list and bring back list pt */
	       pt->nitems--;

	       /* do not write effects to RD graph */
	       if (i<pt->nitems)
		      continue;
	       else 
		       break;
	    }

	    pt->x[i]=(float)(entropy/(float)log10(2.0));
	    pt->e[i]=(float)error;

	    /* finish loop */
	    i++;
	    last_entropy=entropy;
	}
	free_int(bin_bottom);

}

static int huffman_estimate_image(IMAGE *image,
                                  int   *q_out,
                                  HUFF  *dc,
                                  HUFF  *ac,
                                  float *bits,
                                  float *error)
{
 BLOCK *block;
 int diff,cq,i,j,n,run,context,start,total;
 float bit;

 block=image->tree_top;

 for (i=0;i<block->size;i++)
     {
      bits[i]=(float)0.0;
      error[i]=(float)0.0;
     }	

 while (block!=NULL)
       {
        context=32768; 
        run=0;
        start=-1;
        cq=quantise(block->pt[0],q_out[0]);
        bits[0]+=(float)estimate_compress_symbol(dc,cq); 
        diff=block->pt[0]-cq*q_out[0];
        error[0]+=diff*diff; 
        i=1;

        while (i<block->size)
              {	
               /* quantise */
               cq=quantise(block->pt[i],q_out[i]);
               if (cq==context)
                  {	// another run
                   if (start==-1)
                       start=i;
                   run++; 
                  }
               else
                   if (run==0)	    // no runs therefore compress coeff
                       bits[i]+=(float)estimate_compress_symbol(ac,cq);
                   else
                      {	 // else a run length
                       bit=(float)estimate_compress_run(ac,run);
                       for (j=start;j<start+run;j++)
                            bits[j]+=bit/(float)run;
                       run=0;
                       start=-1;
                       bits[i]+=(float)estimate_compress_symbol(ac,cq); 
                      }
       

               /* find error in coeff */
               diff=block->pt[i]-cq*q_out[i];
               error[i]+=(float)(diff*diff);
       		   
			   i++; // increament to next position	in wavelet tree
               context=cq;	// change context
               
			   /* check for jumps(ac->run-1) and stops (ac->run) and max runs */
               if (run+1>=ac->run-1)
                  {
                   if (context==0)	   // only interested in zeros
                       while (n=check_for_jump(block,q_out,i-run))
                             {
                              if (n==block->size)
                                 {
                                  bit=(float)estimate_compress_run(ac,ac->run); //stop
                                  for (j=i-run;j<n;j++)
                                      bits[j]+=bit/(float)(n-i+run);
                                  // enter error for truncation of runs
                                  for (j=i;j<n;j++)
								      error[j]+=(float)(block->pt[j]*block->pt[j]); 
                                  run=0;
                                  start=-1;
                                  i=n;
                                  break;
                                 }
       
                              if (n<i)   // uses a max run size to skip subands if 
                                  break; // it is more effective than jump
       
                              // default case ... jump symbol
                              bit=(float)estimate_compress_run(ac,ac->run-1); //jump
                              for (j=i-run;j<n;j++)
                                   bits[j]+=bit/(float)(n-i+run);
                              // enter error for truncation of runs
							  for (j=i;j<n;j++)
                                   error[j]+=(float)(block->pt[j]*block->pt[j]); 
                              start=-1;
                              i=n;
                              run=0;
                             }
       
                   if (run!=0)
                      {
                       bit=(float)estimate_compress_run(ac,run); // repeated max run
                       for (j=start;j<start+run;j++)
                            bits[j]+=bit/(float)run;
                       run=0;
                       start=-1;
                      }
       
                   continue;  //  always return to loop if max run occurs 
                  }
              }

        if (run!=0)
           {
            bit=(float)estimate_compress_run(ac,ac->run);
            for (j=start;j<start+run;j++)
                 bits[j]+=bit/(float)run;
           }

        block=block->next;
       }

 total=0;
 for (i=0;i<image->tree_top->size;i++)
      total+=(int)bits[i];

 return total;
}

int ammend_quantisation
(IMAGE *image,
 int *q_out, 
 HUFF *dc, 
 HUFF *ac,
 int n_bits)
{
	int i,j,total_a,total_b,d,tmp;
	int *tmp_q;
	float *bits_a,*error_a;
	float *bits_b,*error_b;
	float *rd;
	float best_rd;
	int best_i;
	BLOCK *block;

	block=image->tree_top;

	tmp_q=malloc_int(block->size,"ammend q tmp_q");
	for (i=0;i<block->size;i++)
	    tmp_q[i]=q_out[i];

	bits_a=malloc_float(block->size,"ammend q bits_a");
	bits_b=malloc_float(block->size,"ammend q bits_b");
	error_a=malloc_float(block->size,"ammend q error_a");
	error_b=malloc_float(block->size,"ammend q error_b");
	rd=malloc_float(block->size,"ammend q rd");

	total_a=huffman_estimate_image(image,q_out,dc,ac,bits_a,error_a);
	
	d=1;
	if (total_a>n_bits)
	   d=1;
	if (total_a==n_bits)
	   return (total_a);
	if (total_a<n_bits)
	   d=-1;

	for (i=0;i<block->size;i++)
	    if (q_out[i]!=-1){
		   tmp=d*(tmp_q[i]/(int)Q_STEP);
		   if (tmp)
		      tmp_q[i]+=tmp;
	       else
			   tmp_q[i]+=d;
		   if (tmp_q[i]<8)
		      tmp_q[i]=8;
	    }

	total_b=huffman_estimate_image(image,tmp_q,dc,ac,bits_b,error_b);

	/* convert error a/b to rate distortions */
	for (i=0;i<block->size;i++){
		if (q_out[i]==-1)
		   continue;
	    rd[i]=(error_a[i]-error_b[i])/(bits_a[i]-bits_b[i]);
	}

	/* sort rate distorion */
	for (j=0;j<block->size/RD_CONDENSE_SPEED;j++){ /* only move smaller ones */
	    best_i=0;
	    best_rd=(float)1000000.0;
	    for (i=0;i<block->size;i++){
		if (best_rd>d*myabs(rd[i]) && q_out[i]!=-1 && rd[i]!=0.0){
		   best_rd=d*myabs(rd[i]);
		   best_i=i;
		}
	    }
	    rd[best_i]=(float)0.0;
	    total_a+=(int)(bits_b[best_i]-bits_a[best_i]);
	    q_out[best_i]=tmp_q[best_i];
	    if ((total_a<n_bits && d==1) || (total_a>n_bits && d==-1)){
	       break;
	    }
	}

	total_a=huffman_estimate_image(image,q_out,dc,ac,bits_a,error_a);

        free_int(tmp_q);
        free_float(error_a);
        free_float(error_b);
        free_float(bits_a);
        free_float(bits_b);
        free_float(rd);

	return (total_a);
}

static int max_coeff
(BLOCK *block,
 int coeff)
{
	int max;
	
	max=0;
	/* loop through linked list */
	while (block!=NULL){
	      if (myabs(block->pt[coeff])>max)
	         max=myabs(block->pt[coeff]);
	      block=block->next;
	}

	return max;

}

void convert_bits_to_quant
(IMAGE *image,
 float *x_out,
 int *q_out,
 int size)
{
	int i;


	for (i=0;i<size;i++){
	    /* skip loop is coefficent is not used */
	    if (x_out[i]==(float)0.0){
	       q_out[i]=-1;
	       continue;
	    }
	    /* find quantisation for each coefficient */
	    q_out[i]=find_quantisation(x_out[i],(image->data)+i);
	}					

}

static int find_quantisation
(float x,
 COEFF_PLANES *pt)
{
	int i;
	float q;
	
	for (i=0;i<pt->nitems-1;i++)
	    if (pt->x[i]<=x && pt->x[i+1]>x)
	       break;

	/* linearly predict q */
	q=(float)(pt->q[i])+(x-pt->x[i])*(float)(pt->q[i+1]-pt->q[i])
	                               /(pt->x[i+1]-pt->x[i]);	
	/* round it to the nearest int */
	q=(float)rint(q);

	return (int)q;	

}

static void unquantise_image
(IMAGE *image,
 int *q_out,
 int size)
{
        int i;
        BLOCK *block;
 
        for (i=0;i<size;i++){
            /* skip unused coefficients */
            if (q_out[i]==-1){
               block=image->tree_top;
               while (block!=NULL){
                     block->pt[i]=0;
                     block=block->next;
               } 
               continue;
            }
 
            /* scan image */
            block=image->tree_top;
            while (block!=NULL){
                  /* unquantise each coefficient inside blocks */
                  block->pt[i]*=q_out[i];
                  /* move to next block */
                  block=block->next;
            }
 
        }
 
}

static int quantise_image
(IMAGE *image,
 int *q_out,
 double *entropy,
 int size)
{
	int i,j,max;
	BLOCK *block;
	double total_entropy;
	int *bin_bottom,*bins,nitems;

        total_entropy=0;
	for (i=0;i<size;i++){
	    /* skip unused coefficients */
	    if (q_out[i]==-1){
	       block=image->tree_top;
	       while (block!=NULL){
                     block->pt[i]=0;
                     block=block->next;
	       }
	       entropy[i]=0;
	       continue;
	    }

	    max=(image->data+i)->max;
	    /* since min quantisation is 1 => nitems is 2*max (accounting for -/+)*/
            bin_bottom=malloc_int(2*max+1,"quantise image");
            bins=bin_bottom+max;

	    /* clear bins */
	    for (j=0;j<2*max+1;j++)
	        bin_bottom[j]=0;
	
 	    /* scan image */
 	    block=image->tree_top;
 	    nitems=0;
 	    while (block!=NULL){
            /* quantise each coefficient inside blocks */
                  block->pt[i]=quantise(block->pt[i],q_out[i]);
                  bins[block->pt[i]]++;
	          nitems++;
	          /* move to next block */
                  block=block->next;
            }

            /* calculate bits / pixel */
	    entropy[i]=0;
            for (j=-max;j<=max;j++)
                if (bins[j]!=0)
                    entropy[i]-=(double)bins[j]*log10((double)bins[j]/(double)nitems);
	    free(bin_bottom);
            entropy[i]/=log10(2.0);
            total_entropy+=entropy[i];

	}

	return (int)total_entropy;

}

static int find_no_ac_symbols
(IMAGE *image,
 int *q_out,
 float *x_out,
 int size,
 int low_high) /* 0 - low    1 - high */
{
	int i,max;
	float min;
	COEFF_PLANES *pt;

	min=(float)(image->rows*image->columns)/(float)(size);
	max=0;
	pt=image->data;
	for (i=1;i<size;i++){
	    if (low_high==1 && x_out[i]<min)
	       continue;

	    if (low_high==0 && x_out[i]>=min)
	       continue;

	    if (max<(int)(pt[i].max/q_out[i]))
	       max=(int)(pt[i].max/q_out[i]);
	}

	DebugF("%d\n",max);

	return max;

}

static void max_min_quant_coeff
(BLOCK *block,
 int coeff,
 int *min,
 int *max)
{
        *max=0;
        *min=0;
        /* loop through linked list */
        while (block!=NULL){
              if (block->pt[coeff]>(*max))
                 *max=block->pt[coeff];
              if (block->pt[coeff]<(*min))
                 *min=block->pt[coeff];
              block=block->next;
        }

}


