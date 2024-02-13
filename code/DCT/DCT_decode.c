/*##############################################################*/
/*			David Bethel				*/
/*		       Bath University 				*/
/*		      daveb@ee.bath.ac.uk			*/
/*		    DCT  Encoder Program			*/
/*		        + decoder  		                */
/*		  	   19/2/96				*/
/*##############################################################*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include "macro.h"
#include "arith.h"
#include "fastct.h"
#include "solution.h"

/* Structure Definitions */
/* Block definitions */
typedef struct block_data {
	int *pt; /* Block data */
	int x,y;
	int size; /* size of block - assumes square */
	struct block_data *next; 
} BLOCK;

/* Image struct containing image data and size of image */
typedef struct {
	int *pt; /* store of image before it is placed in blocks*/
	BLOCK *tree_top; /* pointer to the top of the linked 
				     list of image blocks*/
        COEFF_PLANES *data;	
	int first_dc;
	int columns;
	int rows;
	int greylevels;
} IMAGE;

/* stores loaded unquantised pdf data -> quantised pdfs also stored in SYMBOL */ 
typedef struct {
	int s_max;
	int s_min;
	int *pdf;
} PDF;


/* defines */
#define QUANT_RANGE 10
	
/* prototypes */
void write_pgm_file();
void allocate_dump();
void write_ppm_file();
int find_quantisation();
BLOCK *allocate_block_memory();


void write_ppm_file(image_y,image_u,image_v,fname)
IMAGE *image_y,*image_u,*image_v;
char *fname;
{
 
        FILE *fp;
        float R,G,B;
        float Y,U,V;
        int i,j,tmp;
         
 
        fp=fopen(fname,"wb");
 
        if(fp==0)
                {
                printf("\n Can't create file - Terminating program\n");
                exit(1);
                }
 
        /* write image stats + comments */
        fputs("P6",fp);
        fprintf(fp,"\n# Created by Wavelet Image Compression (daveb 96)#");
        fprintf(fp,"\n%d %d",image_y->columns,image_y->rows);
        fprintf(fp,"\n255\n");                          /* 255 graylevels */
 
        /* write image data */
        for (j=0;j<image_y->rows;j++)
            for (i=0;i<image_y->columns;i++){
                /* load up Y/U/V */
                Y=(float)image_y->pt[image_y->columns*j+i];
                U=(float)(image_u->pt[image_u->columns*(j/2)+(i/2)]-128);
                V=(float)(image_v->pt[image_v->columns*(j/2)+(i/2)]-128);
 
                /* convert to RGB */
                R=Y+0.000060*U+1.402581*V;
                G=Y-0.344369*U-0.714407*V;
                B=Y+1.773043*U-0.000130*V;
 
 
                /* save to file */
                tmp=limit((int)rint(R));
                putc(tmp,fp);
         
                tmp=limit((int)rint(G));
                putc(tmp,fp);
         
                tmp=limit((int)rint(B));
                putc(tmp,fp);
           }
 
        fclose(fp);
}

void write_pgm_file(image,fname)
IMAGE *image;
char *fname;
{
	int i;
	FILE *fp;
	
	unsigned char tmp;
 
        fp=fopen(fname,"wb");
 
        if(fp==0)
                {
                printf("\n Can't create file - Terminating program\n");
                exit(1);
                }
 
	/* write image stats + comments */
        fputs("P5",fp);         
        fprintf(fp,"\n# Created by Wavelet Image Compression (daveb 96)#");
        fprintf(fp,"\n%d %d",image->columns,image->rows);
        fprintf(fp,"\n255\n");                          /* 255 graylevels */
 
	/* write image data */
        for (i=0;i<image->rows*image->columns;i++){
            if (image->pt[i]>255)
               image->pt[i]=255;
            if (image->pt[i]<0)
               image->pt[i]=0;
	   
            putc(image->pt[i],fp);
	}

        fclose(fp);
} 

/* put blocks back ito an image for writing  */
reform_image(image,size)
IMAGE *image;
int size;
{
	int i,j;
	BLOCK *block;
	
	/* allocate memory for image */
        if (image->pt!=NULL)
           free(image->pt);
        image->pt=(int *)malloc(sizeof(int)*image->columns*image->rows);

	block=image->tree_top;

	while (block!=NULL){

	      /* load block into images*/
	      for (i=0;i<size;i++)
	          for (j=0;j<size;j++)
	              image->pt[image->columns*(j+block->y)+i+block->x]=
			       block->pt[size*j+i];

	      /* move pointer to next position */
	      block=block->next;
	}

}

free_blocks(image,size)
IMAGE *image;
int size;
{
        BLOCK *block,*next;
	
	block=image->tree_top;
	while (block!=NULL){
	      next=block->next;
	      free(block->pt);
	      free(block);
	      block=next;
	}
}

/* block image with fixed blank block size */
setup_blank_blocks(image,size)
IMAGE *image;
int size;
{
        int i,j,ii,jj;
        BLOCK *block,*last;

        last=NULL;
        for (j=0;j<image->rows;j+=size)
            for (i=0;i<image->columns;i+=size){
 
                /* allocate block memory */
                block=allocate_block_memory(size,i,j);
 
                /* load image into block */
                for (ii=0;ii<size;ii++)
                    for (jj=0;jj<size;jj++)
                        block->pt[size*jj+ii]=0;
 
                /* load top of tree into image  struct */
                if (i==0 && j==0)
                   image->tree_top=block;
                else
                    last->next=block; /* form link list if applicable */
 
                /* load last pointer for next pass */
                last=block;
 
            }
 
}

BLOCK *allocate_block_memory(size,x,y)
int size;
int x,y;
{

	BLOCK *pt;
	
	/* allocate memory for new block */
	pt=(BLOCK *)malloc(sizeof(BLOCK));

	pt->size=size;
	pt->x=x;
	pt->y=y;
	pt->next=NULL;
	
	/* allocate mem for section of image inside the block */
	pt->pt=(int *)malloc(sizeof(int)*size*size);

	return pt;

}

void idct_block(block,cosine)
BLOCK *block;
COSINE *cosine;
{
	int i,j;
	double *f;

	f=(double *)malloc(sizeof(double)*block->size);
	    
	/* DCT in j direction */
	for (i=0;i<block->size;i++){
	    /* load line into tmp varible */
	    for (j=0;j<block->size;j++)
	        f[j]=(double)block->pt[block->size*j+i];

	    /* dct line */
	    ifct_noscale(f,cosine,block->size);

	    /* load line back into blcok */
	    for (j=0;j<block->size;j++)
	        block->pt[block->size*j+i]=(int)rint(f[j]);
	}

	/* DCT in x direction */
	for (j=0;j<block->size;j++){
	    /* load line into tmp varible */
	    for (i=0;i<block->size;i++)
	        f[i]=(double)block->pt[block->size*j+i];

	    /* dct line */
	    ifct_noscale(f,cosine,block->size);

	    /* load line back into block  and scale by 2/NM*/
	    for (i=0;i<block->size;i++)
	        block->pt[block->size*j+i]=(int)rint(f[i])>>(cosine->m-1);
	}

	free(f);

}

void idct_image(image,cosine)
IMAGE *image;
COSINE *cosine;
{
	BLOCK *block;

	/* initalise pointer to top of blcok list */
	block=image->tree_top;

	/* pass through block list and dct each block */
	while (block!=NULL){
	      idct_block(block,cosine);
	      block=block->next;
	}

}

void unquantise_image(image,q_out,size)
IMAGE *image;
int *q_out;
int size;
{
        int i;
        BLOCK *block;
 
        for (i=0;i<size*size;i++){
            /* skip unused coefficients */
            if (q_out[i]==-1){
               block=image->tree_top;
               while (block!=NULL){
                     block->pt[i]=0;
                     block=block->next;
               } 
               continue;
            }
 
            /* scan image */
            block=image->tree_top;
            while (block!=NULL){
                  /* unquantise each coefficient inside blocks */
                  block->pt[i]*=q_out[i];
                  /* move to next block */
                  block=block->next;
            }
 
        }
 
}

void allocate_dump(dump,range,offset,n_bits)
PACKED *dump;
int range;
int offset;
int n_bits;
{

	dump->sym_mask=0x01;
	dump->symbols=(unsigned char *)malloc(sizeof(unsigned char)*(n_bits/8));
	dump->symbols_top=dump->symbols;
	
	/* set up arith symbol stream */
	dump->s=(SYMBOL *)malloc(sizeof(SYMBOL));
	(dump->s)->no_symbols=range;
	(dump->s)->end_of_stream=range; /* may not be needed */
	(dump->s)->offset=offset; /* may not be needed */

}


load_pdf(loaded,size)
PDF *loaded;
int size;
{
	FILE *fp;
	SYMBOL *s;
	int i,j,k,s_max,s_min,tmp;
	int nitems,cnt;
	float *prob;

	/* open input file */
	fp=fopen("pdf.all","rb");

	for (i=0;i<size*size;i++){
	    /* load pdf from disk */
	    fscanf(fp,"%d\t%d\n",&s_min,&s_max);

	    loaded[i].s_min=s_min;    
	    loaded[i].s_max=s_max;    

	    loaded[i].pdf=(int *)malloc(sizeof(int)*(s_max-s_min+1));
	    for (j=s_min;j<=s_max;j++){
		fscanf(fp,"%d\n",&tmp);
	        loaded[i].pdf[j-s_min]=tmp;    
	    }
	}
}

apply_pdf(dump,loaded,q_out,max,min)
PACKED *dump;
PDF loaded;
int q_out;
int max,min;
{
        SYMBOL *s;
        int j,k,s_max,s_min,tmp;
        int nitems,cnt;
        float *prob;

	s_min=loaded.s_min;
        s_max=loaded.s_max;

	/* load up temp pointer */
	s=dump->s;    

	/* clear probabilities */
	prob=(float *)malloc(sizeof(float)*(max-min+1));
	for (j=min;j<=max;j++)
		prob[j-min]=0;

 	 /* convert pdf to quantised pdf */
         nitems=0;
	 /* moveing out from centre ->bottom first */
	 for (j=0;j>=min;j--){
	     /* check for symbols larger than model */
	     if (s_min<=(j-1)*q_out)
		for (k=0;k>-q_out;k--)
		    prob[j-min]+=(float)(loaded.pdf[j*q_out-s_min+k]); 
	     else 			
		 prob[j-min]=1.0;
	     nitems+=(int)prob[j-min];	
	 }

	 for (j=0;j<=max;j++){
             /* check for symbols larger than model */
             if (s_max>=(j+1)*q_out)
                for (k=0;k<q_out;k++)
                    prob[j-min]+=(float)(loaded.pdf[j*q_out-s_min+k]);
             else             
                 prob[j-min]=1.0;
             nitems+=(int)prob[j-min];
        }

	/* scale probablity */
	for (j=min;j<=max;j++){
	    prob[j-min]/=(float)nitems; 
        }

	for (j=0;j<(s->no_symbols);j++){
            cnt=0;
            s->totals[j][0]=0;
            for (k=0;k<(s->no_symbols+1);k++){
                /* work out context from pdf */
                if (k<s->no_symbols)
                   tmp=(int)rint(((prob[j]*prob[k])*(float)nitems));
                else
                    tmp=0;
 
                /* update contexts data */
                if (tmp==0)
                   cnt++;
                else
                    cnt+=tmp;
                s->totals[j][k+1]=cnt;
            }
        }
	   
   	free(prob);


}

void read_compressed_file(fp,image,dump,header,loaded,min,max,q_out,size)
FILE *fp;
IMAGE *image;
PACKED **dump;
PACKED *header;
PDF *loaded;
int *min,*max;
int *q_out;
int size;
{
        int i,j,header_size,*in_size,min_min,max_max;
        int tmp;
        unsigned char *in;
 
	
        /* also allocates mem for images */
        /* Read image columns to disk */
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        image->columns=tmp;
 
        /* Read image rows to disk */
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        image->rows=tmp;
	
	setup_blank_blocks(image,size);

        /* Read dc max from disk  - assumes 2 bytes*/
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
	max[0]=tmp;
	min[0]=0;

	/* Read min_min from disk  - assumes 2 bytes*/
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
	min_min=-tmp;

	/* Read max_max from disk  - assumes 2 bytes*/
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
	max_max=tmp;

	/* Read header from disk  - assumes 2 bytes*/
        tmp=getc(fp);
        tmp<<=8;    
        tmp|=getc(fp);   
        header_size=tmp; 


 	allocate_dump(header,max_max-min_min+1,-min_min,header_size*8);

	/* allocate space for input compressed image */
	in=header->symbols_top;
	for (i=0;i<header_size;i++)
	    in[i]=getc(fp);  

	/* unpack header */
        initialize_arithmetic_decoder(header);
 
        /* read in quantisation tables */
        for (i=0;i<size*size;i++){
            tmp=Expand_symbol(header);
            if (tmp==0)
               q_out[i]=-1;
            else
                q_out[i]=tmp;
        }
 
        /* read in min and max */
        for (i=1;i<size*size;i++)
            if (q_out[i]!=-1){
               min[i]=Expand_symbol(header);
               max[i]=Expand_symbol(header);
            }

	in_size=(int *)malloc(sizeof(int)*size*size);
	/* read size of streams */
	for (i=0;i<size*size;i++)
	    if (q_out[i]!=-1){
	       /* allocate stream */
	       dump[i]=(PACKED *)malloc(sizeof(PACKED));

	       /* load in size of stream */
	       tmp=getc(fp);
               tmp<<=8;
               tmp|=getc(fp);
               in_size[i]=tmp;
 	       allocate_dump(dump[i],max[i]-min[i]+1,-min[i],in_size[i]*8);
	    }
	/* read streams */
	for (i=0;i<size*size;i++)
	    if (q_out[i]!=-1){
	       in=dump[i]->symbols_top;
               for (j=0;j<in_size[i];j++)
                   in[j]=getc(fp);
	    }
	free(in_size);
 
}

unpack_streams(image,dump,loaded,min,max,q_out,size)
IMAGE *image;
PACKED **dump;
PDF *loaded;
int *min,*max;
int *q_out;
int size;
{
	int i,tmp,last;
        BLOCK *block;

	/* unpack coefficent streams */
        for (i=0;i<size*size;i++)
	    if (q_out[i]!=-1){
	       /* setup arith coder with right pdf */
	       initialize_arithmetic_decoder(dump[i]);
	       apply_pdf(dump[i],loaded[i],q_out[i],max[i],min[i]);

               /* scan image */
               block=image->tree_top;
               while (block!=NULL){
                     block->pt[i]=Expand_symbol(dump[i]);
                     /* move to next block */
                     block=block->next;
               } 
            }

}

dct_decompress(image,fp,loaded,cosine,size)
IMAGE *image;
FILE *fp;
PDF *loaded;
COSINE *cosine;
int size;
{

	int i,j;
        int *max,*min;
        int *q_out;
        float *x_out;
        PACKED **dump,*header;

	/* alloccate memoey for quantisation */
	q_out=(int *)malloc(sizeof(int)*size*size);
	max=(int *)malloc(sizeof(int)*size*size);
	min=(int *)malloc(sizeof(int)*size*size);

	/* setup header stream */
        header=(PACKED *)malloc(sizeof(PACKED));
        dump=(PACKED **)malloc(sizeof(PACKED *)*size*size);

	read_compressed_file(fp,image,dump,header,loaded,min,max,q_out,size);

	/* decodes arithmetic coded streams */ 
	unpack_streams(image,dump,loaded,min,max,q_out,size);

	/* restore DCT coefficients in image */
	unquantise_image(image,q_out,size);

	idct_image(image,cosine);

	reform_image(image,size);
	
	/* clean things  */
        free(header->s);
        free(header);
        for (i=0;i<size*size;i++){
            if (q_out[i]!=-1){
               free(dump[i]->s->totals);
               free(dump[i]->s->pdf);
               free(dump[i]->s);
               free(dump[i]->symbols_top);
               free(dump[i]);
            }
        }
        free(dump);
        free(q_out);
        free(min);
        free(max);

}

main(argc,argv)
int argc;
char *argv[];
{
	int i,j,scales,colour,pgm;
        int n_bits_y,n_bits_u,n_bits_v;
	IMAGE *image_y,*image_u,*image_v;
        PDF *loaded;
        int size,no_quant=0;
        float xt;
        FILE *fp;
        COSINE *cosine;
        char cn[250];


        if (argc<3){
           printf("%s:
        <input> - no extention -> gets .bct file
        <output> - no extention -> outputs .y .u .v or .raw  file
        <cppm/cpgm/y> - ip / op type \n",argv[0]);
           exit(-1);
        }

        size=16;

	/* allocate mem */
        cosine=(COSINE *)malloc(sizeof(COSINE));
        loaded=(PDF *)malloc(sizeof(PDF)*size*size);

        /* load pdfs from disk */
        load_pdf(loaded,size);

        /* colour = 1 for colour pics */
        /* pgm = 1 for YUV loadin / out */

        if (argc>=4){
           if (strcmp(argv[3],"cppm")==0){
              colour=1;
              pgm=0;
              printf("Colour DCT Codec:raw\n");
           }
           if (strcmp(argv[3],"cpgm")==0){
              colour=1;
              pgm=1;
              printf("Colour DCT Codec:y u v\n");
           }
           if (strcmp(argv[3],"y")==0){
              colour=0;
              pgm=1;
              printf("Greyscale DCT Codec\n");
           }
        }
        else {
             colour=0;
             pgm=1;
             printf("Greyscale DCT Codec\n");
        }
        
        /* make space for images */
        image_y=(IMAGE *)malloc(sizeof(IMAGE));
        image_y->pt=NULL;
        if (colour){
           image_u=(IMAGE *)malloc(sizeof(IMAGE));
           image_u->pt=NULL;
           image_v=(IMAGE *)malloc(sizeof(IMAGE));
           image_v->pt=NULL;
        }
 
	/* decompress image */
        sprintf(cn,"%s.bct",argv[1]);
        fp=fopen(cn,"rb");
        dct_decompress(image_y,fp,loaded,cosine,size);
        if (colour){
           dct_decompress(image_u,fp,loaded,cosine,size);
           dct_decompress(image_v,fp,loaded,cosine,size);
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

}

