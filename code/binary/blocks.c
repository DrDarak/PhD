/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                    Block handleing                           */
/*                      reworked 28/8/96                        */
/*		      Modified for binary Oct/Nov 96		*/
/*##############################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mem_hand.h"
#include "blocks.h"

/* defines */
#define PI 3.142

/* macros */
#define myabs(x)    ( x < 0 ? -x : x)

/* put blocks back ito an image for writing  */
void reform_image
(IMAGE *image)
{
        int i,j;
        BLOCK *block;
         
        /* allocate memory for image */
        if (image->pt!=NULL)
           free_unsigned_char(image->pt);
        image->pt=malloc_unsigned_char(image->columns*image->rows,"reform image");

        block=image->tree_top;
        while (block!=NULL){

              /* load block into images*/
              for (i=0;i<block->size;i++)
                  for (j=0;j<block->size;j++)
                      if (i+block->x<image->columns && 
			  j+block->y<image->rows &&
                          i+block->x>=0 && 
			  j+block->y>=0)
                         image->pt[image->columns*(j+block->y)+i+block->x]=
                                                    block->pt[block->size*j+i];

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
        int i,j,ii,jj,tmp,flag,avg,cnt,n_blocks;
        BLOCK *block,*last;
         
        last=NULL;
	n_blocks=0;
	j=0;
        for (i=0;i<image->columns;i+=size){
	    n_blocks++;

            /* allocate block memory */
            block=allocate_block_memory(size,i,j);

            /* load image into block */
            flag=0; avg=0; cnt=0;
            for (ii=0;ii<size;ii++)
                for (jj=0;jj<size;jj++){
                    tmp=image->columns*(j+jj)+i+ii;
                    if ((ii+i)>=image->columns || (jj+j)>=image->rows){
                       block->pt[size*jj+ii]=-1;
                       flag=1;
                    }
                    else {
                        block->pt[size*jj+ii]=image->pt[tmp];
                        avg+=image->pt[tmp];
                        cnt++;
                    }
                }
            /* if out of image replace with average of block */
            if (flag) 
               for (ii=0;ii<size;ii++)
                   for (jj=0;jj<size;jj++)
                       if (block->pt[size*jj+ii]==-1)
                          block->pt[size*jj+ii]=avg/cnt;

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

BLOCK *allocate_block_memory
(int size,
 int x,	
 int y)
{
        BLOCK *pt;
 
	pt=setup_new_block(size,x,y);
        /* allocate mem for section of image inside the block */
        pt->pt=malloc_int(size*size,"allocate block mem");
 
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
	pt->index=-1;/* no vq block used to start with */
        pt->next=NULL;
	pt->pt=NULL;
	pt->type=none;

	return pt;
        
}
 
