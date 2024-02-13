/*##############################################################*/
/*			David Bethel				*/
/*		       Bath University 				*/
/*		      daveb@ee.bath.ac.uk			*/
/*		   	 LQDCT Coder				*/
/*		  	   25/10/96				*/
/*##############################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "mem_hand.h"
#include "blocks.h"
#include "img_huff.h"
#include "quad.h"
#include "file_io.h"
#include "image_io.h"

#ifdef FRACTAL
	#include "frac.h"
	#ifdef RD_SWITCH
		#include "rd_swtch.h"
	#endif
#endif


/* internal prototypes */
void compress_image (IMAGE *, FILE *, BASIS *,HUFF *, HUFF *, int , int );
typedef enum huff_type {normal,wipe,append} HUFF_TYPE; 
float count_fractals(IMAGE *image);

float global_rate=0.0;


void compress_image
(IMAGE *image,
 FILE *fp,
 BASIS *basis,
 HUFF *qtree,
 HUFF *frac,
 int size,
 int bits)
{
	int n_blocks;
	float rate;

	printf("Setting up blocks\n");
        n_blocks=setup_blocks(image,size);

	printf("Allocating packed streams\n");
        allocate_packed_streams(basis,frac,n_blocks,bits);

	printf("Fitting basis to image\n");
        rate=fit_basis_to_image(image,basis,bits);
	printf("Gradient of RD graph= %f\n",rate);
	// don cludge
	if (global_rate!=0.0)
	   rate=global_rate;
	printf("Gradient supplied to RD switch= %f\n",rate);
#ifdef FRACTAL
	printf("fractal enchancement\n");
	fit_fractal_to_image(image,basis);
	#ifdef RD_SWITCH
		/* swich out fractal with rd less than rate */
		switch_fractals (image,frac,rate);
		/* initialise fractal runlength coder */
	        run_length_code_fractal (0,0,1,0); 
	#endif
#endif

	printf("fractal Area=%f percent",count_fractals(image));
	printf("huffman encode image\n");
        huffman_encode_image(image,basis,frac);

	printf("Huffman encode qtree\n");
        encode_qtree_image(image,qtree,size);
	
	printf("write compressed file\n");
        write_compressed_file(fp,image,basis,qtree,size);

	printf("clearup\n");
        /* clear rubbish */
	free_blocked_image(image);
        free_unsigned_char(qtree->dump->symbols_top);
        qtree->dump->symbols_top=NULL;
        free_unsigned_char(basis->huff->dump->symbols_top);
	basis->huff->dump->symbols_top=NULL;

}

float count_fractals(IMAGE *image)
{

	BLOCK *block;
	int area=0;
	float percent;

	for (block=image->tree_top;block;block=block->next)
	    if (block->cf)
	       area+=block->size*block->size;

	percent=(float)area*100.0/(float)(image->rows*image->columns);

	return percent;

}



void main
(int argc,
 char *argv[])
{
	int size,colour,last,i;
	int columns,rows;
        int n_bits_y,n_bits_u=0,n_bits_v=0;
        IMAGE *image_y,*image_u=NULL,*image_v=NULL;
        BASIS *basis;
        HUFF *qtree,*frac;
	char input_fname[256],output_fname[256],pdf_fname[256];
	float bpp;
        FILE *fp;
	HUFF_TYPE type;

	/* set defaults */
        sprintf(output_fname,"new" FILE_SUFFIX);
        sprintf(pdf_fname,"pdf.all");
        last=0;
	type=normal;

        /* check arguments */
        for (i=1;i<argc;i++){

            /* outpur file option */
            if (!strcmp(argv[i],"-o")){
               i++;
               if (argv[i]==NULL){
                  printf("NULL output argument\n");
                  exit(-1);
               } 
               sprintf(output_fname,"%s",argv[i]);
               /*printf("-o: %s\n",output_fname);*/
               last=i;
               continue;
            }

            /* rate*/
            if (!strcmp(argv[i],"-r")){
               i++;
               if (argv[i]==NULL){
                  printf("NULL rate argument\n");
                  exit(-1);
               }
               global_rate=atof(argv[i]);
               last=i;
               continue;
            }    


	    /*  append option */
            if (!strcmp(argv[i],"-a")){
	       type=append;
               last=i;
               continue;
            }

	    /* wipe pdf file option */
            if (!strcmp(argv[i],"-w")){
	       type=wipe;
               last=i;
               continue;
            }
 
            /* pdf  file option */
            if (!strcmp(argv[i],"-p")){
               i++;
	       if (argv[i]==NULL){
                  printf("NULL output argument\n");
                  exit(-1);
               } 
               sprintf(pdf_fname,"%s",argv[i]);
               /*printf("-w: %s\n",wavelet_fname);*/
               last=i;
               continue;
            }
 
            /* unknown option */
            if (!strncmp(argv[i],"-",1)){
               printf("Unknown option\n");
               exit(-1);
            }
        }
 
        /* warn if wrong input length */
        if (last+2>=argc){
           printf("%s:
        [-a ] appends huffman pdf
        [-w ] wipe huffman pdf
        [-r ] gradient of rate distortion graph
        [-o <output>]   default: new.lqt
        [-p <pdf file>] default: pdf.all
        <bpp>   - compression
        <input> - pgm/ppm file\n",argv[0]);
           exit(-1);
        }
 
        /* load last 2 items */
        bpp=atof(argv[last+1]);
        sprintf(input_fname,"%s",argv[last+2]);

        /* read header */                       
        fp=read_raw_header(input_fname,&rows,&columns,&colour);
 
        /* make space for images */
        image_y=malloc_IMAGE(1,"main y");
        if (colour){
           image_u=malloc_IMAGE(1,"main u");
           image_v=malloc_IMAGE(1,"main v");
        }
	
	/* sets up image sizes */
        image_y->rows=rows;
        image_y->columns=columns;
 
        if (colour)
           get_ppm_file(fp,image_y,image_u,image_v);
        else
            get_pgm_file(fp,image_y);
 
        fclose(fp);
 
        /* image size limitation */
        if (image_y->columns<32 || image_y->rows<32){
           printf("Image too small to compress\n");
           exit(-1);
        }

	/* setup  basis structures */
        basis=malloc_BASIS(1,"main");
        size=32;
        setup_cosine_basis(basis,size);

	/* setup huffman coders */
        qtree=malloc_HUFF(1,"main qtree");
        frac=malloc_HUFF(1,"main frac");
        setup_huffman_coder(basis,qtree,frac,pdf_fname);

        /* huffman file options */             
        if (type==wipe){
           wipe_image_pdf(basis,qtree,frac,pdf_fname);
           printf("wipeing huffman pdf file\n");
           exit(-1);
        }

        /* work out bits availible */
        n_bits_y=image_y->rows*image_y->columns*bpp;
 
        /* set colour compression strengths */
        if (colour){
           n_bits_u=(int)(0.075*(float)n_bits_y); /* 15% colour */
           n_bits_v=(int)(0.075*(float)n_bits_y);
           n_bits_y-=(n_bits_u+n_bits_v);        /* keep totals bits same */
        }

	/* compress image */
	fp=write_compressed_file_header (output_fname,image_y->columns,image_y->rows,colour);
	compress_image(image_y,fp,basis,qtree,frac,size,n_bits_y);
        if (colour){
	   compress_image(image_u,fp,basis,qtree,frac,size,n_bits_u);
	   compress_image(image_v,fp,basis,qtree,frac,size,n_bits_v);
        }
        fclose(fp);

	/* huffman file options */ 
        if (type==append){
           save_image_pdf(basis,qtree,frac,pdf_fname); 
           printf("appending huffman pdf to disk\n"); 
        } 

	/* free memory used by coder */
	shutdown_huffman_coder(basis,qtree,frac);
	shutdown_cosine_basis(basis);
	free_HUFF(qtree);
	free_HUFF(frac);
	free_IMAGE(image_y);
	if (colour){
	   free_IMAGE(image_u);
	   free_IMAGE(image_v);
	}

	system ("ls -l new.ift");
	system ("decode new.ift");
	sprintf(output_fname,"getrms new.pgm %s",input_fname);
	system (output_fname);
 
}

