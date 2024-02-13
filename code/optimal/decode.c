/*##############################################################*/
/*		Test of wavelet coder 				*/
/*##############################################################*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "code.h" 

typedef enum huff_type {normal,wipe,append,fix} HUFF_TYPE;
typedef enum quant_type {read,create} QUANT_TYPE;

void compress_image
(IMAGE *image,
 WAVELET *c,
 HUFF_TYPE type,
 QUANT_TYPE mode,
 float xt,
 HUFF *dc,
 HUFF *ac,
 int scales)
{
	int size,*q_out=NULL,diff,accuracy;
	float *x_out;

	wavelet_filter_image (image,c,scales);
        setup_blocks(image,scales);
 
	accuracy=1000; /* -+ 1000 bits */
        size=(1<<(scales))*(1<<(scales-1));
 
        if (mode==create){
           /* aplly wide ban quantisation to generate quantiation tables */
           form_quantisation_tables(image,size);
           quantise_coefficients(image,size);
 
           /* alloccate memoey for output */
           x_out=malloc_float(size,"compress");
 
           /* solve problem to within 1000 bits */
           solve(image->data,x_out,xt,1000.0,size);
 
           /* alloccate memoey for quantisation */
           q_out=malloc_int(size,"compress");
 
           /* Now need to covert bits allocated to each coefficient -> quantisation */
           convert_bits_to_quant(image,x_out,q_out,size);
	   free_float(x_out);
 
           if (type==normal || type==fix){
              diff=2*accuracy;
              while (diff>accuracy){
                    diff=ammend_quantisation (image,q_out,dc,ac,(int)xt);
                    diff-=(int)xt;
                    diff=myabs(diff);
                    printf("diff=%d\n",diff);
              }
           }
           write_quantisation("q_out",q_out,size);
        }
 
        if (mode==normal)
           q_out=read_quantisation("q_out",size);
 
        huffman_encode_image (image,q_out,dc,ac);
 
        printf("HUFFMAN BITS %d\n",8*(dc->dump->symbols-dc->dump->symbols_top));
        if (type==normal){
           dc->dump->symbols=dc->dump->symbols_top;
           dc->dump->sym_mask=1;
           huffman_decode_image (image,q_out,dc,ac);

           reform_image (image,scales);              
           wavelet_inv_filter_image (image,c,scales);

        }

	free_blocked_image (image);
	free_int(q_out);

}

void main
(int argc,
 char **argv)
{
	int last,i,scales;
	IMAGE *image,*col;
	WAVELET *c;
	int dummy;
	FILE *fp;
	float xt;
        HUFF *ac,*dc;
        PACKED *dump;
        char input_fname[256],output_fname[256],pdf_fname[256];
        float bpp;
        HUFF_TYPE type;
	QUANT_TYPE mode;

        /* set defaults */
        sprintf(output_fname,"new.y");
        sprintf(pdf_fname,"pdf.all");
        last=0;
        type=normal;
        mode=read;

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

	    /*  quantisation mode */
            if (!strcmp(argv[i],"-q")){
               mode=create;
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
        [-f ] fix huffman pdf
        [-q ] creates new Q table
        [-o <output>]   default: new.y
        [-p <pdf file>] default: pdf.all
        <bpp>   - compression
        <input> - pgm/ppm file\n",argv[0]);
           exit(-1);
        }
        /* load last 2 items */
        bpp=atof(argv[last+1]);
        sprintf(input_fname,"%s",argv[last+2]);
 
        /* -------------------------------- */
        /*       HUFFMAN CODER SETUP	    */
        /* -------------------------------- */
 
        /* setup huffman coders */
        dc=malloc_HUFF(1,"main dc huff table"); /* huffman tables */
        ac=malloc_HUFF(1,"main ac huff table"); /* huffman tables */
        dump=malloc_PACKED(1,"main output bit stream"); /* output stream */
 
        /* open huffman pdf file */
        fp=fopen(pdf_fname,"rb");
        setup_huffman_tree (dc,dump,fp);
        setup_huffman_tree (ac,dump,fp);
        fclose(fp);
 
        /* wipe huffman pdf */
        if (type==wipe){
           fp=fopen(pdf_fname,"wb");
           wipe_huffman_pdf(dc,fp);
           wipe_huffman_pdf(ac,fp);
           fclose(fp);
           printf("wipeing huffman pdf file\n");
           exit(-1);
        }

	scales=5;
	c=malloc_WAVELET(1,"");
	setup_wavelet ("bi10_0.4.wav",c);

	image=malloc_IMAGE(1,"");
	col=malloc_IMAGE(1,"");
	/*
	fp=read_raw_header (input_fname,&(image->rows),&(image->columns),&dummy);
	*/
	fp=fopen(input_fname,"rb");
	image->columns=352;
	image->rows=288;
	col->columns=352;
	col->rows=144;

        /* calculate number of bits fro image */
        xt=bpp*(float)(image->columns*image->rows);
        printf("BITS %d\n",(int)xt);
        allocate_dump(dump,(int)(xt*10)); /*fudge size in bits for dump*/
		
	for (i=0;i<50;i++){
	    get_pgm_file(fp,image);
	    get_pgm_file(fp,col);
            write_pgm_file(image,"new.org");
	    dump->sym_mask=0x01;
	    dump->symbols=dump->symbols_top;
	    dump->symbols[0]=0;
	    dump->bits=0;
	    compress_image (image,c,type,mode,xt,dc,ac,scales);
            write_pgm_file(image,output_fname);
	    printf("frame=%d\n",i);
	}

	if (type==append || type==fix){
           fp=fopen(pdf_fname,"wb");
           save_huffman_pdf (dc,fp);
           save_huffman_pdf (ac,fp);
           fclose(fp);
           printf("appending huffman pdf to disk\n");
        }

        /* free memory used by coder */
        free_huffman_tree(dc);
        free_HUFF(dc);
        free_huffman_tree(ac);
        free_HUFF(ac);

}

