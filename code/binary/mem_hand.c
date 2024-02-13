/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                    memory handling                           */
/*                      reworked 3/7/96                         */
/*##############################################################*/
#include <stdio.h>
#include <stdlib.h>
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
	pt->pt=NULL;

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
	if (pt->pt!=NULL)
	   free_unsigned_char(pt->pt);

	free(pt);

        if (VERBOSE)
           printf ("freeing IMAGE memory at %p\n",pt);

}

BLOCK *malloc_BLOCK
(int size,
 char *loc)
{
        int cnt; 
        BLOCK *pt;
 
        pt=(BLOCK *)calloc(size,sizeof(BLOCK));
        cnt=size*sizeof(BLOCK);
	pt->pt=NULL;
 
        /* check for errors */
        if (pt==NULL){
           printf("\nNULL BLOCK pointer at %s\n",loc);
           exit(-1);
        }
 
        if (VERBOSE)
           printf ("mallocing %d bytes of BLOCK memory at %p\n",cnt,pt);
 
        return pt;
}        
 
void free_BLOCK  
(BLOCK *pt) 
{
	/* free memory inside block */
	if (pt->pt!=NULL)
           free_int(pt->pt);
 
        free(pt);
 
        if (VERBOSE)
           printf ("freeing BLOCK memory at %p\n",pt);
 
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
	   /* this is the correct error for programmers 
              but insted lets put a stupid none informative
	      error message 
           printf("\nNULL FILE pointer at %s\n",loc);*/
           printf("\nCan not find input file\n");
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
           printf ("mallocing %d bytes of float  memory at %p\n",cnt,pt);
 
        return pt;                                                     
}
 
void free_float
(float *pt)
{
 
        free(pt);
 
        if (VERBOSE)
           printf ("freeing float memory at %p \n",pt);
 
}

