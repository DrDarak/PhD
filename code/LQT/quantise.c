/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                         Quantiser		                */
/*                      reworked 29/8/96                        */
/*##############################################################*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "quantise.h"

/* Internal functions */
void unquantise_block (BLOCK *, BASIS *);

void quantise_image
(IMAGE *image,
 BASIS *basis)
{
       BLOCK *block;
 
        block=image->tree_top;
        while (block!=NULL){
              quantise_block(block,basis);
              block=block->next;
        }
}
 
void quantise_block
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
 
        i=0;
        while (basis!=NULL){
              block->c[i]=rint(block->c[i]/basis->q);
              i++;
              basis=basis->next;
        }
 
}        
 
void unquantise_image
(IMAGE *image,
 BASIS *basis)
{
        BLOCK *block;
 
        block=image->tree_top;
        while (block!=NULL){
              unquantise_block(block,basis);
              block=block->next;
        }
}

void unquantise_block
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
 
        i=0;  
        while (basis!=NULL){
              block->c[i++]*=basis->q;
              basis=basis->next;
        }
}        

