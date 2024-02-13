/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                   	 quadtree                               */
/*                      reworked 29/8/96                        */
/*		      Binary Implementation oct/Nov 96		*/
/*##############################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mem_hand.h"
#include "blocks.h"
#include "huff.h"
#include "quad.h"

/* Internal Functions */
void split_blocks (BLOCK *);
void append_block_to_list (BLOCK *, BLOCK *);
BLOCK *encode_qtree_block (PACKED *, BLOCK *, int);
BLOCK *decode_qtree_block (PACKED *, BLOCK *, int ,int ,int);
void convert_block_to_binary (BLOCK *);
void convert_index_to_block (BLOCK *);

void form_quad_tree
(IMAGE *image)
{
        BLOCK *block;
        int i,sum;
 
        /* now quadtree image */
        block=image->tree_top;
        while (block!=NULL){
	       /* sum pixels in block to find type */
	       sum=0;
	       for (i=0;i<block->size*block->size;i++)
		   sum+=block->pt[i];
	
	       /* deside type black,white,vq, split */
	       if (sum==0){
		  block->type=black;
 		  block=block->next;
		  continue;
	       }

	       if (sum==block->size*block->size){
		  block->type=white;
 		  block=block->next;
		  continue;
	       }

	       if (block->size==MIN_SIZE){
		  block->type=vq;
		  convert_block_to_binary(block);
 		  block=block->next;
		  continue;
	       }

	       if (block->size<MIN_SIZE){
		  printf("Block overflow in form qtree\n");
		  exit(-1);
               } 
 
               /* split active  blocks into 4 quads */
               split_blocks(block);
        }
 
}

/* quadtree spliting function */
void split_blocks
(BLOCK *parent)
{
        int i,j,k,size;
        int *pt;
        BLOCK *child,*last;
 
 
        /* determine child size */
        size=parent->size/2;
 
        /* now place children into block structures*/
 
        /* Top Right */
        child=allocate_block_memory(size,parent->x+size,parent->y);
        /* load up pixel arrys from parent to children */
        k=0;
        for (j=0;j<size;j++)
            for (i=size;i<parent->size;i++)
                child->pt[k++]=parent->pt[j*parent->size+i];
        append_block_to_list(parent,child);
        last=child;
 
        /* Bottom left */
        child=allocate_block_memory(size,parent->x,parent->y+size);
        /* load up pixel arrys from parent to children */
        k=0;
        for (j=size;j<parent->size;j++)
            for (i=0;i<size;i++)
                child->pt[k++]=parent->pt[j*parent->size+i];
        append_block_to_list(last,child);
        last=child;
 
        /* Bottom Right */
        child=allocate_block_memory(size,parent->x+size,parent->y+size);
        /* load up pixel arrys from parent to children */
        k=0;
        for (j=size;j<parent->size;j++)
            for (i=size;i<parent->size;i++)
                child->pt[k++]=parent->pt[j*parent->size+i];
        append_block_to_list(last,child);
 
        /* Top left */
        pt=parent->pt;
        parent->pt=malloc_int(size*size,"quad tree split");
        k=0;
        for (j=0;j<size;j++)
            for (i=0;i<size;i++)
                parent->pt[k++]=pt[j*parent->size+i];
        parent->size=size;
        free_int(pt);
        
}

void append_block_to_list
(BLOCK *first,
 BLOCK *second)
{
        BLOCK *next;
 
        /* place append block after block block */
        next=first->next;
        first->next=second;
        second->next=next;
 
}

void convert_block_to_binary
(BLOCK *block)
{

	int i,mask,index;

	/* set bit mask to 1 */
	mask=1;
	index=0;

	for (i=0;i<block->size*block->size;i++){
	    if (block->pt[i])
	       index|=mask;
	    mask<<=1;
	}

	/* save block index */
	block->index=index;

}

void convert_index_to_block
(BLOCK *block)
{

	int i,mask;

        /* set bit mask to 1 */
        mask=1;

        for (i=0;i<block->size*block->size;i++){
	    if (mask&block->index)
               block->pt[i]=1;
	    else
		block->pt[i]=0;
	    mask<<=1;
        }

} 

void encode_qtree_image
(IMAGE *image,
 PACKED *qtree,
 int size)
{
        BLOCK *block;
        
        /* encode qtree */
        block=image->tree_top;
        while (block!=NULL){
 
              if (block->size==size){
		 /* black white or VQ */
		 if (block->type==black)
                    compress_symbol(qtree,-1);
                 if (block->type==white)
                    compress_symbol(qtree,-2);

		 /* error if not */
		 if (block->type!=black && 
		     block->type!=white){
		    printf ("error in encode qtree - unknown symbol\n");
		    exit(-1);
		 }
                 block=block->next;
              }
              else {
                  compress_symbol(qtree,-3);
                  block=encode_qtree_block(qtree,block,size/2);
              }
        }      
 
 
}
 
BLOCK *encode_qtree_block
(PACKED *qtree,
 BLOCK *block,
 int size)
{
        int i;
 
        for (i=0;i<4;i++)
            if (block->size<size){
               compress_symbol(qtree,-3);
               block=encode_qtree_block(qtree,block,size/2);
            }
            else {
		/* black white or VQ */
		if (block->type==black)
                   compress_symbol(qtree,-1);
		if (block->type==white)
                   compress_symbol(qtree,-2);
		if (block->type==vq)
                   compress_symbol(qtree,block->index);

		/* error if not */
		if (block->type!=black && 
		    block->type!=white &&
		    block->type!=vq){
		   printf ("error in encode qtree - unknown symbol\n");
		   exit(-1);
		}
                block=block->next;
		
            }
              
        return block;
 
}

/* decodes size rows of image */
void decode_qtree_image
(IMAGE *image,
 PACKED *qtree,
 int size)
{
        int x,y,i;
	int index;
        BLOCK *block,*top;
 
 
        /* decode quad tree */
        /* allocate duff block */
        top=allocate_block_memory(0,0,0);
        block=top;
 
        x=0;
        y=0;
 
        for (;;) {
        
	    index=uncompress_symbol(qtree);
            if (index==-3){
               block=decode_qtree_block(qtree,block,x,y,size/2);
            } 
	    if (index>-3){
               block->next=allocate_block_memory(size,x,y);
               block=block->next;
	       if (index==-1){
		  block->type=black;
		  for (i=0;i<block->size*block->size;i++)
		      block->pt[i]=0;
	       }
	       if (index==-2){
		  block->type=white;
		  for (i=0;i<block->size*block->size;i++)
		      block->pt[i]=1;
	       }
	    }
	    if (index>-1 || index<-3){
	       printf("error on top level of decode\n");
	       exit(-1);
	    }
               
            /* increament position in image */
            if (x+size<image->columns)
               x+=size;
            else {
		 /* break after 1 row */
	         break;
            }
               
        }
 
        image->tree_top=top->next;
 
        /* clear up */
        free_BLOCK(top);
}

BLOCK *decode_qtree_block
(PACKED *qtree,
 BLOCK *block,
 int x,
 int y,
 int size)
{
        int i,j,k;
	int index;
 
        for (j=0;j<size*2;j+=size)
            for (i=0;i<size*2;i+=size){
		index=uncompress_symbol(qtree);
                if (index==-3){
                   block=decode_qtree_block(qtree,block,x+i,y+j,size/2);
                }
	        if (index>-3){
                   block->next=allocate_block_memory(size,x+i,y+j);
                   block=block->next;
                   if (index==-1){
                      block->type=black;
                      for (k=0;k<block->size*block->size;k++)
                      block->pt[k]=0;
                   }
                   if (index==-2){
                      block->type=white;
                      for (k=0;k<block->size*block->size;k++)
                          block->pt[k]=1;
                   }
		   if (index>=0){
                      block->type=vq; 
		      block->index=index;
		      convert_index_to_block(block);
                   }

                }   
                if (index<-3){
                   printf("error in lower level of qtree decode\n");
                   exit(-1); 
                } 
            }    
 
        return block;
 
}



