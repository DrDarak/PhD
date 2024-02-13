/*##############################################################*/
/*			David Bethel				*/
/*		       Bath University 				*/
/*		      daveb@ee.bath.ac.uk			*/
/*		Lossly vector quantisation			*/	
/*		  	 11/11/96				*/
/*##############################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mem_hand.h"
#include "huff.h"
#include "vq.h"

/*------------------------------------------*/
/* WARNING THIS PROGRAM ASSUMES WHITE PAPER */
/*	i.e. black marks are detail	    */
/*------------------------------------------*/

void derive_mapping_from_pdfs
(int *map,
 PACKED *qtree,
 int comp)
{

	int i,j,k,best_i,best_error,best_match,match;
	int nitems=1<<(MIN_SIZE*MIN_SIZE);
	int *error; /* error set to -1 to indicate remap block */
	int mask,sum,budget,total_error;

	/* first make mapping to orginal without compression */
	map[0]=-1; /* black */
	for (i=1;i<nitems-1;i++)
	    map[i]=i;
	map[nitems-1]=-2; /* white */

	/* return if lossless compression */
	if (comp==100){
	   printf("Lossless Compression\n");
	   return ;
	}

	/* list of errors caused by each vq index */
	error=malloc_int(nitems,"error list in derive mappings");

	/* work out error for each index */
	for (i=0;i<nitems;i++){
	    sum=0;
	    mask=1;
	    for (j=0;j<MIN_SIZE*MIN_SIZE;j++){
		if (mask&i)
		   sum++;
		mask<<=1;
	    }
	    error[i]=MIN_SIZE*MIN_SIZE-sum; /* assumes white paper */
	}

	/* scale errors by number of occurences in pdf */
	sum=0;
	for (i=0;i<nitems;i++){
	    /* maps are used to map 0 and 511 onto black/white blocks */
	    error[i]*=qtree->huff[map[i]-qtree->min].nitems;
	    sum+=error[i];
	}
	
	/* work out limits for pruning mechanism */
	total_error=sum;
	budget=(sum*comp)/100;
	
	/* scan list of errors and delete until budget satisfied */
	while (sum>budget){
	      /* find lowest error improvement */
	      best_i=0;
	      best_error=0x0fffffff; /* max int */
	      for (i=0;i<nitems;i++)
		  if (error[i]!=-1)
		     if (error[i]<best_error){
			best_error=error[i];
		        best_i=i;
		     }

	      /* remove lowest error improvement from list */
	      sum-=best_error;
	      error[best_i]=-1;
	}

	/* replace indexs with nearest matches + change huffman tables */
	for (i=0;i<nitems;i++)
	    if (error[i]==-1){
	       /* search for best replacement -> maximum match measure */
	       best_i=0;
	       best_match=0;
	       for (j=0;j<nitems;j++){

		   /* ignore other replaced indexes */ 
		   if (error[j]==-1)
		      continue;

		   /* match indexes */
	           sum=0;
	           mask=1;
		   match=~(i^j); /* assumes white match image */
	           for (k=0;k<MIN_SIZE*MIN_SIZE;k++){
		       if (mask&match)
		          sum++;
		       mask<<=1;
		   }
	           sum*=qtree->huff[map[i]-qtree->min].nitems;
		   if (sum>best_match){
		      best_i=j;
		      best_match=sum;
		   }

	       }

	       /* save best mapping */
	       map[i]=best_i;

	       /* make huffman pdf change */
	       qtree->huff[map[i]-qtree->min].nitems+=
	                          qtree->huff[i-qtree->min].nitems;
	       qtree->huff[i-qtree->min].nitems=1;
	    }

	/* rework huffman tables */

	/* free existing huffman tree */
	free_tree(qtree->huff_top);

	/*  reconstruct original list */
        for (i=qtree->min;i<qtree->max;i++)
            qtree->huff[i-qtree->min].next=qtree->huff[i-qtree->min].orig;
 
        /* initialise huffman tables */
        qtree->huff_top=construct_huff_tree(qtree->huff);

	/* clear mess */
	free_int(error);

	

}

void map_indexes
(IMAGE *image,
 int *map)
{
	BLOCK *block;
	
	/* scan block list and map indexes where appropriate*/
	block=image->tree_top;
	while (block!=NULL){
	      if (block->type==vq)
	         block->index=map[block->index];
	      block=block->next;
	}

}


