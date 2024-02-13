/*##############################################################*/
/*		Test of wavelet coder 				*/
/*##############################################################*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "code.h" 
#include <malloc.h>

void compress_image
(IMAGE *image,
 WAVELET *c,
 float xt,
 int scales)
{
	int i,size,*q_out=NULL,accuracy;
	float *x_out;
	double *entropy;

	for (i=0;i<image->columns*image->rows;i++)
	    image->pt[i]<<=5;

	wavelet_filter_image (image,scales);
        setup_blocks(image,scales);
	accuracy=1000; /* -+ 1000 bits */
        size=(1<<(scales))*(1<<(scales));
	q_out=malloc_int(size,"");
 
        /* aplly wide ban quantisation to generate quantiation tables */
        form_quantisation_tables(image,size);
        quantise_coefficients(image,size);
 
        /* alloccate memoey for output */
        x_out=malloc_float(size,"compress");

        /* solve problem to within 1000 bits */
        solve(image->data,x_out,xt,1000.0,size);
 
        /* Now need to covert bits allocated to each coefficient -> quantisation */
        convert_bits_to_quant(image,x_out,q_out,size);

	entropy=(double *)malloc(size*sizeof(double));	
 	printf("%f\n",(quantise_image (image,q_out,entropy,size))/(float)(image->rows*image->columns));
	
	unquantise_image (image,q_out,size);

	reform_image (image,scales);

        for (i=0;i<image->columns*image->rows;i++)
            image->pt[i]>>=5; 

   	wavelet_inv_filter_image (image,scales);

	free_float(x_out);
	free(entropy);
	free_blocked_image (image);

}

void main
(int argc,
 char **argv)
{
	int last,i,scales;
	IMAGE *image;
	WAVELET *c;
	int dummy;
	FILE *fp;
	float xt;
        char input_fname[256],output_fname[256],pdf_fname[256],quant_fname[256];
        float bpp;

        /* set defaults */
        sprintf(output_fname,"new.y");
        sprintf(pdf_fname,"pdf.all");
        sprintf(quant_fname,"q_out");
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
 
            /* unknown option */
            if (!strncmp(argv[i],"-",1)){
               printf("Unknown option\n");
               exit(-1);
            }
        }
 
        /* warn if wrong input length */
        if (last+2>=argc){
           printf("%s:
        [-o <output>]   default: new.y
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
 
	/* setup the wavelet filters */
	scales=3;
	c=malloc_WAVELET(1,"");

	/* setup the image */
	image=malloc_IMAGE(1,"");

	fp=read_raw_header (input_fname,&(image->rows),&(image->columns),&dummy);

        /* calculate number of bits fro image */
        xt=bpp*(float)(image->columns*image->rows);
		
	get_pgm_file(fp,image);
        write_pgm_file(image,"new.org");
	compress_image (image,c,xt,scales);
        write_pgm_file(image,output_fname);


}
