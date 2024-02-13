 /*##############################################################*/
/*			David Bethel				*/
/*		       Bath University 				*/
/*		      daveb@ee.bath.ac.uk			*/
/*		    Wavelet Encoder Program			*/
/*		    + decoder + mirroring	                */
/*		  	   23/1/96				*/
/*	            Reworked 2/7/96 			        */
/*##############################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "arith.h"
#include "mem_hand.h"
#include "image_io.h"
#include "filters.h"
#include "zero_tree.h"
#include "file_io.h"




/* Internal functions */
void wavelet_compress(IMAGE *,PACKED *,int );


/* this function calls the routines which wavelet compress the image, */
/* and then arithmetic code the result                                */

void wavelet_compress
(IMAGE *image,
 PACKED *dump,
 int n_bits)
{

	int i,scales;

	/* 4 scales */
	scales=4;

	/*ENCODER*/
 
	/* shift image up to add accuracy */
	for (i=0;i<image->columns*image->rows;i++)
	    image->pt[i]<<=5;

	/* perform wavelet filtering */
	wavelet_filter_image(image,scales); 
	        
	/* start arthmetric coder */
	initialize_arithmetic_encoder(dump->s);

	/* use zero tree coding to compress image*/
	zero_tree_code_image(image,dump,scales);

}

void main
(int argc,
 char **argv)
{
	int colour=0,last,i;
	int n_bits_y=0,n_bits_u=0,n_bits_v=0;
	int columns,rows;
	PACKED *dump_y,*dump_u,*dump_v;
	IMAGE *image_y,*image_u,*image_v;
	char input_fname[256],output_fname[256];
	float bpp;
	FILE *fp;

	/* set defaults for program */
	sprintf(output_fname,"new.bwt");
	last=0;

	/* check command line arguments */
	for (i=1;i<argc;i++){
	    
	    /* command line output file option */
	    if (!strcmp(argv[i],"-o")){
	       i++;
	       if (argv[i]==NULL){
		  printf("NULL output argument\n");
	          exit(-1);
	       }
	       sprintf(output_fname,"%s",argv[i]);
	       last=i;
	       continue;
	    }

	    /* unknown command line option */
	    if (!strncmp(argv[i],"-",1)){
	       printf("Unknown option\n");
	       exit(-1);
	    }
	}

	/* warn if command line wrong input length */
	if (last+2>=argc){
	   printf("%s:\n ", argv[0]);
	   printf("[-o <output>]  default: new.bwt\n");
	   printf("<bpp>   - compression\n");
	   printf("<input> - pgm/ppm file\n");
           exit(-1);    
        }

	/* load last 2 items from command line */
	bpp=atof(argv[last+1]);
	sprintf(input_fname,"%s",argv[last+2]);
	
	/* read header from the image, store image details */
	/* in variables                                    */
	fp=read_raw_header(input_fname,&rows,&columns,&colour);

	/* make space for images */
	image_y=malloc_IMAGE(1,"main y");	
	if (colour){
	   image_u=malloc_IMAGE(1,"main u");	
	   image_v=malloc_IMAGE(1,"main v");	
	}

	/* sets up image sizes in internal image structures */
	image_y->rows=rows;
	image_y->columns=columns;
	
	/* read the image into the appropriate data structure(s) */
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

	/* work out bits availible */
	n_bits_y=image_y->rows*image_y->columns*bpp;

	/* account for headers */
	n_bits_y-=128;
	if (colour)
	   n_bits_y-=256;

	/* set colour compression strengths */ 
	if (colour){
	   n_bits_u=(int)(0.075*(float)n_bits_y); /* 10% colour */
	   n_bits_v=(int)(0.075*(float)n_bits_y);
	   n_bits_y-=(n_bits_u+n_bits_v);	 /* keep totals bits same */
	}

	/* set up space for compressed  Y image  */
	dump_y=malloc_PACKED(1,"main y");	
	allocate_dump(dump_y,n_bits_y);

	/* if dealing with a colour image, set up space for U and V images */
	if (colour){
	   dump_u=malloc_PACKED(1,"main u");	
	   allocate_dump(dump_u,n_bits_u);
	   dump_v=malloc_PACKED(1,"main v");	
	   allocate_dump(dump_v,n_bits_v);
	}

	/* compress / decompress image */
	wavelet_compress(image_y,dump_y,n_bits_y);

	if (colour){
	   wavelet_compress(image_u,dump_u,n_bits_u);
	   wavelet_compress(image_v,dump_v,n_bits_v);
	}
	
	/* write compressed image to file */
	fp=write_compressed_file_header(output_fname, columns, rows, colour);
	write_compressed_file(fp,image_y,dump_y);
	if (colour){
	   write_compressed_file(fp,image_u,dump_u);
	   write_compressed_file(fp,image_v,dump_v);
	}
	fclose(fp);


	/* exit gracefully */
	/* i.e. free all memory allocated during run time */
	free_IMAGE(image_y);
	free_SYMBOL(dump_y->s);
	free_PACKED(dump_y);
	if (colour){
	   free_IMAGE(image_u);
	   free_SYMBOL(dump_u->s);
	   free_PACKED(dump_u);

	   free_IMAGE(image_v);
	   free_SYMBOL(dump_v->s);
	   free_PACKED(dump_v);
	}
}


