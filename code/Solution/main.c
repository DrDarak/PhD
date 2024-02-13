/*##############################################################*/
/*		Test of wavelet coder 				*/
/*##############################################################*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "miscw.h"
#include "struct.h"
#include "mem_hand.h"
#include "code.h"
#include "huff.h"
#include "blocks.h"
#include "filters.h"
#include "optim_q.h"
#include "q_tables.h"
#include "image_io.h"
#include "bitstrm.h"

void main
(int argc,
 char **argv)
{
	int last,i,j,scales,rows,columns,dummy;
	IMAGE *image,*colour,*orig;
	HARDCODED_WAVELET *c;
	QUANT *quant,*quant_col;
	FILE *fp;
	float xt,xt_col;
    HUFF *ac,*dc;
	HUFF *ac_col,*dc_col;
	BLOCK *block;
    char input_fname[256],output_fname[256],pdf_fname[256];
	char quant_fname[256],quant_col_fname[256],col_pdf_fname[256];
    char wave_fname[256];
    float bpp;
    HUFF_TYPE type;
	QUANT_TYPE mode;

        /* set defaults */
#ifdef WIN32
        sprintf(output_fname,"d:\\daveb\\new.pgm");
        sprintf(pdf_fname,"d:\\daveb\\y.pdf");
		sprintf(col_pdf_fname,"d:\\daveb\\uv.pdf");
        sprintf(quant_fname,"d:\\daveb\\q_out");
        sprintf(quant_col_fname,"d:\\daveb\\q_out.col");
        sprintf(wave_fname,"d:\\daveb\\ucla1.wav");
#else
        sprintf(output_fname,"new.pgm");
        sprintf(pdf_fname,"pdf.all");
        sprintf(quant_fname,"q_out");
        sprintf(quant_col_fname,"q_out.col");
        sprintf(wave_fname,"ucla1.wav");
#endif
        last=0;
        type=normal; /* NB: when appending use short seq... int overflow */
        mode=read;

        /* check arguments */
        for (i=1;i<argc;i++){

            /* outpur file option */
            if (!strcmp(argv[i],"-o")){
               i++;
               if (argv[i]==NULL){
                  Fatal("NULL output argument\n");
               } 
               sprintf(output_fname,"%s",argv[i]);
               /*DebugF("-o: %s\n",output_fname);*/
               last=i;
               continue;
            }
 
            /*  append option */
            if (!strcmp(argv[i],"-a")){
               type=append;
               last=i;
               continue;
            }

	    /*  quantisation mode */
            if (!strcmp(argv[i],"-q")){
               mode=create;
               last=i;
               continue;
            }

	    /*  quantisation mode */
            if (!strcmp(argv[i],"-u")){
               mode=update;
               last=i;
               continue;
            }

	    /*  fix tables after reduction option */ 
            if (!strcmp(argv[i],"-f")){  
               type=fix;
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
                  Fatal("NULL output argument\n");
               } 
               sprintf(pdf_fname,"%s",argv[i]);
               /*DebugF("-w: %s\n",wavelet_fname);*/
               last=i;
               continue;
            }
 
            /* unknown option */
            if (!strncmp(argv[i],"-",1)){
               Fatal("Unknown option\n");
            }
        }
 
        /* warn if wrong input length */
        if (last+2>=argc){
           Fatal("%s:\n"
        "[-a ] appends huffman pdf\n"
        "[-w ] wipe huffman pdf\n"
        "[-f ] fix huffman pdf\n"
        "[-q ] creates new Q table\n"
        "[-o <output>]   default: new.y\n"
        "[-p <pdf file>] default: pdf.all\n"
        "<bpp>   - compression\n"
        "<input> - pgm/ppm file\n",argv[0]);
        }
        /* load last 2 items */
        bpp=(float)atof(argv[last+1]);
        sprintf(input_fname,"%s",argv[last+2]);
 
    /* -------------------------------- */
    /*       HUFFMAN CODER SETUP	    */
    /* -------------------------------- */
    /* setup huffman coders */
    dc=malloc_HUFF(1,"main dc huff table"); /* huffman tables */
    ac=malloc_HUFF(1,"main ac huff table"); /* huffman tables */
 	dc_col=malloc_HUFF(1,"main dc huff table"); /* huffman tables */
    ac_col=malloc_HUFF(1,"main ac huff table"); /* huffman tables */
 
    /* open huffman pdf file */
    fp=fopen(pdf_fname,"rb");
	NULL_FILE_error(fp,"main setup huff");
    setup_huffman_tree (dc,fp);
    setup_huffman_tree (ac,fp);
    fclose(fp);
 	fp=fopen(col_pdf_fname,"rb");
	NULL_FILE_error(fp,"main setup huff col");
    setup_huffman_tree (dc_col,fp);
    setup_huffman_tree (ac_col,fp);
    fclose(fp);

    /* wipe huffman pdf */
    if (type==wipe){
       fp=fopen(pdf_fname,"wb");
	   NULL_FILE_error(fp,"main wipe");
       wipe_huffman_pdf(dc,fp);
       wipe_huffman_pdf(ac,fp);
       fclose(fp);
	   fp=fopen(col_pdf_fname,"wb");
	   NULL_FILE_error(fp,"main wipe col");
       wipe_huffman_pdf(dc_col,fp);
       wipe_huffman_pdf(ac_col,fp);
       fclose(fp);
       Fatal("wipeing huffman pdf file\n");
    }



	/* setup quantisation */
	scales=5;
	quant=malloc_QUANT(1,"main");
	quant->size=(1<<(scales))*(1<<(scales-1));
	quant->q_out=malloc_int(quant->size,"main quant q_out");
	quant->q=malloc_float(quant->size,"main quant q");
	quant->nitems=malloc_int(quant->size,"main quant nitems");
	if (mode!=create)
           read_quantisation(quant_fname,quant);

 	/* setup colour quantisation */
    quant_col=malloc_QUANT(1,"main");
    quant_col->size=(1<<(scales))*(1<<(scales-1));
    quant_col->q_out=malloc_int(quant_col->size,"main quant_col q_out");
    quant_col->q=malloc_float(quant_col->size,"main quant_col q");
    quant_col->nitems=malloc_int(quant_col->size,"main quant_col nitems");
    if (mode!=create)
       read_quantisation(quant_col_fname,quant_col);

	/* setup the image */

	fp=read_raw_header(input_fname,
                      &rows,
                      &columns,
                      &dummy);

	image=new IMAGE(bpp,columns,rows);
   	orig=new IMAGE(0.01,columns,rows);
    colour=new IMAGE(bpp,176,288);

	printf("%d\t%d\n",image->columns,image->rows);
  	
	/* setup the wavelet filters */
	c=new HARDCODED_WAVELET;
	c=HardCodedWavelet_New(image->columns,image->rows);
//	setup_wavelet (wave_fname,c,image->columns,image->rows);
	/*
	fp=read_raw_header (input_fname,&(image->rows),&(image->columns),&dummy);
	*/
//	fp=fopen(input_fname,"rb");

	/* setup blocked image */
	setup_blocks (image,scales);
	setup_blocks (colour,scales);

    /* calculate number of bits fro image */
    xt=    (float)bpp*(float)(image->columns*image->rows);
    quant->bpp=bpp; // updates bpp in quant tables
	
	xt_col=(float)0.1*bpp*(float)(image->columns*image->rows); /* colour has 10% of bits */
	quant_col->bpp=0.1*bpp; // updates bpp in quant tables
	
	DebugF("Grey BITS/FRAME %d\n",(int)xt);
	DebugF("Colour BITS/FRAME %d\n",(int)xt_col);

	for (j=0;j<1;j++){
	    get_pgm_file(fp,image);
//		get_pgm_file(fp,colour);

		imgcpy(orig,image);	// copies grey scale image to tmp file
		/* shift image up */
		for (i=0;i<image->rows*image->columns;i++)
			image->pt[i]<<=2;
//		for (i=0;i<colour->rows*colour->columns;i++){
//			 colour->pt[i]-=128;
//			 colour->pt[i]<<=2;
//		}				


        DebugF("Compress Y\n");
	    compress_image (image,c,quant,type,mode,xt,dc,ac,scales,1,1);

//		DebugF("Compress UV\n");
//	    compress_image (colour,c,quant_col,type,mode,xt_col,dc_col,ac_col,scales,1,1);
		
		/* stage 5 threstholds and bitrate control */
		//printf("%d\n",mark_threshold_trees(image,100,(int)xt));
		//for (block=image->tree_top;bloc;block=block->next)
		//printf("%ld\t%d\n",block->error,block->transmit);
		//mark_threshold_trees(colour,(int)xt_col);
		//printf("quant=%f[%f]\n",estimate_quant_for_image(image,xt,50000),xt);

		//printf("bits=%d\n",mark_for_transmittion(image,(int)xt/2));

	    if (type==normal){
           DebugF("Decompress Y\n");
	       decompress_image (image,c,quant,dc,ac,scales,1,1);
  //         DebugF("Decompress UV\n");
//	       decompress_image (colour,c,quant_col,dc_col,ac_col,scales,1,1);
		}

		/* shift image down  */
		for (i=0;i<image->rows*image->columns;i++)
			image->pt[i]>>=2;
//		for (i=0;i<colour->rows*colour->columns;i++){
//			colour->pt[i]>>=2;
 //  			colour->pt[i]+=128;
//		}

		printf("RMSE=%f\n",sqrt(get_mse(orig,image)));

	    /* output image */
        write_pgm_file(image,output_fname);
 //       write_pgm_file(colour,"col.pgm");

	    DebugF("frame=%d\n",j);

	    if (mode==create)
	       mode=update;  /* switch to update after creating q files */
	}

	if (mode==create || mode==update){
           write_quantisation(quant_fname,quant);
//           write_quantisation(quant_col_fname,quant_col);
	}

	if (type==append || type==fix){
           fp=fopen(pdf_fname,"wb");
		   NULL_FILE_error(fp,"main append");
           save_huffman_pdf (dc,fp);
           save_huffman_pdf (ac,fp);
           fclose(fp);
//		   fp=fopen(col_pdf_fname,"wb");
//		   NULL_FILE_error(fp,"main append");
//           save_huffman_pdf (dc_col,fp);
//           save_huffman_pdf (ac_col,fp);
//           fclose(fp);
           DebugF("appending huffman pdf to disk\n");
        }

        /* free memory used by coder */
	free_blocked_image(image);
	free_blocked_image(colour);
    delete image;	// c++ versions
	delete colour;
	delete orig;
    free_QUANT(quant);
	free_QUANT(quant_col);

    free_huffman_tree(dc_col);
    free_HUFF(dc_col);
    free_huffman_tree(ac_col);
    free_HUFF(ac_col);

	free_huffman_tree(dc);
    free_HUFF(dc);
    free_huffman_tree(ac);
    free_HUFF(ac);

}
