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


/* the functions below all perform standard memory allocation, however, */
/* if the VERBOSE flag is set to 1, using these routines allows one to track all */
/* memory usage as it occurs, and to locate any errors which occur due to mempry */
/* mis-management with ease */

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

