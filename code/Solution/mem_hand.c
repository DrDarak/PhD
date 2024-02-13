/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                    memory handling                           */
/*                      reworked 3/7/96                         */
/*##############################################################*/

#include <stdlib.h>
#include <stdio.h>

#ifndef __cplusplus
#  include <malloc.h>
#endif

#include "miscw.h"
#include "struct.h"
#include "bitstrm.h"
#include "mem_hand.h"

#ifdef __cplusplus
#  include <memleak.h> /* must be the last .h file included */
#endif

#define VERBOSE 0 /* 1 prints all memory use */

/*--------------------------------------------------------------------------*/

#ifndef __cplusplus

IMAGE *malloc_IMAGE(int   size,
                    char *loc)
{
 int cnt;
 IMAGE *pt;

 cnt=size*sizeof(IMAGE);

 pt=(IMAGE *)calloc(size,sizeof(IMAGE));

 if (pt==NULL)
    Fatal("\nNULL IMAGE pointer at %s\n",loc);

 if (VERBOSE)
    DebugF("mallocing %d btyes of IMAGE memory at %p\n",cnt,pt);

 return pt;
} 

#endif

/*--------------------------------------------------------------------------*/

#ifndef __cplusplus

void free_IMAGE(IMAGE *pt)
{
 free_int(pt->pt);

 free(pt);

 if (VERBOSE)
    DebugF("freeing IMAGE memory at %p\n",pt);
}

#endif

/*--------------------------------------------------------------------------*/

IMAGE::IMAGE(float bpp,int c,int r) : dump ((int)((float)(r*c)*bpp)) 
{
 columns=c;
 rows=r; 
 pt=NULL;
 tree_top=NULL;
 data=NULL;
}

/*--------------------------------------------------------------------------*/

IMAGE::~IMAGE()
{
 free_int(pt);
 // may need more 
}

/*--------------------------------------------------------------------------*/

QUANT *malloc_QUANT(int   size,
                    char *loc)
{
 int cnt;
 QUANT *pt;

 cnt=size*sizeof(QUANT);

#ifdef __cplusplus
 pt = new QUANT[size];
 if (pt)
    ::memset(pt,0,cnt);
#else
 pt=(QUANT *)calloc(size,sizeof(QUANT));
#endif

 if (pt==NULL)
    Fatal("\nNULL QUANT pointer at %s\n",loc);

 if (VERBOSE)
    DebugF("mallocing %d btyes of QUANT memory at %p\n",cnt,pt);

 return pt;
}

/*--------------------------------------------------------------------------*/

void free_QUANT(QUANT *pt)
{
 /* free QUANT data */
 if (pt->q_out!=NULL)
    free_int(pt->q_out);

 if (pt->nitems!=NULL)
    free_int(pt->nitems);

 if (pt->nitems!=NULL)
    free_float(pt->q);

#ifdef __cplusplus
 delete [] pt;
#else
 free(pt);
#endif

 if (VERBOSE)
    DebugF("freeing QUANT memory at %p\n",pt);
}

/*--------------------------------------------------------------------------*/

HUFF *malloc_HUFF(int   size,
                  char *loc)
{
 int cnt;
 HUFF *pt;

 cnt=size*sizeof(HUFF);

#ifdef __cplusplus
 pt = new HUFF[size];
 if (pt)
    ::memset(pt,0,cnt);
#else
 pt=(HUFF *)calloc(size,sizeof(HUFF));
#endif

 if (pt==NULL)
    Fatal("\nNULL HUFF pointer at %s\n",loc);

 if (VERBOSE)
    DebugF("mallocing %d btyes of HUFF memory at %p\n",cnt,pt);

 return pt;
}
 
/*--------------------------------------------------------------------------*/

void free_HUFF(HUFF *pt)
{
#ifdef __cplusplus
 delete [] pt;
#else
 free(pt);
#endif

 if (VERBOSE)
    DebugF("freeing HUFF memory at %p\n",pt);
}

/*--------------------------------------------------------------------------*/

LEAF *malloc_LEAF(int   size,
                  char *loc)
{
 int cnt;
 LEAF *pt;

 cnt=size*sizeof(LEAF);

#ifdef __cplusplus
 pt = new LEAF[size];
 if (pt)
    ::memset(pt,0,cnt);
#else
 pt=(LEAF *)calloc(size,sizeof(LEAF));
#endif

 if (pt==NULL)
    Fatal("\nNULL LEAF pointer at %s\n",loc);

 if (VERBOSE)
    DebugF("mallocing %d bytes of LEAF memory at %p\n",cnt,pt);

 return pt;
}
 
/*--------------------------------------------------------------------------*/

void free_LEAF(LEAF *pt)
{
#ifdef __cplusplus
 delete [] pt;
#else
 free(pt);
#endif

 if (VERBOSE)
    DebugF("freeing LEAF memory at %p\n",pt);
}

/*--------------------------------------------------------------------------*/

COEFF_PLANES *malloc_COEFF_PLANES(int   size,
                                  char *loc)
{
 int cnt;
 COEFF_PLANES *pt;

 cnt=size*sizeof(COEFF_PLANES);

#ifdef __cplusplus
 pt = new COEFF_PLANES[size];
 if (pt)
    ::memset(pt,0,cnt);
#else
 pt=(COEFF_PLANES *)calloc(size,sizeof(COEFF_PLANES));
#endif

 if (pt==NULL)
    Fatal("\nNULL COEFF_PLANES pointer at %s\n",loc);

 if (VERBOSE)
    DebugF("mallocing %d btyes of COEFF_PLANES memory at %p\n",cnt,pt);

 return pt;
}

/*--------------------------------------------------------------------------*/

void free_COEFF_PLANES(COEFF_PLANES *pt)
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

#ifdef __cplusplus
 delete [] pt;
#else
 free(pt);
#endif

 if (VERBOSE)
    DebugF("freeing COEFF_PLANES memory at %p\n",pt);
}

/*--------------------------------------------------------------------------*/

BLOCK *malloc_BLOCK(int   size,
                    char *loc)
{
 int cnt; //,i;
 BLOCK *pt;

 cnt=size*sizeof(BLOCK);

#ifdef __cplusplus
 pt = new BLOCK[size];
 if (pt)
    ::memset(pt,0,cnt);
#else
 pt=(BLOCK *)calloc(size,sizeof(BLOCK));
#endif

 if (!pt)
     Fatal("\nNULL BLOCK pointer at %s\n",loc);

 if (VERBOSE)
    DebugF("mallocing %d btyes of BLOCK memory at %p\n",cnt,pt);

 return pt;
}

/*--------------------------------------------------------------------------*/

void free_BLOCK(BLOCK *pt)
{
#ifdef __cplusplus
 delete [] pt;
#else
 free(pt);
#endif

 if (VERBOSE)
     DebugF("freeing BLOCK memory at %p\n",pt);
}

/*--------------------------------------------------------------------------*/

WAVELET *malloc_WAVELET(int   size,
                        char *loc)
{
 int cnt;
 WAVELET *pt; 

 cnt=size*sizeof(WAVELET);

#ifdef __cplusplus
 pt = new WAVELET[size];
 if (pt)
    ::memset(pt,0,cnt);
#else
 pt=(WAVELET *)calloc(size,sizeof(WAVELET));
#endif

 if (pt==NULL)
    Fatal("\nNULL WAVELET pointer at %s\n",loc);

 if (VERBOSE)
    DebugF("mallocing %d bytes of WAVELET memory at %p\n",cnt,pt);

 return pt; 
}        

/*--------------------------------------------------------------------------*/

void free_WAVELET(WAVELET *pt)
{
 double *ccmem,*cumem;

 ccmem=pt->c+pt->cmin;
 free_double(ccmem);

 cumem=pt->u+pt->umin;
 free_double(cumem);

 cumem=pt->u_pm+pt->umin;
 free_double(cumem);

 cumem=pt->memLow;
 delete [] cumem;

 cumem=pt->memHigh;
 delete [] cumem;

#ifdef __cplusplus
 delete [] pt;
#else
 free(pt);
#endif

 if (VERBOSE)
    DebugF("freeing WAVELET memory at %p\n",pt);
}

/*--------------------------------------------------------------------------*/

int *malloc_int(int   size,
                char *loc)
{
 int cnt;
 int *pt; 

 cnt = size*sizeof(int);

#ifdef __cplusplus
 pt  = new int[size];
 if (pt)
    ::memset(pt,0,cnt);
#else
 pt=(int *)calloc(size,sizeof(int));
#endif

 if (!pt)
    Fatal("\nNULL int pointer at %s\n",loc);

 if (VERBOSE)
    DebugF("mallocing %d bytes of int memory at %p\n",cnt,pt);

 return pt; 
}        

/*--------------------------------------------------------------------------*/

void free_int(int *pt)
{
#ifdef __cplusplus
 delete [] pt;
#else
 free(pt);
#endif

 if (VERBOSE)
    DebugF("freeing int memory at %p left\n",pt);
}

/*--------------------------------------------------------------------------*/

unsigned char *malloc_unsigned_char(int   size,
                                    char *loc)
{
 int cnt;
 unsigned char *pt; 

 cnt=size*sizeof(unsigned char);

#ifdef __cplusplus
 pt - new unsigned char[size];
 if (pt)
    ::memset(pt,0,cnt);
#else
 pt=(unsigned char *)calloc(size,sizeof(unsigned char));
#endif

 if (pt==NULL)
    Fatal("\nNULL unsigned char pointer at %s\n",loc);

 if (VERBOSE)
    DebugF("mallocing %d bytes of unsigned char memory at %p\n",cnt,pt);

 return pt; 
}        

/*--------------------------------------------------------------------------*/

void free_unsigned_char(unsigned char *pt)
{
#ifdef __cplusplus
 delete [] pt;
#else
 free(pt);
#endif

 if (VERBOSE)
    DebugF("freeing unsigned char memory at %p\n",pt);
}

/*--------------------------------------------------------------------------*/

void NULL_FILE_error(FILE *pt,
                     char *loc)
{
 if (pt==NULL)
    Fatal("\nNULL FILE pointer at %s\n",loc);
}        

/*--------------------------------------------------------------------------*/

float *malloc_float(int   size,
                    char *loc)
{
 int cnt;
 float *pt;
    
 cnt=size*sizeof(float);
 
#ifdef __cplusplus
 pt = new float[size];
 if (pt)
    ::memset(pt,0,cnt);
#else
 pt=(float *)calloc(size,sizeof(float));
#endif

 if (pt==NULL)
    Fatal("\nNULL float pointer at %s\n",loc);

 if (VERBOSE)
    DebugF("mallocing %d bytes of int memory at %p\n",cnt,pt);

 return pt;                                                     
}
 
/*--------------------------------------------------------------------------*/

void free_float(float *pt)
{
#ifdef __cplusplus
 delete [] pt;
#else
 free(pt);
#endif

 if (VERBOSE)
    DebugF("freeing float memory at %p left\n",pt);
}

/*--------------------------------------------------------------------------*/

double *malloc_double(int   size,
                      char *loc)
{
 int cnt;
 double *pt;
   
 cnt=size*sizeof(double);
 
#ifdef __cplusplus
 pt = new double[size];
 if (pt)
    ::memset(pt,0,cnt);
#else
 pt=(double *)calloc(size,sizeof(double));
#endif

 if (pt==NULL)
    Fatal("\nNULL float pointer at %s\n",loc);

 if (VERBOSE)
    DebugF("mallocing %d bytes of int memory at %p\n",cnt,pt);

 return pt;                                                     
}
 
/*--------------------------------------------------------------------------*/

void free_double(double *pt)
{
#ifdef __cplusplus
 delete [] pt;
#else
 free(pt);
#endif

 if (VERBOSE)
    DebugF("freeing float memory at %p left\n",pt);
}

/*--------------------------------------------------------------------------*/
