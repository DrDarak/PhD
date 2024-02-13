/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                    Block handleing                           */
/*                      reworked 28/8/96                        */
/*##############################################################*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "blocks.h"
#include "mem_hand.h"
#include "render.h"

/* defines */
#define PI 3.142

/* macros */
#define myabs(x)    ( x < 0 ? -x : x)

/* put blocks back ito an image for writing  */
void reform_image
(IMAGE *image)
{
        int i,j,*image_ptr,*block_ptr;
        BLOCK *block;

	// no longer necessary
	return;
         
        /* allocate memory for image */
        if (image->pt!=NULL)
           free_int(image->pt);
        image->pt=malloc_int(image->columns*image->rows,"reform image");

        block=image->tree_top;
        while (block!=NULL){

              /* load block into images*/
	      if (block->c!=NULL){
		 block_ptr=block->pt;
		 image_ptr=image->pt+image->columns*block->y+block->x;
                 for (i=0;i<block->size;i++){
                     for (j=0;j<block->size;j++)
                         if (i+block->x<image->columns && 
			     j+block->y<image->rows &&
                             i+block->x>=0 && 
			     j+block->y>=0)
                            (*image_ptr++)=(*block_ptr++);
		     image_ptr+=image->columns-block->size;
		 }
	     }

              /* move pointer to next position */
              block=block->next;
        }

}

/* removes all block allocated in coder  */
void free_blocked_image
(IMAGE *image)
{
        BLOCK *block,*next;

        block=image->tree_top;
        while (block!=NULL){
              next=block->next;
	      free_BLOCK(block);
	      /* move to next block*/
	      block=next;
        }

}


/* block image with fixed block size */
int setup_blocks
(IMAGE *image,
 int size)
{
        int i,j,n_blocks;
        BLOCK *block,*last;
         
        last=NULL;
	n_blocks=0;
        for (j=0;j<image->rows;j+=size)  
            for (i=0;i<image->columns;i+=size){
	        n_blocks++;

                /* allocate block memory */
                block=allocate_block_memory(image,size,i,j);

                /* load top of tree into image  struct */
                if (i==0 && j==0)
                   image->tree_top=block;
                else
                    last->next=block; /* form link list if applicable */

                /* load last pointer for next pass */
                last=block;

            }    

	return n_blocks;
}

void setup_cosine_basis
(BASIS *basis,
 int size)
{
        int i,j,k,l,s,cnt;
        BASIS *next,*lower;
        float sum,u;
        int mask[4][4]={{1,1,1,0},
                        {1,1,0,0},
                        {1,0,0,0},
                        {0,0,0,0}};
 
        lower=basis;
        for (s=size;s>=2;s/=2){
 
            /* continue vertical linked list */
            if (s!=size){
               basis=malloc_BASIS(1,"setup cosine basis");
               lower->lower=basis;
               lower=basis;
            } 
              
            cnt=0;                  
            for (l=0;l<4;l++)
                for (k=0;k<4;k++){
                    /* mask unwanted enteries */
                    if (mask[l][k]==0)
                       continue;

                    /* do not form basis larger than block size*/
                    if (k>=s || l>=s)
                       continue;

                    /* continue horizontal linked list */
                    if (!(k==0 && l==0)){
                       next=malloc_BASIS(1,"setup cosine basis");
                       basis->next=next;
                       basis=next;
                    }

                    /* clear foward pointers */
                    basis->next=NULL;
                    basis->lower=NULL;

                    /* setup basis */  
                    basis->size=s;
                    basis->nitems=0; /* only > 0 for root level */
                    basis->pt=malloc_float(s*s,"setup cosine basis pt");
                    basis->min=0;
                    basis->max=0;
 
                    u=2.0/(float)s;
                    /* scaling */
                    if (k==0)
                       u/=sqrt(2.0);
                    if (l==0)
                       u/=sqrt(2.0);

                    /* setup cosine basis */
                    sum=0.0;
                    for (j=0;j<s;j++)
                        for (i=0;i<s;i++){
                            basis->pt[i+s*j]=u*cos(PI*(float)k*((float)i+0.5)/(float)s)
                                             *cos(PI*(float)l*((float)j+0.5)/(float)s);
                            sum+=basis->pt[i+s*j]*basis->pt[i+s*j];
                        }
                
                    basis->sum=sum; /* equal to block size for dct but for others? */
 
                    /* setup quantisation of basis function */
                    basis->q=Q*s; /* simple method */
                
                    /* increament number of basis functions */
                    cnt++;
                }
 
                /* load number of basis function into root pointer*/
                lower->nitems=cnt;
        }
}

void shutdown_cosine_basis
(BASIS *top)
{
	BASIS *basis,*lower,*next;

	/* free memory used by cosine basis */
	basis=top;
	while (basis!=NULL){
	      lower=basis->lower;
	      while (basis!=NULL){
		    next=basis->next;
		    /* free basis function */
		    free_BASIS(basis);
		    /* move along to next basis */
		    basis=next;
	      }  
	      /* move to lower basis function */
	      basis=lower;
	}

}
 
BLOCK *allocate_block_memory
(IMAGE *image,
 int size,
 int x,	
 int y)
{
        BLOCK *pt;
 
	pt=setup_new_block(size,x,y);
	pt->jump=image->columns-size;
        pt->pt=image->pt+(image->columns*y)+x;
 
        return pt;
}

BLOCK *setup_new_block
(int size,
 int x,
 int y)
{
        BLOCK *pt;
        
        /* allocate memory for new block */
        pt=malloc_BLOCK(1,"allocate block mem");
 
        pt->size=size;
        pt->x=x;
        pt->y=y;
        pt->next=NULL;
        pt->c=NULL;
        pt->cf=0;

	return pt;
        
}
 
float calculate_block_error
(BLOCK *block,
 BASIS *basis)
{
 
        int i,j,*ptr;
#ifndef DDCT
        float c,cq;
#endif
        float error;
        
 
        /* first find compatable basis size */
        while (basis->size!=block->size){
              basis=basis->lower;
              if (basis==NULL){
                 printf("Unable to find basis for block\n");
                 exit(-1);
              }
        }
 
        /* first calculate entire error */
        error=0;
	ptr=block->pt;
        for (j=0;j<block->size;j++,ptr+=block->jump)
            for (i=0;i<block->size;i++){
                error+=(float)((*ptr)*(*ptr));
		ptr++;
	    }
              
#ifndef DDCT
        /* calcuate reduction in error caused by each coeff  */
        i=0;
        while (basis!=NULL){
              c=block->c[i++];
              cq=basis->q*rint(c/basis->q);
              error-=(2*cq*c-cq*cq);
              basis=basis->next;
        }     
#endif
 
        return myabs(error);
}

void render_image
(IMAGE *image,
 BASIS *basis)
{
        BLOCK *block;
 
        block=image->tree_top;
        while (block!=NULL){
	      switch (block->size){
		     case(1): block->pt[0]=(int)(block->c[0]+0.5);
			      break;
		     case(2): render2x2m4(block);
			      break;
		     case(4): render4x4m6(block);
			      break;
		     case(8): render8x8m6(block);
			      break;
		     case(16): render16x16m6(block);
			       break;
		     case(32): render32x32m6(block);
			       break;
		     default: render_block(block,basis);
	      }
              block=block->next;
	}

}
 
void render_block
(BLOCK *block,
 BASIS *basis)
{
 
        int i,j,k,*ptr,cnt;

        /* first find compatable basis size */
        while (basis->size!=block->size){
              basis=basis->lower;
              if (basis==NULL){
                 printf("Unable to find basis for block\n");
                 exit(-1);
              }
        }

	/* return if block does not need to be rendered ie no coeff */
	if (block->c==NULL)
	   return;
	
        /* render image from basis function + scaling */
	k=0;
        while (basis!=NULL){
	      cnt=0;
	      ptr=block->pt;
	      for (j=0;j<block->size;j++,ptr+=block->jump)
                  for (i=0;i<block->size;i++)
                      (*ptr++)+=block->c[k]*basis->pt[cnt++];
              k++;
              basis=basis->next;
        }
 
}        
 
float calculate_scaling
(int *pt,
 BASIS *basis)
{
        int i;
        double sum,a;

        /* find sum */
        sum=0;
        for (i=0;i<basis->size*basis->size;i++)
            sum+=(float)(pt[i])*basis->pt[i];

        /* calcuate scaling of basis function */
        if (basis->sum==0.0)
           a=0;
        else
            a=sum/basis->sum;
         
        return (float)a;

}

float calculate_block_scaling  
(BLOCK *block,
 BASIS *basis)   
{
        int i,j,*ptr,k;   
        double sum,a;
 
        /* find sum */
        sum=0;
	ptr=block->pt;
	k=0;
        for (j=0;j<block->size;j++,ptr+=block->jump)
            for (i=0;i<block->size;i++)
                sum+=(float)(*ptr++)*basis->pt[k++];
 
        /* calcuate scaling of basis function */
        if (basis->sum==0.0) 
           a=0;  
        else  
            a=sum/basis->sum;
         
        return (float)a; 
 
}

#ifdef DDCT
/* DDCT block fitting / rendering method */
void fit_basis_to_block
(BLOCK *block,
 BASIS *basis)
{
 
        int i;
 
        /* first find compatable basis size */
        while (basis->size!=block->size){
              basis=basis->lower;
              if (basis==NULL){
                 printf("Unable to find basis for block\n");
                 exit(-1);
              }  
        }
        /* allocate space for coeffcients */
        block->c=malloc_float(basis->nitems,"fit basis to block");
        block->nitems=basis->nitems;
 
        /* calcuate basis scaling / coeffiecients for each basis function */
        switch (block->size){
               case(2): code2x2m4(block);
                        break;
               case(4): code4x4m6(block);
                        break;
               case(8): code8x8m6(block);
                        break;
               case(16): code16x16m6(block);
                        break;
               case(32): code32x32m6(block);
                        break;
               default: i=0;
                        while (basis!=NULL){
                              block->c[i++]=calculate_block_scaling(block,basis);
                              basis=basis->next;
                        }
        }
	
	/* make coefiicents -ve for rendering... remove approx from picture */
	for (i=0;i<block->nitems;i++){ 
	   if(block->c[i]<=-1000*block->size)
		printf("%d\t%f\t%d\t%d\t%d\n",block->size,block->c[i],i,block->x,block->y);
	    block->c[i]=rint(block->c[i]/(float)(Q*block->size));
	    block->c[i]*=-(Q*block->size);
	}
	
	/*render block */
	switch (block->size){
               case(1): block->pt[0]=(int)(block->c[0]+0.5);
                        break;
               case(2): render2x2m4(block);
                        break;
               case(4): render4x4m6(block);
                        break;
               case(8): render8x8m6(block);
                        break;
               case(16): render16x16m6(block);
                         break;
               case(32): render32x32m6(block);
                         break;
               default: render_block(block,basis);
        }  
	
	/* restore coefficents */
	for (i=0;i<block->nitems;i++) 
	    block->c[i]*=-1;
 
}        

#else
/* LQDCT block fitting method */
void fit_basis_to_block
(BLOCK *block,
 BASIS *basis)
{

        int i;

        /* first find compatable basis size */
        while (basis->size!=block->size){
              basis=basis->lower;
              if (basis==NULL){
                 printf("Unable to find basis for block\n");
                 exit(-1);
              }         
        }

        /* allocate space for coeffcients */
        block->c=malloc_float(basis->nitems,"fit basis to block");
        block->nitems=basis->nitems;

        /* calcuate basis scaling / coeffiecients for each basis function */
	switch (block->size){
	       case(2): code2x2m4(block);
		        break;
	       case(4): code4x4m6(block);
		        break;
	       case(8): code8x8m6(block);
			break;
	       case(16): code16x16m6(block);
			break;
	       case(32): code32x32m6(block);
			break;
	       default: i=0;
                        while (basis!=NULL){
                              block->c[i++]=calculate_block_scaling(block,basis);
                              basis=basis->next;
                        }
	}

}         
#endif
