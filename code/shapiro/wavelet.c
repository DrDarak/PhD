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
#include "wavelet.h" 

void wavelet_compress
(IMAGE *image,
 WAVELET *c,
 PACKED *dump,
 int n_bits)
{

	int i,scales;

	/* 4 scales */
	scales=4;

	/*ENCODER*/
        printf("----Starting Encode-----\n");	
	/* shift image up to add accuracy */
	for (i=0;i<image->columns*image->rows;i++)
	    image->pt[i]<<=5;

	/* perform wavelet filtering */
	wavelet_filter_image(image,c,scales); 
	        
	/* start arthmetric coder */
	initialize_arithmetic_encoder(dump->s);

	/* use zero tree coding to compress image*/
	zero_tree_code_image(image,dump,scales);

        printf("----Starting Decode-----\n");	
	/*DECODER*/
	/* set up dump for decoding */
	dump->symbols=dump->symbols_top;
        dump->sym_mask=0x01;
	dump->refine=dump->refine_top;
        dump->ref_mask=0x01;
	dump->bits=0;	

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
	wavelet_inv_filter_image(image,c,scales);  

	/* shift image back to original */
	for (i=0;i<image->columns*image->rows;i++)
	    image->pt[i]>>=5;

}

void main
(int argc,
 char **argv)
{
	int colour=0,pgm=1;
	int n_bits_y=0,n_bits_u=0,n_bits_v=0;
	PACKED *dump_y,*dump_u,*dump_v;
	IMAGE *image_y,*image_u,*image_v;
	WAVELET *c;
	FILE *fp;
	char cn[250];

	/* check arguments */
	if (argc<5){
	   printf("%s: 
	<input> - no extention -> gets .y .u .v or .raw file
        <output> - no extention -> outputs .y .u .v or .raw  file
        <wavelet> - no extension -> gets .wav file
        <bpp> 	- compression
        <cppm/cpgm/y> - ip / op type \n",argv[0]);
	   exit(-1);
	}

	/* colour = 1 for colour pics */
	/* pgm = 1 for YUV loadin / out */

	if (argc>=6){
	   if (strcmp(argv[5],"cppm")==0){
	      colour=1;
	      pgm=0;
	      printf("Colour Wavelet Codec:raw\n");
	   }
	   if (strcmp(argv[5],"cpgm")==0){
	      colour=1;
	      pgm=1;
	      printf("Colour Wavelet Codec:y u v\n");
	   }
	   if (strcmp(argv[5],"y")==0){
	      colour=0;
	      pgm=1;
	      printf("Greyscale Wavelet Codec\n");
	   }
	}
	else {
	     colour=0;	
	     pgm=1;
	     printf("Greyscale Wavelet Codec\n");
	}
	
	/* make space for images */
	image_y=malloc_IMAGE(1,"main y");	
	if (colour){
	   image_u=malloc_IMAGE(1,"main u");	
	   image_v=malloc_IMAGE(1,"main v");	
	}

	/* get file from disk */
	if (colour){
	   if (pgm){
	      sprintf(cn,"%s.y",argv[1]);
	      get_pgm_file(image_y,cn);
	      sprintf(cn,"%s.u",argv[1]);
	      get_pgm_file(image_u,cn);
	      sprintf(cn,"%s.v",argv[1]);
	      get_pgm_file(image_v,cn);
	   }
	   else {
	        sprintf(cn,"%s.raw",argv[1]);
	        get_ppm_file(image_y,image_u,image_v,cn);
	   }
	}
	else {
	     sprintf(cn,"%s.y",argv[1]);
	     get_pgm_file(image_y,cn);
	}
	
	/* get wavelet from disk */
	c=malloc_WAVELET(1,"main");	
	sprintf(cn,"%s.wav",argv[3]);
	setup_wavelet(cn,c);

	/* work out bits availible */
	n_bits_y=image_y->rows*image_y->columns*atof(argv[4]);

	/* account for headers */
	n_bits_y-=128;
	if (colour)
	   n_bits_y-=256;

	/* set colour compression strengths */ 
	if (colour){
	   n_bits_u=(int)(0.05*(float)n_bits_y); /* 10% colour */
	   n_bits_v=(int)(0.05*(float)n_bits_y);
	   n_bits_y-=(n_bits_u+n_bits_v);	 /* keep totals bits same */
	}


	/* set up space for compressed  Y image  */
	dump_y=malloc_PACKED(1,"main y");	
	allocate_dump(dump_y,n_bits_y);

	if (colour){
	   dump_u=malloc_PACKED(1,"main u");	
	   allocate_dump(dump_u,n_bits_u);
	   dump_v=malloc_PACKED(1,"main v");	
	   allocate_dump(dump_v,n_bits_v);
	}

	/* compress / decompress image */
	wavelet_compress(image_y,c,dump_y,n_bits_y);

	if (colour){
	   wavelet_compress(image_u,c,dump_u,n_bits_u);
	   wavelet_compress(image_v,c,dump_v,n_bits_v);
	}
	
	/* write compressed image to file */
	sprintf(cn,"%s.bwt",argv[2]);
	fp=fopen(cn,"wb");
	write_compressed_file(fp,image_y,dump_y);
	if (colour){
	   write_compressed_file(fp,image_u,dump_u);
	   write_compressed_file(fp,image_v,dump_v);
	}
	fclose(fp); 

	/* Write image to file */
	if (colour){
	   if (pgm){
	      sprintf(cn,"%s.y",argv[2]);
	      write_pgm_file(image_y,cn);
	      sprintf(cn,"%s.u",argv[2]);
	      write_pgm_file(image_u,cn);
	      sprintf(cn,"%s.v",argv[2]);
	      write_pgm_file(image_v,cn);
	   }
	   else {
	        sprintf(cn,"%s.raw",argv[2]);
	        write_ppm_file(image_y,image_u,image_v,cn);
	   }
	}
	else {
	     sprintf(cn,"%s.y",argv[2]);
	     write_pgm_file(image_y,cn);
	}

	/* exit gracefully */
	free_int(image_y->pt);
	free_IMAGE(image_y);
	free_SYMBOL(dump_y->s);
	free_PACKED(dump_y);
	if (colour){
	   free_int(image_u->pt);
	   free_IMAGE(image_u);
	   free_SYMBOL(dump_u->s);
	   free_PACKED(dump_u);

	   free_int(image_v->pt);
	   free_IMAGE(image_v);
	   free_SYMBOL(dump_v->s);
	   free_PACKED(dump_v);

	}
	free_WAVELET(c);
}

