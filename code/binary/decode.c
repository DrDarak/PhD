/*##############################################################*/
/*			David Bethel				*/
/*		       Bath University 				*/
/*		      daveb@ee.bath.ac.uk			*/
/*                    Bath Binary image coder                   */ 
/*                      Decoder 14/11/96                        */ 
/*##############################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "mem_hand.h"
#include "image_io.h"
#include "file_io.h"
#include "blocks.h"
#include "huff.h"
#include "quad.h"
#include "vq.h"

void main
(int argc,
 char *argv[])
{
	IMAGE *image;
	PACKED *qtree;
	int size,i,j,last,comp;
	int *map;
 	char input_fname[256],output_fname[256],pdf_fname[256];
        FILE *fp;
 
        /* set defaults */
        sprintf(output_fname,"new.pgm");
        sprintf(pdf_fname,"pdf.all");
        last=0;
 
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

            /* pdf  file option */
            if (!strcmp(argv[i],"-p")){
               i++;
               if (argv[i]==NULL){
                  printf("NULL input argument\n");
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
        if (last+1>=argc){
           printf("%s:\n",argv[0]);
           printf("\t[-o <output>]          default: new.pgm\n");
           printf("\t[-p <pdf file>]        default: pdf.all\n");
           printf("\t<input> - .bbt file\n");
           exit(-1);
        }

	/* load last */
        sprintf(input_fname,"%s",argv[last+1]);

	size=48; /* multiple of MIN_SIZE */
	map=malloc_int(1<<(MIN_SIZE*MIN_SIZE),"main map");
	
	/* setup qtree output file */
	qtree=malloc_PACKED(1,"main");
	fp=fopen(pdf_fname,"rb");
	setup_huffman_tree(qtree,fp);
	fclose(fp);

	/* setup image */
	image=malloc_IMAGE(1,"main");

	/* read compressed file */
	fp=fopen(input_fname,"rb");
	comp=read_compressed_file_header(fp,image);
	derive_mapping_from_pdfs(map,qtree,comp);
	read_compressed_file (fp,qtree);
	fclose(fp);

	
	/* decompress file size rows at a time .. avoids memory problems */
	fp=write_pgm_header (output_fname,image);
	for (j=0;j<image->rows;j+=size){
	    decode_qtree_image (image,qtree,size);
	    reform_image (image);
	    /* provents too much image being compressed */
	    if (j+size>=image->rows)
	       size=image->rows-j;
	    write_pgm_file(fp,image,size);
	    free_blocked_image (image);
	}

	fclose(fp);

	/* clear memory used for compression */
        free_int(map);
        free_huffman_tree(qtree);
        free_PACKED(qtree);
        free_IMAGE(image);

}



