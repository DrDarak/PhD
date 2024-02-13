/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                    memory handling                           */
/*                      reworked 3/7/96                         */
/*##############################################################*/
#include <stdio.h>
#include <malloc.h>
#include "mem_hand.h"

#define VERBOSE 0 /* 1 prints all memory use */

IMAGE *malloc_IMAGE
(int size,
 char *loc)
{
	int cnt;
	IMAGE *pt;

	/* calloc memory */
	pt=(IMAGE *)calloc(size,sizeof(IMAGE));

	cnt=size*sizeof(IMAGE);

	/* check for errors */
	if (pt==NULL){
	   printf("\nNULL IMAGE pointer at %s\n",loc);
	   exit(-1);
	}

	if (VERBOSE)
	   printf ("mallocing %d btyes of IMAGE memory at %p\n",cnt,pt);

	return pt;
}	

void free_IMAGE
(IMAGE *pt)
{

	/* free image data */
	free_int(pt->pt);

	free(pt);

        if (VERBOSE)
           printf ("freeing IMAGE memory at %p\n",pt);

}

QUANT *malloc_QUANT
(int size,
 char *loc)
{
        int cnt;
        QUANT *pt;

        /* calloc memory */
        pt=(QUANT *)calloc(size,sizeof(QUANT));

        cnt=size*sizeof(QUANT);

        /* check for errors */
        if (pt==NULL){
           printf("\nNULL QUANT pointer at %s\n",loc);
           exit(-1);
        }

        if (VERBOSE)
           printf ("mallocing %d btyes of QUANT memory at %p\n",cnt,pt);

        return pt;
}

void free_QUANT
(QUANT *pt)
{

        /* free QUANT data */
	if (pt->q_out!=NULL)
           free_int(pt->q_out);
	if (pt->nitems!=NULL)
           free_int(pt->nitems);
	if (pt->nitems!=NULL)
           free_float(pt->q);

        free(pt);

        if (VERBOSE)
           printf ("freeing QUANT memory at %p\n",pt);

}

 HUFF *malloc_HUFF
(int size,
 char *loc)
{
        int cnt,i;
        HUFF *pt;

        /* calloc memory */
        pt=(HUFF *)calloc(size,sizeof(HUFF));

        cnt=size*sizeof(HUFF);

        /* check for errors */
        if (pt==NULL){
           printf("\nNULL HUFF pointer at %s\n",loc);
           exit(-1);
        }

        /* setup huff variables */
        for (i=0;i<size;i++){
            pt[i].dump=NULL;
            pt[i].huff=NULL;
            pt[i].huff_top=NULL;
            pt[i].min=0;
            pt[i].max=0;
            pt[i].run=0;
            pt[i].context=0;
        }
 
        if (VERBOSE)
           printf ("mallocing %d btyes of HUFF memory at %p\n",cnt,pt);
 
        return pt;
}
 
void free_HUFF
(HUFF *pt)
{
 
        free(pt);
 
        if (VERBOSE)
           printf ("freeing HUFF memory at %p\n",pt);
 
}
PACKED *malloc_PACKED
(int size,
 char *loc)
{
        int cnt;
        PACKED *pt;

        pt=(PACKED *)calloc(size,sizeof(PACKED));
        cnt=size*sizeof(PACKED);

        /* check for errors */   
        if (pt==NULL){
           printf("\nNULL PACKED pointer at %s\n",loc);
           exit(-1);
        }

        if (VERBOSE)
           printf ("mallocing %d bytes of PACKED memory at %p\n",cnt,pt);

        return pt;                                                        
}
 
void free_PACKED
(PACKED *pt)
{
 
        /* free packed data streams */
        if (pt->symbols_top!=NULL)
           free_unsigned_char(pt->symbols_top);

        free(pt);                               
 
        if (VERBOSE)
           printf ("freeing PACKED memory at %p\n",pt);
 
}
LEAF *malloc_LEAF
(int size,
 char *loc)
{
        int cnt;
        LEAF *pt;

        pt=(LEAF *)calloc(size,sizeof(LEAF));
        cnt=size*sizeof(LEAF);

        /* check for errors */ 
        if (pt==NULL){
           printf("\nNULL LEAF pointer at %s\n",loc);
           exit(-1);
        }

        if (VERBOSE)
           printf ("mallocing %d bytes of LEAF memory at %p\n",cnt,pt);
 
        return pt;
}
 
void free_LEAF
(LEAF *pt)
{
 
        free(pt);
 
        if (VERBOSE)
           printf ("freeing LEAF memory at %p\n",pt);
 
}

COEFF_PLANES *malloc_COEFF_PLANES
(int size,
 char *loc)
{
        int cnt;
        COEFF_PLANES *pt;

        /* calloc memory */
        pt=(COEFF_PLANES *)calloc(size,sizeof(COEFF_PLANES));

        cnt=size*sizeof(COEFF_PLANES);

        /* check for errors */
        if (pt==NULL){
           printf("\nNULL COEFF_PLANES pointer at %s\n",loc);
           exit(-1);
        }

        if (VERBOSE)
           printf ("mallocing %d btyes of COEFF_PLANES memory at %p\n",cnt,pt);

        return pt;
}

void free_COEFF_PLANES
(COEFF_PLANES *pt)
{

        /* free image data */
	if (pt->x!=NULL)
	   free_float(pt->x);
	if (pt->e!=NULL)
	   free_float(pt->e);
	if (pt->dx!=NULL)
	   free_float(pt->dx);
	if (pt->de!=NULL)
	   free_float(pt->de);
	if (pt->q!=NULL)
	   free_int(pt->q);

        free(pt);

        if (VERBOSE)
           printf ("freeing COEFF_PLANES memory at %p\n",pt);

}

BLOCK *malloc_BLOCK
(int size,
 char *loc)
{
        int cnt;
        BLOCK *pt;

        /* calloc memory */
        pt=(BLOCK *)calloc(size,sizeof(BLOCK));

        cnt=size*sizeof(BLOCK);

	pt->pt=NULL;
	pt->next=NULL;

        /* check for errors */
        if (pt==NULL){
           printf("\nNULL BLOCK pointer at %s\n",loc);
           exit(-1);
        }

        if (VERBOSE)
           printf ("mallocing %d btyes of BLOCK memory at %p\n",cnt,pt);

        return pt;
}

void free_BLOCK
(BLOCK *pt)
{

        /* free block data */
	if (pt->pt!=NULL)
           free_int(pt->pt);

        free(pt);

        if (VERBOSE)
           printf ("freeing BLOCK memory at %p\n",pt);

}


WAVELET *malloc_WAVELET
(int size,
 char *loc)
{
	int cnt;
        WAVELET *pt; 
 
        pt=(WAVELET *)calloc(size,sizeof(WAVELET));
	cnt=size*sizeof(WAVELET);
 
        /* check for errors */
        if (pt==NULL){
           printf("\nNULL WAVELET pointer at %s\n",loc);
           exit(-1);
        }
 
        if (VERBOSE)
           printf ("mallocing %d bytes of WAVELET memory at %p\n",cnt,pt);
 
        return pt; 
}        

void free_WAVELET
(WAVELET *pt)
{
        float *ccmem,*cumem;

        ccmem=pt->c+pt->cmin;
        free_float(ccmem);

        cumem=pt->u+pt->umin;
        free_float(cumem);

        free(pt);

        if (VERBOSE)
           printf ("freeing WAVELET memory at %p\n",pt);


}

int *malloc_int
(int size,
 char *loc)
{
	int cnt;
        int *pt; 
 
        pt=(int *)calloc(size,sizeof(int));
	cnt=size*sizeof(int);
 
        /* check for errors */
        if (pt==NULL){
           printf("\nNULL int pointer at %s\n",loc);
           exit(-1);
        }
 
        if (VERBOSE)
           printf ("mallocing %d bytes of int memory at %p\n",cnt,pt);
 
        return pt; 
}        

void free_int
(int *pt)
{

        free(pt);

        if (VERBOSE)
           printf ("freeing int memory at %p left\n",pt);

}

unsigned char *malloc_unsigned_char
(int size,
 char *loc)
{
	int cnt;
        unsigned char *pt; 
 
        pt=(unsigned char *)calloc(size,sizeof(unsigned char));
	cnt=size*sizeof(unsigned char);
 
        /* check for errors */
        if (pt==NULL){
           printf("\nNULL unsigned char pointer at %s\n",loc);
           exit(-1);
        }
 
        if (VERBOSE)
           printf ("mallocing %d bytes of unsigned char memory at %p\n",cnt,pt);
 
        return pt; 
}        

void free_unsigned_char
(unsigned char *pt)
{

        free(pt);

        if (VERBOSE)
           printf ("freeing unsigned char memory at %p\n",pt);

}

void NULL_FILE_error
(FILE *pt,
 char *loc)
{
	/* check for errors */
        if (pt==NULL){
           printf("\nNULL FILE pointer at %s\n",loc);
           exit(-1);
        }

 
}        

float *malloc_float
(int size,
 char *loc)
{
        int cnt;
        float *pt;
           
        pt=(float *)calloc(size,sizeof(float));
        cnt=size*sizeof(float);
 
        /* check for errors */
        if (pt==NULL){
           printf("\nNULL float pointer at %s\n",loc);
           exit(-1);
        }
 
        if (VERBOSE)
           printf ("mallocing %d bytes of int memory at %p\n",cnt,pt);
 
        return pt;                                                     
}
 
void free_float
(float *pt)
{
 
        free(pt);
 
        if (VERBOSE)
           printf ("freeing float memory at %p left\n",pt);
 
}

