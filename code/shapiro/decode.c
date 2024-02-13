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
void wavelet_decompress(IMAGE *,PACKED *);


/* this function calls the routines which un arithmetic code the image, */
/* and then un wavelet compress the image                               */

void wavelet_decompress
(IMAGE *image,
 PACKED *dump)
{

	int i,scales;

	/* 4 scales */
	scales=4;

        printf("----Starting Decode-----\n");	

	/* set up arithmetic coder */
        initialize_arithmetic_decoder(dump);
	
	/* clear image for decode */
	for (i=0;i<image->columns*image->rows;i++)
	    image->pt[i]=0;

	/* decode zero tree from data */
	zero_tree_decode_image(image,dump,scales);

	/* clear the marks before inverse wavelet */
	clear_markers (image);

	/* un filter wavelet coded image  */
	wavelet_inv_filter_image(image,scales);  

	/* shift image back to original */
	for (i=0;i<image->columns*image->rows;i++)
	    image->pt[i]>>=5;

}

void main
(int argc,
 char **argv)
{
	int colour=0,cflag=0,fnameflag=0,rows,columns,last,i;
	PACKED *dump_y,*dump_u,*dump_v;
	IMAGE *image_y,*image_u,*image_v;
	FILE *fp;
	char input_fname[255],output_fname[255];

	/* set defaults */
        colour=0;       /* colour = 1 for colour pics */
        last=0;

        /* check command line arguments */
        for (i=1;i<argc;i++){
            /* colour option */
            if (!strcmp(argv[i],"-g")){
               cflag=1;
               last=i;
               /*printf("-g: greyscale\n");*/
               continue;
            }

            /* output file option */
            if (!strcmp(argv[i],"-o")){
               i++;
               if (argv[i]==NULL){
                  printf("NULL output argument\n");
                  exit(-1);
               }
	       fnameflag=1; /* 1- indicates user fname */
               sprintf(output_fname,"%s",argv[i]);
               /*printf("-o: %s\n",output_fname);*/
               last=i;
               continue;
            }

            /* unknown command line option */
            if (!strncmp(argv[i],"-",1)){
               printf("Unknown option\n");
               exit(-1);
            }
        }
 
        /* warn if wrong command line length */
        if (last+1>=argc){
           printf("%s:\n",argv[0]);
           printf("[-o <output>]  default: new.raw\n");
           printf("[-g ] -greyscale  default: as per image type\n");
           printf("<input> - .bwt file\n");
           exit(-1);
        }
 
        /* load last 2 items from the command line */
        sprintf(input_fname,"%s",argv[last+1]);
	
	/* read compressed image header from file */
        fp=read_compressed_file_header(input_fname, &columns, &rows, &colour);
	NULL_FILE_error(fp,"main");

	if (cflag) colour = 0;

	/* default file name */
	if (!fnameflag)
	   if (colour)
              sprintf(output_fname,"new.ppm");
	   else
              sprintf(output_fname,"new.pgm");

	/* make space for images */
	image_y=malloc_IMAGE(1,"main y");	
	if (colour){
	   image_u=malloc_IMAGE(1,"main u");	
	   image_v=malloc_IMAGE(1,"main v");	
	}

	/* set image rows and columns */
	image_y->rows = rows;
	image_y->columns = columns;
	if (colour) {
	  image_u->rows = rows;
	  image_u->columns = columns;
	  image_v->rows = rows;
	  image_v->columns = columns;
	}	  

	/* set up space for compressed  Y image, and U and V images if required  */
	dump_y=malloc_PACKED(1,"main y");	
	if (colour){
	   dump_u=malloc_PACKED(1,"main u");	
	   dump_v=malloc_PACKED(1,"main v");	
	}

	/* read compressed image from file */
	read_compressed_file(fp,image_y,dump_y);
        if (colour){
           read_compressed_file(fp,image_u,dump_u);
           read_compressed_file(fp,image_v,dump_v);
        }
        fclose(fp); 

	/* decompress image */
	wavelet_decompress(image_y,dump_y);

	if (colour){
	   wavelet_decompress(image_u,dump_u);
	   wavelet_decompress(image_v,dump_v);
	}
	


	/* Write image to file */
	if (colour)
	        write_ppm_file(image_y,image_u,image_v,output_fname);
	else 
	     write_pgm_file(image_y,output_fname);
	

	/* exit gracefully */
	/* i.e. free up all memory allocated during run time */
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

