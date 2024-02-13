/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                    LQDCT decoder Main                        */
/*                      reworked 29/8/96                        */
/*		  zoom implementation 30/10/96			*/
/*##############################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "file_io.h"
#include "mem_hand.h"
#include "blocks.h"
#include "img_huff.h"
#include "image_io.h"
#include "img_huff.h"
#include "quantise.h"
#include "quad.h"
#include "vq.h"

#ifdef FRACTAL
  	#include "frac.h"
        #ifdef RD_SWITCH
                #include "rd_swtch.h"
        #endif
#endif

/* Internal functions */
void decompress_image (IMAGE *, FILE *, BASIS *,HUFF *,HUFF *, int, ZOOM );
void setup_zoom_m6 (IMAGE *, ZOOM ,int);
void setup_image_for_decompress(IMAGE *);
void remove_zoom_padding(IMAGE *,ZOOM , int );


void decompress_image
(IMAGE *image,
 FILE *fp,
 BASIS *basis, 
 HUFF *qtree,
 HUFF *frac,
 int size,
 ZOOM zoom) 
{
	VQ *first;

	printf("read image\n");
	read_compressed_file(fp,image,basis,qtree,size);

	/* check if image is divisable by macro block size */
	if (image->columns%size || image->rows%size)
	   zoom.flag=1; /* turn on zoom to pad image */

	printf("decode qtree\n");
	decode_qtree_image(image,qtree,size);

#ifdef RD_SWITCH
        /* initialise fractal runlength coder */ 
        run_length_code_fractal (0,0,1,0);
#endif

	printf("huffman decode\n");
	huffman_decode_image(image,basis,frac);

	printf("unquant image\n");
	unquantise_image(image,basis);

	printf("setup zoom\n");
	setup_zoom_m6(image,zoom,size); /* only works for 6 coeff */
	
	setup_image_for_decompress(image);

	printf("render image\n");
	render_image(image,basis);

#ifdef FRACTAL
	printf("render fractal enhancement\n");
	render_fractal(image,basis);
#endif

#ifdef  VQ_ON
        printf("render vq enhancement\n");
        first=loadVQ("list.vq");    
        renderVQ(image,first);
        treeDeleteVQ(first); 
#endif

	remove_zoom_padding(image,zoom,size);

	free_blocked_image(image);

	/* clear rubbish */
	free_unsigned_char(qtree->dump->symbols_top);
	qtree->dump->symbols_top=NULL;
	free_unsigned_char(basis->huff->dump->symbols_top);
        basis->huff->dump->symbols_top=NULL;

}

void setup_zoom_m6
(IMAGE *image,
 ZOOM zoom,
 int size)
{
	int i;
	float *c;
	BLOCK *block;

	/* return if the image is not zoomed */
	if (!zoom.flag)
	   return;

	printf("%f\n",zoom.factor);
	/* first find out if image should be zoomed */

	/* make sure window is in image */
	if (zoom.x<0 || zoom.y<0 || zoom.x>=image->columns || zoom.y>=image->rows){
	   zoom.flag=0;/* canel zoom*/
	   return;
	}

	/* scale window to image */
	if (zoom.width==0 || zoom.height==0){
	   zoom.width=image->columns-zoom.x;
	   zoom.height=image->rows-zoom.y;
	}

	/* search through block list and 
	   [1] Check if block is in the image window, if not free coeff 
	  	 if it is then modify c,x,y,size 
	   [2] change 2x2 blocks for expanding and 2x2/4x4/8x8 for shrinking 
	*/

	block=image->tree_top;
	while (block!=NULL){
	      if (block->x>zoom.x-block->size && 
	          block->x<zoom.x+zoom.width+block->size &&
	          block->y>zoom.y-block->size && 
	          block->y<zoom.y+zoom.height+block->size){

		 /* modify coeff locations for 2x2 blocks and zoom */
		 if (zoom.factor>1.0 && block->size==2){
		    c=malloc_float(6,"setup_zoom_m6");
		    c[0]=block->c[0];
		    c[1]=block->c[1];
		    c[2]=0;
		    c[3]=block->c[2];
		    c[4]=block->c[3];
		    c[5]=0;
		    free_float(block->c);
		    block->c=c;
		 }
		 /* modify coeff locations for 8x8/4x4 under shrink blocks */
		 if ((zoom.factor==0.5 && block->size==4) ||
		     (zoom.factor==0.25 && block->size==8)){
		    /* only 2 need swaping */
                    block->c[2]=block->c[3];
                    block->c[3]=block->c[4];
                 }

		 /* scale sizes */
		 block->size=(int)((float)(block->size)*zoom.factor);

		 /* scale coefficents */
	         for (i=0;i<4;i++)
		     block->c[i]*=zoom.factor;
	         if (block->size>2)
	            for (i=4;i<6;i++)
		        block->c[i]*=zoom.factor;
		 
		 if (block->size<1){
		    block->size=1; 
		    block->c[0]*=2.0; /* pushed 2x2 block past limit */   
		 }

#ifdef FRACTAL
		 if (block->size<=2)
		    block->cf=0;
#endif

	         /* scale / shift positions */
	         block->x-=zoom.x;
		 block->x=(int)((float)(block->x)*zoom.factor);
		 block->y-=zoom.y;
		 block->y=(int)((float)(block->y)*zoom.factor);

	      }
	      else {
		    free_float(block->c);
	            block->c=NULL; /* no coeff for DCT .. skips rendering */
#ifdef FRACTAL
		    block->cf=0; /* no coeff for fract .. skips rendering */
#endif
	      }

	      /* move to next block */
	      block=block->next;
	}

	/* alter image dimensions */
	image->rows=(int)((float)zoom.height*zoom.factor);
	image->columns=(int)((float)zoom.width*zoom.factor);

	/* to avoid image rendering problems when zooming */
	size=(int)((float)size*zoom.factor);
	image->columns+=size*2;
	image->rows+=size*2;
	for (block=image->tree_top;block!=NULL;block=block->next){
	    block->x+=size;
	    block->y+=size;
	} 
	

}

void remove_zoom_padding(IMAGE *image,ZOOM zoom,int size)
{
	int cols,rows,i,j,i_pad,j_pad;

	/* return if the image is not zoomed */
	if (!zoom.flag)
	   return;

	/* tmp create correct image size */
	size=(int)((float)size*zoom.factor);
	cols=image->columns-size*2;
	rows=image->rows-size*2;

	/* move image back to right positions */
	for (j=0,j_pad=size;j<rows;j++,j_pad++)
	    for (i=0,i_pad=size;i<cols;i++,i_pad++)
		image->pt[j*cols+i]=image->pt[j_pad*image->columns+i_pad];

	/* move correct image size back*/
	image->columns=cols;
	image->rows=rows;

}

void setup_image_for_decompress(IMAGE *image)
{
	BLOCK *block;
	
	/* malloc space for image */
	image->pt=malloc_int(image->columns*image->rows,"image in zoom");

	block=image->tree_top;
        while (block!=NULL){
	      if (block->c){
	         block->jump=image->columns-block->size;
	         block->pt=image->pt+image->columns*block->y+block->x;
	      }
	      block=block->next;
	}

}

void main
(int argc,
 char *argv[])
{
	int size,colour,last,i;
        IMAGE *image_y,*image_u=NULL,*image_v=NULL;
        BASIS *basis;
        HUFF *qtree,*frac;
        FILE *fp;
	ZOOM zoom;
	char input_fname[256],output_fname[256],pdf_fname[256];
 
	/* set zoom */
	zoom.x=0;
	zoom.y=0;
	zoom.width=0;
	zoom.height=0;
	zoom.factor=1.0;
	/* window must be > max block size */
	
	/* set defaults */
        colour=0;       /* colour = 1 for colour pics */
        sprintf(output_fname,"new.pgm");
        sprintf(pdf_fname,"pdf.all");
        last=0;
 
        /* check arguments */
        for (i=1;i<argc;i++){
            /* output file option */
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

	    /* window position option */
            if (!strcmp(argv[i],"-w")){
               i++;
               if (argv[i]==NULL){
                  printf("NULL window position argument\n");
                  exit(-1);
               }
	       zoom.x=atoi(argv[i++]);
               if (argv[i]==NULL){
                  printf("NULL window postion argument\n");
                  exit(-1);
               }
	       zoom.y=atoi(argv[i]);
	       zoom.flag=1;
               last=i;
               continue;
            }

	    /* window size option */
            if (!strcmp(argv[i],"-s")){
               i++;
               if (argv[i]==NULL){
                  printf("NULL window size argument\n");
                  exit(-1);
               }
	       zoom.width=atoi(argv[i++]);
               if (argv[i]==NULL){
                  printf("NULL window size argument\n");
                  exit(-1);
               }
	       zoom.height=atoi(argv[i]);
	       zoom.flag=1;
               last=i;
               continue;
            }

	    /* zoom option */
            if (!strcmp(argv[i],"-z")){ 
               i++; 
               if (argv[i]==NULL){
                  printf("NULL zoom factor argument\n");
                  exit(-1);
               } 
               zoom.factor=atof(argv[i]); 
	       /* make sure zoom is as specified */
	       if (zoom.factor!=4.0 &&
	           zoom.factor!=2.0 &&
	           zoom.factor!=1.0 &&
	           zoom.factor!=0.5 &&
	           zoom.factor!=0.25)
		   zoom.factor=1.0;
	       else
	           zoom.flag=1;
               last=i;   
               continue; 
            } 

            /* pdf file option */
            if (!strcmp(argv[i],"-p")){
               i++;
               if (argv[i]==NULL){
                  printf("NULL pdf file argument\n");
                  exit(-1);
               }
               sprintf(pdf_fname,"%s",argv[i]);
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
           printf("%s:\n"
        "[-o <output>]   			default: new.pgm\n"
        "[-p <pdf file>] 			default: pdf.all\n"
        "[-w <x> <y> ] - window top left corner default: 0,0 \n"
        "[-s <width> <height>] - window size 	default: image size\n" 
        "[-z <zoom 4,2,1,0.5,0.25>] - zoom	default: 1.0\n"
        "<input> - " FILE_SUFFIX " file\n",argv[0]);
           exit(-1);
        }

        /* load last 2 items */
        sprintf(input_fname,"%s",argv[last+1]);


	/* setup  basis structures */
        basis=malloc_BASIS(1,"main");
        size=32;
        setup_cosine_basis(basis,(int)((float)size*4)); /* 4 - max zoom */

	/* setup huffman coders */
	qtree=malloc_HUFF(1,"main qtree");
	frac=malloc_HUFF(1,"main frac");
        setup_huffman_coder(basis,qtree,frac,pdf_fname);

	printf("decompressing image\n");

	/* setup for decompress */
        image_y=malloc_IMAGE(1,"main y");
	image_y->pt=NULL;
	fp=read_compressed_file_header (input_fname,
			&(image_y->columns),&(image_y->rows),&colour);

	if (colour){
           image_u=malloc_IMAGE(1,"main u");
	   image_u->pt=NULL;
	   image_u->rows=image_y->rows;
	   image_u->columns=image_y->columns;

           image_v=malloc_IMAGE(1,"main v");
	   image_v->pt=NULL;
	   image_v->rows=image_y->rows;
	   image_v->columns=image_y->columns;
	}

        /* decompress image */
	decompress_image(image_y,fp,basis,qtree,frac,size,zoom);
        if (colour){
	   decompress_image(image_u,fp,basis,qtree,frac,size,zoom);
	   decompress_image(image_v,fp,basis,qtree,frac,size,zoom);
        }
        fclose(fp);

	printf("writing image\n");

	/* Write image to file */
        if (colour)
           write_ppm_file(image_y,image_u,image_v,output_fname);
        else 
            write_pgm_file(image_y,output_fname);

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


}
