/*##############################################################*/
/*			David Bethel				*/
/*		       Bath University 				*/
/*		      daveb@ee.bath.ac.uk			*/
/*		      Bath Binary image coder 			*/
/*			Encoder 1/10/96				*/
/*##############################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "mem_hand.h"
#include "vq.h"
#include "file_io.h"
#include "quad.h"
#include "huff.h"
#include "blocks.h"
#include "image_io.h"
typedef enum huff_type {normal,wipe,append} HUFF_TYPE;

void main
(int argc,
 char *argv[])
{
	IMAGE *image;
	PACKED *qtree;
	int size,nitems,i,j,last,comp;
	int *map;
 	char input_fname[256],output_fname[256],pdf_fname[256];
        FILE *fp;
        HUFF_TYPE type;
 
        /* set defaults */
        sprintf(output_fname,"new.bbt");
        sprintf(pdf_fname,"pdf.all");
        last=0;
        type=normal;
	comp=100;
 
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
                  printf("NULL input argument\n");
                  exit(-1);
               }
               sprintf(pdf_fname,"%s",argv[i]);
               /*printf("-w: %s\n",wavelet_fname);*/
               last=i;
               continue;
            }

	    /* compression  option */
            if (!strcmp(argv[i],"-c")){
               i++;
               if (argv[i]==NULL){
                  printf("NULL compression argument\n");
                  exit(-1);
               } 
               comp=atoi(argv[i]);
	       if (comp<0 || comp>100){
		  printf("Invalide quality measure - outside range 0-100\n");
		  exit(-1);
	       }
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
           printf("\t[-a ] appends huffman pdf\n");
           printf("\t[-w ] wipe huffman pdf\n");
           printf("\t[-o <output>]          default: new.bbt\n");
           printf("\t[-p <pdf file>]        default: pdf.all\n");
	   printf("\t[-c <percent quality>] default:100 percent\n");
           printf("\t<input> - pgm/ppm file\n");
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

	/* huffman file options */
        if (type==wipe){
	   fp=fopen("pdf.all","wb");
	   wipe_huffman_pdf (qtree,fp);
	   fclose(fp);
           printf("wipeing huffman pdf file\n");
           exit(-1);
        }

	derive_mapping_from_pdfs(map,qtree,comp);

	/* setup image */
	image=malloc_IMAGE(1,"main");
	fp=read_pgm_header (input_fname,image);

	/* allocate qtree stream  x2 image size > binary data */
        allocate_dump(qtree,image->columns*image->rows*4); /*fudge neweds fixing */

	/* compress binary image size rows at at time ... avoids mem problems */
	for (j=0;j<image->rows;j+=size){
	    nitems=read_pgm_file (fp,image,size);
	    setup_blocks (image,size);
	    form_quad_tree(image);
	    map_indexes(image,map);
	    encode_qtree_image (image,qtree,size);
	    free_blocked_image (image);
	}
	fclose(fp);
	printf("bits=%d\n",8*(qtree->symbols-qtree->symbols_top));
	printf("compression=%f\n",(float)(image->columns*image->rows)
				/(float)(8*(qtree->symbols-qtree->symbols_top)));


	/* save compressed file */
	fp=fopen(output_fname,"wb");
	write_compressed_file (fp,image,qtree,comp);
	fclose(fp);

	/* huffman file options */
        if (type==append){
	   fp=fopen("pdf.all","wb");
	   save_huffman_pdf (qtree,fp);
	   fclose(fp);
           printf("appending huffman pdf to disk\n");
        }
	printf("%f\n",huffman_bpc(qtree));


	/* clear memory used for compression */
	free_int(map);
	free_huffman_tree(qtree);
        free_PACKED(qtree);
        free_IMAGE(image);


}



