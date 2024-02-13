/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                   	wavelet blocks 	                        */
/*                      reworked 3/7/96                         */
/*##############################################################*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "blocks.h"

void setup_blocks
(IMAGE *image,
 int scales)
{

	int i,j,size;
	int x_size,y_size;
	BLOCK *block,*last;

	size=(1<<scales)*(1<<scales);
	x_size=image->columns>>scales;
	y_size=image->rows>>(scales);

	/* break wavelet into blocks */
	last=NULL; 
	for (j=0;j<y_size;j++)
            for (i=0;i<x_size;i++){
		/* allocate blocks */
	        block=malloc_BLOCK(1,"setup blocks ");
	        block->pt=malloc_int(size,"setup blocks wavelet block");
		block->x=i;
		block->y=j;
		block->size=size;

 	        /* load top of tree into image  struct */
                if (i==0 && j==0)
                   image->tree_top=block;
                else
                    last->next=block; /* form link list if applicable */

		/* load last pointer for next pass */
                last=block;
	}

	block=image->tree_top;
	while (block!=NULL){
	      tree_block (image,block,scales);
	      block=block->next;
	}

}

void tree_block
(IMAGE *image,
 BLOCK *block,
 int scales)
{

 	int size,size2;
        int size_x,size_y,i,j,offset,k;
        int x_size,y_size;

	x_size=image->columns>>scales;
	y_size=image->rows>>scales;

	size=1;
        size2=1;
        offset=1;
        size_x=x_size;
        size_y=y_size;
        block->pt[0]=image->pt[image->columns*block->y +block->x];
        for (k=0;k<scales;k++){
            for (j=0;j<size;j++)
                for (i=0;i<size;i++){
                    block->pt[offset+j*size+i]=
                             image->pt[image->columns*(size*block->y+size_y+j)
                                       +block->x*size+i];
                    block->pt[offset+size2+j*size+i]=
                             image->pt[image->columns*(size*block->y+size_y+j)
                                       +block->x*size+size_x+i];
                    block->pt[offset+size2*2+j*size+i]=
                             image->pt[image->columns*(size*block->y+j)
                                       +block->x*size+size_x+i];
                }
           offset+=3*size2;
           size*=2;
           size2*=4;
           size_x*=2;
           size_y*=2;

        }

}

void reform_image
(IMAGE *image,
 int scales)
{

	BLOCK *block;

        block=image->tree_top;
        while (block!=NULL){
	      untree_block (image,block,scales);
              block=block->next;
        }      
}

/* generic untree */
void untree_block 
(IMAGE *image,
 BLOCK *block,
 int scales)
{

	int size,size2;
        int size_x,size_y,i,j,offset,k;
        int x_size,y_size;

	x_size=image->columns>>scales;
        y_size=image->rows>>scales;

	size=1;
        size2=1;
        offset=1;
        size_x=x_size;
        size_y=y_size;
        image->pt[image->columns*block->y+block->x]=block->pt[0];
        for (k=0;k<scales;k++){
            for (j=0;j<size;j++)
                for (i=0;i<size;i++){
                    image->pt[image->columns*(size*block->y+size_y+j)+block->x*size+i]=
                             block->pt[offset+j*size+i];
                    image->pt[image->columns*(size*block->y+size_y+j)+block->x*size+size_x+i]=
                             block->pt[offset+size2+j*size+i];
                    image->pt[image->columns*(size*block->y+j)+block->x*size+size_x+i]=
                             block->pt[offset+size2*2+j*size+i];
                }
           offset+=3*size2;
           size*=2;
           size2*=4;
           size_x*=2;
           size_y*=2;

        }             

}

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

