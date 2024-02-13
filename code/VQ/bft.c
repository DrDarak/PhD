/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*    	   	     BATH FRACTAL TRANSFORM 			*/
/*                      reworked 29/8/96                        */
/*			tuned 18/2/97				*/
/*##############################################################*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "mem_hand.h"
#include "blocks.h"
#include "frac.h"

/* prototypes */
float fit_fractal_to_block (IMAGE *, BLOCK *, BASIS *);
float fit_block_to_parent (BLOCK *, BASIS *); 
void orthogonalise_block (BASIS *, BASIS *);
 void shrink_parent_onto_child (IMAGE *, BASIS *);
BASIS find_parent_location (IMAGE *, BLOCK *, BASIS *);
 
void fit_fractal_to_image
(IMAGE *image,
 BASIS *basis)
{
	BLOCK *block;
	
	for (block=image->tree_top;block!=NULL;block=block->next){
	    fit_fractal_to_block(image,block,basis);
	}

}

float fit_fractal_to_block
(IMAGE *image,
 BLOCK *block,
 BASIS *basis)
{
        BASIS parent;
        float error;

	if (block->size<=2){
	   block->cf=0;
	   return 0.0; /* 2x2 dct ~ lossless */
	}

        /* find centred parent block */
        parent=find_parent_location(image,block,basis);

        /* take centred parent and shrink onto child */
        parent.size=block->size;
        shrink_parent_onto_child(image,&parent);

        /* orthogonalise block */
        orthogonalise_block(basis,&parent);

        error=fit_block_to_parent(block,&parent);

        /* claer up */
        free_float(parent.pt);

        return error;
}

float fit_block_to_parent
(BLOCK *block,
 BASIS *parent)
{
        float c,cq,error;
 
        /* find scaleing of fractal */
        c=calculate_block_scaling(block,parent);
 
        /* quantise cf */
        cq=rint(c/(float)(Q*block->size));
 
        /* save quantised fractal coeff */
        block->cf=(int)cq;
 
        /* calcuate error */
        cq*=(float)(Q*block->size);
        error=2*cq*c-cq*cq;
 
        return error;
 
}

void orthogonalise_block
(BASIS *top,
 BASIS *parent)
{
        int i;
        float c,sum;
        int *pt;
        BASIS *basis;
 
        /* find compatable basis size */
        while (top->size!=parent->size){
              top=top->lower;
              if (top==NULL){
                 printf("Unable to find basis for block %d %d\n",top->size,parent->size);
                 exit(-1);
              }
        }       
 
        /* malloc tmp store for parent */
        pt=malloc_int(parent->size*parent->size,"orthogonalise block");
        /* make sure parent and ponter are same */
        for (i=0;i<parent->size*parent->size;i++){
            pt[i]=(int)rint(parent->pt[i]);
            parent->pt[i]=(float)pt[i];
        }
 
        /* Gramm Smitt orthogonalisation */
        basis=top;
        while (basis!=NULL){
              c=calculate_scaling(pt,basis);
              for (i=0;i<basis->size*basis->size;i++)
                  parent->pt[i]-=c*basis->pt[i];
              basis=basis->next;
        }       
 
        /* free tmp store of parent */
        free_int(pt);
 
        sum=0;
        for (i=0;i<top->size*top->size;i++)
            sum+=parent->pt[i]*parent->pt[i];
 
        /*
        for (i=0;i<top->size*top->size;i++)
            if (sum==0)
               parent->pt[i]=0;
            else
                parent->pt[i]/=sqrt(sum);
        */
 
        parent->sum=sqrt(sum);
         
}

void shrink_parent_onto_child
(IMAGE *image,
 BASIS *parent)
{
        int i,j,k,l,cnt;
        int sum;
        float *pt; 

        /* allocate mem for shrunk parent */
        pt=malloc_float(parent->size*parent->size,"shrink parent onto child");     

        cnt=0;
        for (j=0;j<2*parent->size;j+=2)
            for (i=0;i<2*parent->size;i+=2){
                sum=0;
                for (l=0;l<2;l++)
                    for (k=0;k<2;k++)
                        sum+=image->pt[(j+l+parent->y)*image->columns 
                                       +i+l+parent->x];
                /* MUST be integer average ??? dies otherwise*/
                pt[cnt++]=(float)(sum/4);        
            }

        parent->pt=pt;

}



BASIS find_parent_location
(IMAGE *image, 
 BLOCK *block,
 BASIS *basis)
{
	BASIS parent;
 
        /* only search if blcok size is more than 2x2 */
        /* Find co-ord of corner of the parent */
        parent.x=block->x-(block->size/2);
        if (parent.x<0)
           parent.x=0;
        if (parent.x+block->size*2>=image->columns)
           parent.x=image->columns-block->size*2;

        parent.y=block->y-(block->size/2);
        if (parent.y<0)
           parent.y=0;
        if (parent.y+block->size*2>=image->rows)
           parent.y=image->rows-block->size*2;
        return parent;

}

void render_fractal
(IMAGE *image,
 BASIS *basis)
{
 
        int i,j,k,pos;
        BLOCK *block;
        BASIS parent;
        float cf;
        int *tmp;
 
        /* load image generated by basis functions in tmp */
        tmp=malloc_int(image->columns*image->rows,"render fractal");

        for (i=0;i<image->rows*image->columns;i++)
            tmp[i]=image->pt[i];
              
        /* more than one pass of rendering cause problems */
        for (k=0;k<3;k++){
            block=image->tree_top;
            while (block!=NULL){
                  /* skip if there is no fractal in the block */
                  if (block->cf==0){
                     block=block->next;
                     continue;
                  }
                  /* find parent position */
                  parent=find_parent_location(image,block,basis);
        	  parent.size=block->size;
 
                  /* unquantise cf */
                  cf=Q*(float)(block->size*block->cf);
 
	          /* shrink down approx parent */
                  shrink_parent_onto_child(image,&parent);

                  /* orthogonalise approx parent */
                  orthogonalise_block(basis,&parent);

		  /* render fractal onto image */ 
                  for (j=0;j<block->size;j++)
                      for (i=0;i<block->size;i++){
                          pos=(block->y+j)*image->columns+block->x+i;
                          if (i+block->x<image->columns && j+block->y<image->rows)
                             image->pt[pos]=(int)rint(cf*
                                    parent.pt[j*block->size+i]/parent.sum)+tmp[pos];
                      }

        
                  free_float(parent.pt);
                  block=block->next;
            }
        }     
        free_int(tmp);
}

