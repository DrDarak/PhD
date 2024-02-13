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
#include "optimal_q.h"


void read_quantisation
(char *fname,
 QUANT *quant)
{

	int i;
	FILE *fp;
	
	/* open and read quant file */
	fp=fopen(fname,"rb");
	NULL_FILE_error(fp,"read quant");

	/* read quant file */
	for (i=0;i<quant->size;i++){
	    fscanf(fp,"%d\t%d\n",quant->q_out+i,quant->nitems+i);
	    quant->q[i]=(float)(quant->q_out[i]);
	}
	fclose(fp);
	
}

void write_quantisation
(char *fname,
 QUANT *quant)
{	

	int i;
	FILE *fp;

	/* open and write quant file */
	fp=fopen(fname,"wb");
	NULL_FILE_error(fp,"write quant");
	for (i=0;i<quant->size;i++){
	    fprintf(fp,"%d\t%d\n",(int)rint(quant->q[i]),quant->nitems[i]);
	}
	fclose(fp);

}

int quantise
(int c,
 int q)
{
	int cq;
	cq=(int)rint((float)c/(float)q);	
	return cq;
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
	    factor=log((float)max)/log(2);
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

void RD_curve_by_entropy
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
	       for (j=i;j<pt->nitems;j++)
	     	   pt->q[j-1]=pt->q[j]; 	
	       /* decrease size of list and bring back list pt */
	       pt->nitems--;

	       /* do not write effects to RD graph */
	       if (i<pt->nitems)
		  continue;
	       else 
		   break;
	    }

	    pt->x[i]=entropy/log10(2.0);
	    pt->e[i]=(float)error;

	    /* finish loop */
	    i++;
	    last_entropy=entropy;
	}

	free(bin_bottom);
}

int huffman_estimate_image
(IMAGE *image,
 int *q_out,
 HUFF *dc,
 HUFF *ac,
 float *bits,
 float *error)
{
	BLOCK *block;
	int diff,cq,i,j,run,context,start,total;
	float bit;

	block=image->tree_top;

	for (i=0;i<block->size;i++){
	    bits[i]=0.0;
	    error[i]=0.0;
	}	

	while (block!=NULL){
	      context=32768; 
	      run=0;
	      start=-1;
	      cq=quantise(block->pt[0],q_out[0]);
	      bits[0]+=(float)(dc->huff[cq-dc->min].length); 
	      diff=block->pt[0]-cq*q_out[0];
	      error[0]+=diff*diff;
	      /*dc->huff[cq-dc->min].nitems++*/; 

	      for (i=1;i<block->size;i++){	
		  /* quantise coeff */
	          if (q_out[i]!=-1){
	             cq=quantise(block->pt[i],q_out[i]);
	             diff=block->pt[i]-cq*q_out[i];
		  }
		  else{
	             cq=0;
	             diff=block->pt[i];
		  }

		  /* find error in coeff */
	          error[i]+=(float)(diff*diff);

		  if (cq==context){
		     if (start==-1)
			start=i;
		     run++; 
		  }
		  else 
		      if (run==0){
		         bits[i]+=(float)(ac->huff[cq-ac->min].length);
	      		 /*ac->huff[cq-ac->min].nitems++*/; 
		      }
		      else {
		           bit=(float)(ac->huff[run+ac->max-ac->min].length);
	                   /*ac->huff[run+ac->max-ac->min].nitems++*/; 
		           for (j=start;j<start+run;j++)
			       bits[j]+=bit/(float)run;
		           run=0;
		           start=-1;
		           bits[i]+=(float)(ac->huff[cq-ac->min].length);
		      }
		  context=cq;
	      }
	      if (run!=0){
	         bit=(float)(ac->huff[run+ac->max-ac->min].length);
	         for (j=start;j<start+run;j++)
                     bits[j]+=bit/(float)run;

	      }
	      block=block->next;

	}
	total=0;
	for (i=0;i<image->tree_top->size;i++){
	    total+=(int)bits[i];
	}

	return total;

}

int ammend_quantisation
(IMAGE *image,
 int *q_out, 
 HUFF *dc, 
 HUFF *ac,
 int n_bits)
{
	int i,j,total_a,total_b,d;
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
	    if (q_out[i]!=-1)
	       tmp_q[i]+=d;

	total_b=huffman_estimate_image(image,tmp_q,dc,ac,bits_b,error_b);

	/* convert error a/b to rate distortions */
	for (i=0;i<block->size;i++){
		if (q_out[i]==-1)
		   continue;
	    rd[i]=(error_a[i]-error_b[i])/(bits_a[i]-bits_b[i]);
	}

	/* sort rate distorion */
	for (j=0;j<block->size/3;j++){ /* only move smaller ones */
	    best_i=0;
	    best_rd=1000000;
	    for (i=0;i<block->size;i++){
		if (best_rd>d*myabs(rd[i]) && q_out[i]!=-1 && rd[i]!=0.0){
		   best_rd=d*myabs(rd[i]);
		   best_i=i;
		}
	    }
	    rd[best_i]=0.0;
	    total_a+=(int)(bits_b[best_i]-bits_a[best_i]);
	    q_out[best_i]=tmp_q[best_i];
	    if ((total_a<n_bits && d==1) || (total_a>n_bits && d==-1)){
	       break;
	    }
	}

        free_int(tmp_q);
        free_float(error_a);
        free_float(error_b);
        free_float(bits_a);
        free_float(bits_b);
        free_float(rd);

	return (total_a);
}

void huffman_encode_image
(IMAGE *image,
 int *q_out,
 HUFF *dc,
 HUFF *ac)
{
        BLOCK *block;

        block=image->tree_top;
        while (block!=NULL){
	      huffman_encode_block(block,q_out,dc,ac);
              block=block->next;
               
        }
 
}

inline void huffman_encode_block
(BLOCK *block,
 int *q_out,
 HUFF *dc,
 HUFF *ac)
{
        int cq,i,run,context;

	context=32768;
        run=0;
        cq=quantise(block->pt[0],q_out[0]);
        compress_symbol(dc,cq);
        for (i=1;i<block->size;i++){
            if (q_out[i]!=-1)
               cq=quantise(block->pt[i],q_out[i]);
            else
                cq=0;
            if (cq==context)
               run++;
            else
                if (run==0)
                   compress_symbol(ac,cq);
                else {
                     compress_run(ac,run);
                     run=0;
                     compress_symbol(ac,cq);
                }
            context=cq;
        }
        if (run!=0)
           compress_run(ac,run);

	return;

}

void huffman_decode_image
(IMAGE *image,
 int *q_out,
 HUFF *dc,
 HUFF *ac)
{
        BLOCK *block;
 
        block=image->tree_top;
        while (block!=NULL){
	      huffman_decode_block(block,q_out,dc,ac);
              block=block->next;
        }                        

}         

inline void huffman_decode_block
(BLOCK *block,
 int *q_out, 
 HUFF *dc, 
 HUFF *ac)
{
        int i,j,run,run_symbol,context;

	context=32768;
        block->pt[0]=q_out[0]*uncompress_symbol(dc);
        i=1;
        while (i<block->size){
              run_symbol=uncompress_run_symbol(ac,&run);
              if (run){
                 if (i+run_symbol>=block->size)
                    run_symbol=block->size-i;
                 for (j=0;j<run_symbol;j++){
                     block->pt[i++]=context;
                 }
              }
              else {
                   if (q_out[i]!=-1)
                      context=q_out[i]*run_symbol;
                   else
                       context=0;
                   block->pt[i++]=context;
             }
        }

}

int max_coeff
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
	    if (x_out[i]==0.0){
	       q_out[i]=-1;
	       continue;
	    }
	    /* find quantisation for each coefficient */
	    q_out[i]=find_quantisation(x_out[i],(image->data)+i);
	}
}

int find_quantisation
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
	q=rint(q);

	return (int)q;	

}
void unquantise_image
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

int quantise_image
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

int find_no_ac_symbols
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

	printf("%d\n",max);
	return max;

}

void max_min_quant_coeff
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


