/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                   	 quadtree                               */
/*                      reworked 29/8/96                        */
/*##############################################################*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "mem_hand.h"
#include "blocks.h"
#include "file_io.h"
#include "quantise.h"
#include "huff.h"
#include "img_huff.h"
#include "quad.h"

/* Internal Functions */
void split_blocks (IMAGE *,BLOCK *);
void append_block_to_list (BLOCK *, BLOCK *);
BLOCK *encode_qtree_block (HUFF *, BLOCK *, int);
BLOCK *decode_qtree_block (HUFF *, BLOCK *, int ,int ,int);

float fit_basis_to_image
(IMAGE *image,
 BASIS *basis,
 int budget)
{
        BLOCK *block,*best,*old;
        float error;
        int bits,i;
#ifdef RD_SWITCH
        float error_rec[32],rate;
        int bits_rec[32],cnt=0;
#endif

        /* code orginal tree of image */
        error=0;
        bits=0;
        block=image->tree_top;
        while (block!=NULL){
              fit_basis_to_block(block,basis);
              block->error=calculate_block_error(block,basis);
              quantise_block(block,basis);
              error+=block->error;
              bits+=huffman_estimate_block(block,basis);
              bits++;
              block=block->next;
        }
        /* now quadtree image */
        while (bits<budget){
 
              block=image->tree_top;
              best=block;
              while (block!=NULL){
                    if (block->error>best->error && block->size>2)
                       best=block;
                    block=block->next;
              }

#ifndef DDCT
              /* remove bits for oold block */
              bits-=huffman_estimate_block(best,basis);
              error-=best->error;
#endif
              
#ifdef RD_SWITCH
              /* store image stats */
              error_rec[cnt&0x1F]=error;
              bits_rec[cnt&0x1F]=bits;
              cnt++;
#endif

              /* store pointer that will be split */
              old=best;
 
              /* split best block into 4 quads */
              split_blocks(image,best);
 
#ifdef DDCT
	      best=best->next; /* move to active blocks */
#endif

              /* code the four blocks */
              for (i=0;i<4;i++){
                  fit_basis_to_block(best,basis);
                  best->error=calculate_block_error(best,basis);
                  error+=best->error;
                  quantise_block(best,basis);
                  bits+=huffman_estimate_block(best,basis);
                  bits++;
                  best=best->next;
              }
           
        }

#ifdef RD_SWITCH
        rate=error_rec[(cnt-31)&0x1F]-error_rec[(cnt-1)&0x1F];
        rate/=(float)(bits_rec[(cnt-1)&0x1F]-bits_rec[(cnt-31)&0x1F]);
 
        return rate;
#else
        return 0;
#endif

}

#ifdef DDCT
/* DDCT quadtree spliting function */
void split_blocks
(IMAGE *image,
 BLOCK *parent)
{
        int size;
        BLOCK *child,*last;

                            
        /* determine child size */
        size=parent->size/2;

	parent->error=0; /* effective removes parent fromm search list*/
        /* now place children into block structures*/

        /* Top left */
	child=allocate_block_memory(image,size,parent->x,parent->y); 
        append_block_to_list(parent,child); 
        last=child;

        /* Top Right */                               
        child=allocate_block_memory(image,size,parent->x+size,parent->y);
        append_block_to_list(last,child);
        last=child;

        /* Bottom left */
        child=allocate_block_memory(image,size,parent->x,parent->y+size);
        append_block_to_list(last,child);
        last=child;

        /* Bottom Right */
        child=allocate_block_memory(image,size,parent->x+size,parent->y+size);
        append_block_to_list(last,child);
 

}

#else
/* LQDCT quadtree spliting function */
void split_blocks
(IMAGE *image,
 BLOCK *parent)
{
        int size;
        BLOCK *child,*last;
 
 
        /* determine child size */
        size=parent->size/2;
 
        /* now place children into block structures*/
 
        /* Top Right */
        child=allocate_block_memory(image,size,parent->x+size,parent->y);
        append_block_to_list(parent,child);
        last=child;
 
        /* Bottom left */
        child=allocate_block_memory(image,size,parent->x,parent->y+size);
        append_block_to_list(last,child);
        last=child;
 
        /* Bottom Right */
        child=allocate_block_memory(image,size,parent->x+size,parent->y+size);
        append_block_to_list(last,child);
 
        /* Top left */
        free_float(parent->c);
        parent->jump=image->columns-size;
        parent->size=size;
        
}
#endif

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

#ifdef DDCT
/* DDCT has verty simple qtree encoder */
void encode_qtree_image
(IMAGE *image,
 HUFF *qtree,
 int size)
{
        int nitems;
        BLOCK *block,*next;

        nitems=0;
        block=image->tree_top;
        while (block!=NULL){
              nitems++;
              block=block->next;
        }

        /* allocate qtree stream */
        allocate_dump(qtree->dump,nitems*2);
 
        /* encode qtree */
        block=image->tree_top;
	next=block->next;
        while (next!=NULL){
              if (next->x<block->x+block->size &&
                  next->y<block->y+block->size ){
                 compress_symbol(qtree,1);
              }  
	      else
                  compress_symbol(qtree,0);
	      /* move to next block */
	      block=next;
	      next=block->next;
        }

        /* last symbol is always no split(0) */
        compress_symbol(qtree,0);
	
}

#else
void encode_qtree_image
(IMAGE *image,
 HUFF *qtree,
 int size)
{
        int nitems;
        BLOCK *block;
        
        nitems=0;
        block=image->tree_top;
        while (block!=NULL){
              nitems++;
              block=block->next;
        }
 
        /* allocate qtree stream */
        allocate_dump(qtree->dump,nitems*2);
 
        /* encode qtree */
        block=image->tree_top;
        while (block!=NULL){
 
              if (block->size==size){
                 compress_symbol(qtree,0);
                 block=block->next;
              }
              else {
                  compress_symbol(qtree,1);
                  block=encode_qtree_block(qtree,block,size/2);
              }
        }      
 
 
}
 
BLOCK *encode_qtree_block
(HUFF *qtree,
 BLOCK *block,
 int size)
{
        int i;
 
        for (i=0;i<4;i++)
            if (block->size<size){
               compress_symbol(qtree,1);
               block=encode_qtree_block(qtree,block,size/2);
            }
            else {
                compress_symbol(qtree,0);
                block=block->next;
           }
              
        return block;
 
}
#endif

/* Qtre decoders... For DDCT there is very little 
   difference except that all blocks are decoded */
void decode_qtree_image
(IMAGE *image,
 HUFF *qtree,
 int size)
{
        int x,y;
        BLOCK *block,*top;
 
 
	/* allocate dummy block */
        top=setup_new_block(0,0,0);
        block=top;
 
        x=0;
        y=0;
 
        for (;;) {
        
            if (uncompress_symbol(qtree)){
#ifdef DDCT
               block->next=setup_new_block(size,x,y);
               block=block->next;
#endif
               block=decode_qtree_block(qtree,block,x,y,size/2);
            } 
            else {
                 block->next=setup_new_block(size,x,y);
                 block=block->next;
            }
               
            /* increament position in image */
            if (x+size<image->columns)
               x+=size;
            else {
                 x=0;
                 y+=size;
            }
               
            if (y>=image->rows){
               block->next=NULL;
               break;
            }
              
        }
 
        image->tree_top=top->next;
 
	/* clear up */
        free_BLOCK(top);
}

BLOCK *decode_qtree_block
(HUFF *qtree,
 BLOCK *block,
 int x,
 int y,
 int size)
{
        int i,j;
 
        for (j=0;j<size*2;j+=size)
            for (i=0;i<size*2;i+=size)
                if (uncompress_symbol(qtree)){
#ifdef DDCT
                   block->next=setup_new_block(size,x+i,y+j);
                   block=block->next;
#endif
                   block=decode_qtree_block(qtree,block,x+i,y+j,size/2);
                }
                else {
                     block->next=setup_new_block(size,x+i,y+j);
                     block=block->next;
                }
 
        return block;
 
}



