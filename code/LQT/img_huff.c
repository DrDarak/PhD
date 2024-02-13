/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                    IMage Huffman encoder                     */
/*                      reworked 29/8/96                        */
/* 			refined 4/2/97				*/
/*##############################################################*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "mem_hand.h"
#include "huff.h"
#include "img_huff.h"

#ifdef RD_SWITCH
	#include "rd_swtch.h"
#endif

/* macros */
#define myabs(x)    ( x < 0 ? -x : x)

void setup_huffman_coder
(BASIS *top,
 HUFF *qtree,
 HUFF *frac,
 char *pdf_fname)
{
 
        PACKED *dump;
	HUFF *huff;
        BASIS *basis,*lower,*top_basis;
        int nitems;
        FILE *fp;
 
        nitems=top->nitems;
 
        fp=fopen(pdf_fname,"rb");
	NULL_FILE_error(fp,"pdf.all");
 

	/* read qtree huff tree from file */
	dump=malloc_PACKED(1,"setup hufman coder qtree");
        setup_huffman_tree (qtree,dump,fp);
 
	/* setup dump for all coefficents */
	dump=malloc_PACKED(1,"setup hufman coder coeff");
        /* load in huff table for basis functions */
        basis=top;
        while (basis!=NULL){
              /* allocate space for tables */
              huff=malloc_HUFF(1,"setup huffman coder");
 
	      /* read huff tree from file */
	      setup_huffman_tree (huff,dump,fp);

              /* store dump in basis structure */
              basis->huff=huff;
              basis=basis->next;
               
        }

#ifdef FRACTAL
	/* read  fractal huff tree from file */
        setup_huffman_tree (frac,dump,fp);
#endif 

        /* close data file */
        fclose(fp);
 
        /* now place these pointers in the other basis functions */
        lower=top->lower;
        while (lower!=NULL){
              basis=lower;
              top_basis=top;
              while (basis!=NULL){
                    basis->huff=top_basis->huff;
                    basis=basis->next;
                    top_basis=top_basis->next;
              }
              lower=lower->lower;
        }
 
}
void shutdown_huffman_coder
(BASIS *top,
 HUFF *qtree,
 HUFF *frac)
{
        BASIS *basis;

        /* read qtree huff tree from file */
	free_huffman_tree(qtree);
        free_PACKED(qtree->dump); 

	/* free coeff dump once */
        free_PACKED(top->huff->dump); 
        /* load in huff table for basis functions */
        basis=top;
        while (basis!=NULL){
              free_huffman_tree(basis->huff); 
              free_HUFF(basis->huff); 
              basis=basis->next;
        }

#ifdef FRACTAL
        free_huffman_tree(frac); 
#endif 

	return;
}


void huffman_encode_image
(IMAGE *image,
 BASIS *basis,
 HUFF *frac)
{
        BLOCK *block;
 
	for (block=image->tree_top;block;block=block->next)
            huffman_encode_block(block,basis);

#ifdef FRACTAL
	for (block=image->tree_top;block;block=block->next)
	#ifdef RD_SWITCH
	    run_length_code_fractal(frac,block->cf,0,0);
	run_length_code_fractal(frac,0,0,1); /* flush last runlength */
	#else
            compress_symbol(frac,block->cf);
	#endif
#endif 

}
 
int huffman_encode_block
(BLOCK *block,
 BASIS *basis)
{
        int i,bits;
        
        /* first find compatable basis size */
        while (basis->size!=block->size){
              basis=basis->lower;
              if (basis==NULL){
                 printf("Unable to find basis for block\n");
                 exit(-1);
              }
        }
 
        i=0;
        bits=0;
        while (basis!=NULL){
              bits+=compress_symbol((basis->huff),(int)(block->c[i++]));
              basis=basis->next;
        }      
 
        return bits;
        
}
 
int huffman_estimate_block
(BLOCK *block,
 BASIS *basis)
{
        int i,bits;
 
        /* first find compatable basis size */
        while (basis->size!=block->size){
              basis=basis->lower;
              if (basis==NULL){
                 printf("Unable to find basis for block\n");
                 exit(-1);
              }
        }
 
        i=0;
        bits=0;
        while (basis!=NULL){
              bits+=estimate_compress_symbol(basis->huff,block->c[i++]);
              basis=basis->next;
        }
 
        return bits;
 
}

void huffman_decode_image
(IMAGE *image,
 BASIS *basis,
 HUFF *frac)
{
        BLOCK *block;
 
	for (block=image->tree_top;block;block=block->next)
              huffman_decode_block(block,basis);

#ifdef FRACTAL
	for (block=image->tree_top;block;block=block->next)
	#ifdef RD_SWITCH
	    block->cf=run_length_decode_fractal(frac,0);
	#else
            block->cf=uncompress_symbol(frac);
	#endif
#endif 
 
}
 
void huffman_decode_block
(BLOCK *block,
 BASIS *basis)
{
        int i;
 
        /* first find compatable basis size */
        while (basis->size!=block->size){
              basis=basis->lower;
              if (basis==NULL){
                 printf("basis-%p\t%d\n",basis,block->size);
                 printf("Unable to find basis for block (huffman decode)\n");
                 exit(-1);
              }
        }
 
        i=0;
        block->c=malloc_float(basis->nitems,"huffman decode block");
        while (basis!=NULL){
              block->c[i++]=(float)uncompress_symbol(basis->huff);
              basis=basis->next;
        }
 
 
}
 
void save_image_pdf
(BASIS *basis,
 HUFF *qtree,
 HUFF *frac,
 char *pdf_fname)
{

        FILE *fp;

        fp=fopen(pdf_fname,"wb");
	NULL_FILE_error(fp,"pdf.all");

        /* save qtree pdf */
        save_huffman_pdf(qtree,fp);

        /* save dump pdf for each basis function */
        while (basis!=NULL){
              save_huffman_pdf(basis->huff,fp);
              basis=basis->next;
        }

#ifdef FRACTAL
        save_huffman_pdf(frac,fp);
#endif 

        fclose(fp);

}                   

void wipe_image_pdf
(BASIS *basis,
 HUFF *qtree,
 HUFF *frac,
 char *pdf_fname)
{

        FILE *fp;

        fp=fopen(pdf_fname,"wb");
	NULL_FILE_error(fp,"pdf.all");
 
        /* wipe qtree pdfs */
        wipe_huffman_pdf(qtree,fp);
 
        while (basis!=NULL){
              wipe_huffman_pdf(basis->huff,fp);
              basis=basis->next;
        }

#ifdef FRACTAL
        wipe_huffman_pdf(frac,fp);
#endif 
 
        fclose(fp);
 
}

