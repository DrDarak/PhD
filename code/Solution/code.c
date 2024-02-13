/*##############################################################*/
/*					Test of wavelet coder 						*/
/*##############################################################*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "miscw.h"
#include "struct.h"

#include "code.h" 

#include "mem_hand.h"
#include "filters.h"
#include "blocks.h"
#include "optim_q.h"
#include "solution.h"
#include "q_tables.h"

/****************************************************************/
/*		 Routines defined & ony used within this module	        */
/****************************************************************/

void create_optimal_qtable
(IMAGE *image,
 QUANT *quant,
 float xt,
 int scales)
{
 int i, size,total;
	float *x_out;

 size=(1<<(scales))*(1<<(scales-1));

 /* aplly wide ban quantisation to generate quantiation tables */
 form_quantisation_tables(image,size);
 
 quantise_coefficients(image,size);

 /* alloccate memoey for output */
 x_out=malloc_float(size,"compress");

 /* solve problem to within 1000 bits */
 solve(image->data,x_out,xt,1000.0,size);


 /* clear qauntisation tables */
 total=0;
 for (i=0;i<quant->size;i++)
     {
	  total+=(int)x_out[i];
      quant->q_out[i]=-1; 
      quant->nitems[i]=1; 
     }

  printf("%d\n",total);

 /* Now need to covert bits allocated to each coefficient -> quantisation */
 convert_bits_to_quant(image,x_out,quant->q_out,size);

 /* set nitems to zero when q is not used */
 for (i=0;i<quant->size;i++)
     {
    	 if (quant->q_out[i]==-1)
         quant->nitems[i]=0; 
     }

 free_float(x_out);
}


void improve_qtables
(IMAGE *image,
 QUANT *quant,
 QUANT_TYPE mode,
 float xt,
 HUFF *dc,
 HUFF *ac,
 int scales)
{
 int *q_out=NULL,diff,accuracy,size,i,cnt=0;

 accuracy=500; /* -+ 1000 bits */
 size=(1<<(scales))*(1<<(scales-1));
 
 q_out=malloc_int(quant->size,"q_out compress image");

 /* copy actual quant table into new quant table */
 for (i=0;i<quant->size;i++)
     q_out[i]=quant->q_out[i];

 /* ammend quantsaition values */
 diff=2*accuracy;
 while(cnt<4){

        diff=ammend_quantisation (image,q_out,dc,ac,(int)xt);
		DebugF("diff=%d\n",diff);
		diff-=(int)xt;
		diff=myabs(diff);

		if (diff<accuracy)
		   cnt++;
		else
			cnt=0;
 }

 if (mode==create)
    {
     /* replace old table with new */
     if (quant->q_out!=NULL)
        free_int(quant->q_out);
     quant->q_out=q_out;

     /* fill in floating point version */
     for (i=0;i<quant->size;i++)
         quant->q[i]=(float)quant->q_out[i];
    }
 else 
    for (i=0;i<quant->size;i++)
        if (q_out[i]!=-1)
           {
            /* if q is used then weighted average results */
            quant->q[i]=((float)(quant->nitems[i])*quant->q[i]+(float)q_out[i])/
                        (float)(quant->nitems[i]+1);
            quant->q_out[i]=(int)(quant->q[i]);
            quant->nitems[i]++;	
           }
        else
           quant->q[i]=-1;
}


void compress_image
(IMAGE *image,
 HARDCODED_WAVELET *c,
 QUANT *quant,
 HUFF_TYPE type,
 QUANT_TYPE mode,
 float xt,
 HUFF *dc,
 HUFF *ac,
 int scales,
 int doReform,
 int doHuff)
{
 wavelet_filter_image(image,c,scales,1); /* turn image into a wavelet filtered image */

 if (doReform)
    {
     rend_image_into_blocks(image,scales); /* turn filtered image a list of 'trees' attached to 'image' */

     if (doHuff)
        {
	        /* create optimal q table from present image */
         if (mode==create)
            create_optimal_qtable(image, quant, xt, scales);

   	     /* inprove quantisation tables */
		 if (mode==create || mode==update)
            if (type==normal || type==fix)
               improve_qtables(image, quant, mode, xt, dc, ac, scales);

         image->dump.Rewind(false);
         image->dump.StartWriteBits();

         huffman_encode_image(image,quant->q_out,dc,ac);
 
         DebugF("HUFFMAN BITS %d\n",image->dump.DirectTellBit());
         image->dump.EndWriteBits();
	       }
	   }
}

void decompress_image
(IMAGE *image,
 HARDCODED_WAVELET *c,
 QUANT *quant,
 HUFF *dc,
 HUFF *ac,
 int scales,
 int undoReform,
 int undoHuff)
{
 if (undoHuff)
    {
  	  image->dump.Rewind(true);
		   image->dump.StartReadBits();
     huffman_decode_image (image,quant->q_out,dc,ac);
		   image->dump.EndReadBits();
    }

 if (undoReform)
        reform_image (image,scales);
        wavelet_inv_filter_image (image,c,scales,1);

}

float get_mse(IMAGE *original, IMAGE *image)
{
	int i;
	int diff,error;
	float mse;

	error=0;
	for (i=0;i<image->rows*image->columns;i++){
		diff=image->pt[i]-original->pt[i];
		error+=diff*diff;
	}

	mse=(float)(error)/(float)(image->rows*image->columns);

	return mse;
}

void imgcpy(IMAGE *dest,IMAGE *source)
{
	int i;

	dest->rows=source->rows;
	dest->columns=source->columns;

	/* malloc memory if necessary */
	if (dest->pt==NULL)
  	   dest->pt=malloc_int(dest->rows*dest->columns,"copy image to imager");

	for (i=0;i<source->rows*source->columns;i++)
		dest->pt[i]=source->pt[i];

	return;
}

