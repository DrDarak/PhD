/*##############################################################*/
/*			David Bethel				*/
/*		       Bath University 				*/
/*		      daveb@ee.bath.ac.uk			*/
/*		    DCT  Encoder Program			*/
/*		  	   19/2/96				*/
/*		finished on 29/3/96				*/
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
void get_pgm_file();
void write_pgm_file();
void allocate_dump();
void get_ppm_file();
void write_ppm_file();
int find_quantisation();
BLOCK *allocate_block_memory();

/* programes */
void get_ppm_file(image_y,image_u,image_v,fname)
IMAGE *image_y,*image_u,*image_v;
char *fname;
{
 
        /* Reads colour image in Raw format*/
        FILE *fp;
        int i,j,tmp;
        char dummy[256];
        float R,G,B;
        float Y,U,V;
 

        /* open image file */
        fp=fopen(fname,"r");
        if (fp==0){
           printf("\n File not found - Terminating program\n");
           exit(1);
        }

        /* remove P6 line */
        fgets(dummy,255,fp);

        /* remove comment lines */
        do {
           fgets(dummy,255,fp);
        } while (strncmp(dummy,"#",1)==0);

        /* read in image stats */          
        sscanf(dummy,"%d %d",&(image_y->columns),&(image_y->rows));
        fscanf(fp,"%d\n",&(image_y->greylevels));
 
        /* write stats into v/u images - sub sampled */
        image_u->columns=image_y->columns/2;
        image_u->rows=image_y->rows/2;
        image_u->greylevels=image_y->greylevels;
 
        image_v->columns=image_y->columns/2;
        image_v->rows=image_y->rows/2;
        image_v->greylevels=image_y->greylevels;

        /* allocate memory for y image */        
        if (image_y->pt!=NULL)
           free(image_y->pt);
        image_y->pt=(int *)malloc(sizeof(int)*image_y->columns*image_y->rows);
 
        /* allocate memory for u image */
        if (image_u->pt!=NULL)
           free(image_u->pt);
        image_u->pt=(int *)malloc(sizeof(int)*image_u->columns*image_u->rows);
 
        /* allocate memory for v image */
        if (image_v->pt!=NULL)
           free(image_v->pt);
        image_v->pt=(int *)malloc(sizeof(int)*image_v->columns*image_v->rows);

        /* read in image data -> file is 3 times as large as image (RGB)*/     
        for (j=0;j<image_y->rows;j++)
            for (i=0;i<image_y->columns;i++){
                /* read in Red / Blue / Green */
                G=(float)getc(fp);
                B=(float)getc(fp);
                R=(float)getc(fp);
 
                /* convert to YUV */
                Y= 0.2990*R+0.5870*G+0.1140*B;
                U=-0.1686*R-0.3311*G+0.4997*B;
                V= 0.4998*R-0.4185*G-0.0813*B;
 
                /* load into image files */
                image_y->pt[image_y->columns*j+i]=(int)rint(Y);
                image_u->pt[image_u->columns*(j/2)+(i/2)]=(int)rint(U+128.0);
                image_v->pt[image_v->columns*(j/2)+(i/2)]=(int)rint(V+128.0);
        }
 
        fclose(fp);
}

void get_pgm_file(image,fname)
IMAGE *image;
char *fname;
{
	/* Reads image in Raw format*/
	FILE *fp;
	int i,j,tmp;
	char dummy[256];

	/* open image file */
	fp=fopen(fname,"rb");
	if (fp==0){
	   printf("\n File not found - Terminating program\n");
	   exit(1);
	}

	/* remove P5 line */
	fgets(dummy,255,fp);

	/* remove comment lines */
	do {
	   fgets(dummy,255,fp);
	}while (strncmp(dummy,"#",1)==0);

	/* read in image stats */
	sscanf(dummy,"%d %d",&(image->columns),&(image->rows));
	fscanf(fp,"%d\n",&(image->greylevels));
	
	/* allocate memory for image */
        if (image->pt!=NULL)
           free(image->pt);
        image->pt=(int *)malloc(sizeof(int)*image->columns*image->rows);
	
	/* read in image data */
        for (i=0;i<image->rows*image->columns;i++)
	    image->pt[i]=getc(fp);

	fclose(fp);
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


/* block image with fixed block size */
setup_blocks(image,size)
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
		        block->pt[size*jj+ii]=image->pt[image->columns*(j+jj)+i+ii];

		/* load top of tree into image  struct */
	        if (i==0 && j==0)
	           image->tree_top=block;
		else
		    last->next=block; /* form link list if applicable */

		/* load last pointer for next pass */
		last=block;

	    }	

	/* free tempory image */
	free(image->pt);

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

void dct_block(block,cosine)
BLOCK *block;
COSINE *cosine;
{
	int i,j;
	double *f;

	f=(double *)malloc(sizeof(double)*block->size);

	/* DCT in x direction */
	for (j=0;j<block->size;j++){
	    /* load line into tmp varible */
	    for (i=0;i<block->size;i++)
	        f[i]=(double)block->pt[block->size*j+i];

	    /* dct line */
	    fct_noscale(f,cosine,block->size);

	    /* load line back into blcok */
	    for (i=0;i<block->size;i++)
	        block->pt[block->size*j+i]=(int)rint(f[i]);
	}
	    
	/* DCT in j direction */
	for (i=0;i<block->size;i++){
	    /* load line into tmp varible */
	    for (j=0;j<block->size;j++)
	        f[j]=(double)block->pt[block->size*j+i];

	    /* dct line */
	    fct_noscale(f,cosine,block->size);

	    /* load line back into blcok  and scale by 2/NM*/
	    for (j=0;j<block->size;j++)
	        block->pt[block->size*j+i]=(int)rint(f[j])>>(cosine->m-1);
	}

	free(f);

}

void dct_image(image,cosine)
IMAGE *image;
COSINE *cosine;
{
	BLOCK *block;

	/* initalise pointer to top of blcok list */
	block=image->tree_top;

	/* pass through block list and dct each block */
	while (block!=NULL){
	      dct_block(block,cosine);
	      block=block->next;
	}

}

void allocate_mem_coeff_planes(pt,nitems)
COEFF_PLANES *pt;
int nitems;
{

        pt->e=(float*)malloc(sizeof(float)*nitems);
        pt->x=(float*)malloc(sizeof(float)*nitems);
        pt->de=(float*)malloc(sizeof(float)*nitems-1);
        pt->dx=(float*)malloc(sizeof(float)*nitems-1);
        pt->q=(int *)malloc(sizeof(int)*nitems);
        pt->nitems=nitems;

}

void quantise_dct(image,size)
IMAGE *image;
int size;
{
	int i,j,max,nitems,last,k;
	float factor;
	int q[QUANT_RANGE];
	COEFF_PLANES *pt;
	
	/* allocate memoery for quantisation data */
        image->data=(COEFF_PLANES *)malloc(sizeof(COEFF_PLANES)*size*size);	
	
	/* loop through all coefficent sets */ 
	for (i=0;i<size*size;i++){

	    /* find max coefficient in set */
 	    max=max_coeff(image->tree_top,i);
	    

	    /* calcualte quant scaling factor */
	    factor=log((float)max)/log(2);
	    factor/=(float)QUANT_RANGE;
	
	    /* if the coefficent > quantisation range the work out quant step */
	    if (max>QUANT_RANGE)
	       for (j=QUANT_RANGE;j>0;j--) 
	           q[QUANT_RANGE-j]=(int)ceil(pow(2,factor*(float)j));
	    else
	        for (j=QUANT_RANGE;j>0;j--) 
	            q[QUANT_RANGE-j]=(int)j;

	    /* search through items and check reportion etc*/
	    last=max;
	    nitems=1;
	    for (j=0;j<QUANT_RANGE;j++)
	        if (q[j]>=last || q[j]<1)
		   q[j]=-1;
		else{ 
		     nitems++;
		     last=q[j];
		}

	    /* check if stream is terminated with no quantisation */
	    if (last>1)
	       nitems++;

	    /* allocate memory for coeff plane */
	    pt=(image->data)+i;
            allocate_mem_coeff_planes(pt,nitems);

	    /* load approprate items into list */
	    pt->q[0]=max+1;  /* max quant is set to max level +1 to ensure 0 entropy */
	    k=1;
	    for (j=0;j<QUANT_RANGE;j++){
		if (q[j]!=-1)
		   pt->q[k++]=q[j];
	    }
	    if (last>1)
	       pt->q[k]=1;
		
	    /* now apply these quantisation tables to the data */
	    quantise_coeff(image,pt,i,max);

	    /* load max in coeff struct */
            pt->max=max;

	}

}

quantise_coeff(image,pt,coeff,max)
IMAGE *image;
COEFF_PLANES *pt;
int coeff;
int max;
{
	int i,j,k,last;
	double entropy,last_entropy;
	int error,q,diff;
	int *bin_bottom,*bins,nitems;
	BLOCK *block;
	
	/* since min quantisation is 1 => nitems is 2*max (accounting for -/+)*/
	bin_bottom=(int *)malloc(sizeof(int)*(2*max+1));
	bins=bin_bottom+max;

	/* scan through quantisations */
	i=0;
	last_entropy=-1;
	while (i<pt->nitems){
	    /* define top of block list */
	    block=image->tree_top;

	    /* clear bins */
	    for (j=0;j<2*max+1;j++)
	        bin_bottom[j]=0;

	    /* clear error */
	    error=0;

	    /* load up quantiastion value */
	    q=pt->q[i];

	    /* scan through block list */
	    nitems=0;
	    while (block!=NULL){
	          diff=(int)(block->pt[coeff]/q); 
	          bins[diff]++; 			
	          diff=(block->pt[coeff])-diff*q;
		  diff*=diff;
	          error+=diff;
		  nitems++;
	          block=block->next; 
	    }
	    
	    /* calculate bits / pixel */
	    entropy=0;
	    for (j=-max;j<=max;j++)
	        if (bins[j]!=0)
                   entropy-=(double)bins[j]*log10((double)bins[j]/(double)nitems);                 

	    /* ensure no repeated points in x */
	    if (last_entropy>=entropy){
	       /* shift quantisation values down */
	       for (j=i;j<pt->nitems;j++)
	     	   pt->q[j-1]=pt->q[j]; 	
	       /* decrease size of list and bring back list pt */
	       pt->nitems--;
	       i--;
	    }

	    pt->x[i]=entropy/log10(2.0);
	    pt->e[i]=(float)error;

	    /* finish loop */
	    i++;
	    last_entropy=entropy;
	}

	free(bin_bottom);
}


int max_coeff(block,coeff)
BLOCK *block;
int coeff;
{
	int max;
	
	max=0;
	/* loop through linked list */
	while (block!=NULL){
	      if (myabs(block->pt[coeff])>max)
	         max=myabs(block->pt[coeff]);
	      block=block->next;
	}

	return max;

}

void convert_bits_to_quant(image,x_out,q_out,size)
IMAGE *image;
float *x_out;
int *q_out;
int size;
{
	int i;

	for (i=0;i<size*size;i++){
	    /* skip loop is coefficent is not used */
	    if (x_out[i]==0.0){
	       q_out[i]=-1;
	       continue;
	    }
	    /* find quantisation for each coefficient */
	    q_out[i]=find_quantisation(x_out[i],(image->data)+i);
	}
}

int find_quantisation(x,pt)
float x;
COEFF_PLANES *pt;
{
	int i;
	float q;
	
	for (i=0;i<pt->nitems-1;i++)
	    if (pt->x[i]<=x && pt->x[i+1]>x)
	       break;

	/* linearly predict q */
	q=(float)(pt->q[i])+(x-pt->x[i])*(float)(pt->q[i+1]-pt->q[i])
	                               /(pt->x[i+1]-pt->x[i]);	
	/* round it to the nearest int */
	q=rint(q);

	return (int)q;	

}
void quantise_image(image,q_out,entropy,size)
IMAGE *image;
int *q_out;
double *entropy;
int size;
{
	int i,j,tmp,max;
	BLOCK *block;
	double total_entropy;
	int *bin_bottom,*bins,nitems;

        total_entropy=0;
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

	    max=(image->data+i)->max;
	    /* since min quantisation is 1 => nitems is 2*max (accounting for -/+)*/
            bin_bottom=(int *)malloc(sizeof(int)*(2*max+1));
            bins=bin_bottom+max;

	    /* clear bins */
	    for (j=0;j<2*max+1;j++)
	        bin_bottom[j]=0;
			
	    /* scan image */
	    block=image->tree_top;
	    nitems=0;
	    while (block!=NULL){
	          /* quantise each coefficient inside blocks */
                  block->pt[i]/=q_out[i];
		  bins[block->pt[i]]++;
		  nitems++;
	          /* move to next block */
                  block=block->next;
            }

            /* calculate bits / pixel */
	    entropy[i]=0;
            for (j=-max;j<=max;j++)
                if (bins[j]!=0)
                   entropy[i]-=(double)bins[j]*log10((double)bins[j]/(double)nitems);
	    free(bin_bottom);
            entropy[i]/=log10(2.0);
            total_entropy+=entropy[i];

	}

	printf("entpy=%f\n",entropy);

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

int find_no_ac_symbols(image,q_out,x_out,size,low_high)
IMAGE *image;
int *q_out;
float *x_out;
int size;
int low_high; /* 0 - low    1 - high */
{
	int i,max;
	float min;
	COEFF_PLANES *pt;

	min=(float)(image->rows*image->columns)/(float)(size*size);
	max=0;
	pt=image->data;
	for (i=1;i<size*size;i++){
	    if (low_high==1 && x_out[i]<min)
	       continue;

	    if (low_high==0 && x_out[i]>=min)
	       continue;

	    if (max<(int)(pt[i].max/q_out[i]))
	       max=(int)(pt[i].max/q_out[i]);
	}

	printf("%d\n",max);
	return max;

}
void pack_image(image,q_out,dump,size)
IMAGE *image;
int *q_out;
PACKED **dump;
int size;
{
        int i,j;
        float min;
        BLOCK *block;

        /* send coefficients to arith */
        for (i=0;i<size*size;i++){
            /* skip unused coefficients */
            if (q_out[i]!=-1){
               /* scan image */
               block=image->tree_top;
               while (block!=NULL){
                     Compress_symbol(dump[i],block->pt[i]);
                     /* move to next block */
                     block=block->next;
               } 
               /* end stream */
               Compress_symbol(dump[i],(dump[i]->s)->no_symbols);
            }
        }

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

ammend_pdf(dump,loaded,q_out,max,min,nitems,size)
PACKED **dump;
PDF *loaded;
int *q_out;
int *max,*min;
int nitems;
int size;
{
	int i,j,k,a,b;
	int a_max,a_min,tmp;
	int *pdf;
	SYMBOL *s;
	float scale;
 
	for (i=0;i<size*size;i++)
	    if (q_out[i]!=-1){
	       s=dump[i]->s;    

	       /* find true scales */
	       if (min[i]*q_out[i]<loaded[i].s_min)
	          a_min=min[i]*q_out[i];
	       else
	           a_min=loaded[i].s_min;

	       if (max[i]*q_out[i]>loaded[i].s_max)
                  a_max=max[i]*q_out[i];
               else 
                   a_max=loaded[i].s_max; 

	       /* malloc space for new pdf */
	       pdf=(int *)malloc(sizeof(int)*(a_max-a_min+q_out[i]));
		
	       /* Next calcluate scale for ammending pdfs */
	       tmp=0;
	       for (j=0;j<(s->no_symbols);j++)
		   tmp+=s->pdf[j];
	       scale=(float)nitems/(float)(tmp*q_out[i]);

	       /* first fill in pdf data from present image */
	       for (j=min[i];j<=max[i];j++){
		   tmp=0;
	           for (k=0;k<q_out[i];k++){
	               pdf[j*q_out[i]+k-a_min]=(int)ceil(scale*(float)(s->pdf[j-min[i]]));
		       tmp+=pdf[j*q_out[i]+k-a_min];
		   }
		}

		/* Now fill in data from previous images(from disk) */
	        for (j=loaded[i].s_min;j<=loaded[i].s_max;j++)
                    pdf[j-a_min]+=loaded[i].pdf[j-loaded[i].s_min];

		/* now load the new (ammended) pdf into the pdf 'file' */
		free(loaded[i].pdf);
	        loaded[i].pdf=pdf;
		loaded[i].s_max=a_max;
		loaded[i].s_min=a_min;
	
	}

}	

wipe_pdf(loaded,size)
PDF *loaded;
int size;
{

	int i,j;

	for (i=0;i<size*size;i++)
	    for (j=loaded[i].s_min;j<=loaded[i].s_max;j++)
	        loaded[i].pdf[j-loaded[i].s_min]=0;    

}

save_pdf(loaded,size)
PDF *loaded;
int size;
{
     	FILE *fp;
        int i,j;
        float *prob;
 
        /* open input file */
        fp=fopen("pdf.all","wb");
 
        for (i=0;i<size*size;i++){
            /* save  pdf to disk */
            fprintf(fp,"%d\t%d\n",loaded[i].s_min,loaded[i].s_max);
            for (j=loaded[i].s_min;j<=loaded[i].s_max;j++)
                fprintf(fp,"%d\n",loaded[i].pdf[j-loaded[i].s_min]);
 
        }    
        fclose(fp); 
 
}

void max_min_quant_coeff(block,coeff,min,max)
BLOCK *block;
int coeff;
int *min,*max;
{
        *max=0;
        *min=0;
        /* loop through linked list */
        while (block!=NULL){
              if (block->pt[coeff]>(*max))
                 *max=block->pt[coeff];
              if (block->pt[coeff]<(*min))
                 *min=block->pt[coeff];
              block=block->next;
        }

}
void write_compressed_file(fp,image,dump,header,max_dc,min_min,max_max,q_out,size)
FILE *fp;
IMAGE *image;
PACKED **dump;
PACKED *header;
int max_dc;
int min_min,max_max;
int *q_out;
int size;
{

        int i,j,out_size,n;
        unsigned char tmp;
        unsigned char *out;

        /* write image columns to disk */
        tmp=(unsigned char)(image->columns>>8);
        putc(tmp,fp);
        tmp=(unsigned char)(image->columns);
        putc(tmp,fp);

        /* write image rows to disk */
        tmp=(unsigned char)(image->rows>>8);
        putc(tmp,fp);
        tmp=(unsigned char)(image->rows);
        putc(tmp,fp);

	/* write max dc to disk  - assumes 2 bytes*/
        tmp=(unsigned char)(max_dc>>8);
        putc(tmp,fp);    
        tmp=(unsigned char)(max_dc);   
        putc(tmp,fp);    

	/* write min_min to disk  - assumes 2 bytes*/
	min_min=myabs(min_min); /* assume negative */
        tmp=(unsigned char)(min_min>>8);
        putc(tmp,fp);
        tmp=(unsigned char)(min_min);
        putc(tmp,fp);

	/* write max_max to disk  - assumes 2 bytes*/ 
        tmp=(unsigned char)(max_max>>8); 
        putc(tmp,fp); 
        tmp=(unsigned char)(max_max); 
        putc(tmp,fp); 

	out_size=header->symbols-header->symbols_top+1;
	/* write headersize to disk  - assumes 2 bytes*/
        tmp=(unsigned char)(out_size>>8);
        putc(tmp,fp);
        tmp=(unsigned char)(out_size);
        putc(tmp,fp);


        /* write header stream to disk */
        out=header->symbols_top;
        for (i=0;i<out_size;i++)
            putc(out[i],fp);
 
	/* output stream sizes */
        for (i=0;i<size*size;i++)
	    if (q_out[i]!=-1){
	       out_size=dump[i]->symbols-dump[i]->symbols_top+1;
	       /* output bytes */
	       tmp=(unsigned char)(out_size>>8); 
               putc(tmp,fp); 
               tmp=(unsigned char)(out_size); 
               putc(tmp,fp); 

	    }
        /* write coeff streams to disk */
        for (i=0;i<size*size;i++)
	    if (q_out[i]!=-1){
               out=dump[i]->symbols_top;
	       out_size=dump[i]->symbols-dump[i]->symbols_top+1;
	       for (j=0;j<out_size;j++)
                   putc(out[j],fp);
	       
	    }

}
 
dct_compress(image,fp,loaded,cosine,xt,size,no_quant)
IMAGE *image;
FILE *fp;
PDF *loaded;
COSINE *cosine;
float xt;
int size;
int no_quant;
{
	int i,j,tmp,last;
	int bits,n_bits,*q_out,nitems;
	float *x_out;
	PACKED **dump,*header;
	int *max,*min,max_max,min_min,range,offset;
	double *entropy;

	/* change image into a set of blocks */
	setup_blocks(image,size);

	/* converts blocks into blocks of DCT coefficents */
	dct_image(image,cosine);

	/* aplly wide ban quantisation to generate quantiation tables */
	quantise_dct(image,size);
	
	/* alloccate memoey for output */
	x_out=(float *)malloc(sizeof(float)*size*size);

	/* solve problem to within 1000 bits */
	solve(image->data,x_out,xt,1000.0,size*size);

	/* alloccate memoey for quantisation */
	q_out=(int *)malloc(sizeof(int)*size*size);

	/* Now need to covert bits allocated to each coefficient -> quantisation */
	convert_bits_to_quant(image,x_out,q_out,size);

	/* saveing pdf if called for */
	if (no_quant)
	   for (i=0;i<size*size;i++){
	       if (i==0)
	          q_out[i]=4;
	       else 
		   q_out[i]=2;
	   }

	/* quantise DCT coefficients in image */
	entropy=(double *)malloc(sizeof(double)*size*size);
	quantise_image(image,q_out,entropy,size);

	/* arithmetic code image data */ 
	dump=(PACKED **)malloc(sizeof(PACKED *)*size*size);

	/* allocate memoery for dump */
	/* dc is for dc codes */
	max=(int *)malloc(sizeof(int)*size*size);
	min=(int *)malloc(sizeof(int)*size*size);
	max_max=0;
	min_min=0;
	for (i=0;i<size*size;i++)
	    if (q_out[i]!=-1){
	
	       /* check for max/min */
	       if (q_out[i]>max_max)
		  max_max=q_out[i];
	       if (q_out[i]<min_min)
		  min_min=q_out[i];
	      
	       /* set up symbol dumps */
	       dump[i]=(PACKED *)malloc(sizeof(PACKED));
	       max_min_quant_coeff(image->tree_top,i,&min[i],&max[i]);

	       /* work out range on offset */
	       offset=-1*min[i];
	       range=max[i]-min[i]+1;
	       allocate_dump(dump[i],range,offset,(int)(entropy[i]*2+1000));

	       /* check for max/min */
	       if (i!=0){
	          if (max[i]>max_max)
		     max_max=max[i];
	          if (min[i]<min_min)
		     min_min=min[i];
	       }
	    }
	
	/* setup header stream */
	header=(PACKED *)malloc(sizeof(PACKED));
	allocate_dump(header,max_max-min_min+1,-1*min_min,(int)8000);
	
	/* setup arith coders */
	for (i=0;i<size*size;i++)
	    if (q_out[i]!=-1)
	       initialize_arithmetic_encoder(dump[i]->s);
	initialize_arithmetic_encoder(header->s);

	/* form PDF tables */
	for (i=0;i<size*size;i++)
	    if (q_out[i]!=-1)
	       apply_pdf(dump[i],loaded[i],q_out[i],max[i],min[i]);
	
	/* pack coefficents */
	pack_image(image,q_out,dump,size);

	/* pack header */
	/* pack quantisation */
	for (i=0;i<size*size;i++){
	    if (q_out[i]!=-1){
	       tmp=q_out[i];
	    }
	    else
		tmp=0;
	    Compress_symbol(header,tmp); 
	}

	/* pack min/max */
	for (i=1;i<size*size;i++)
	    if (q_out[i]!=-1){
	       Compress_symbol(header,min[i]); 
	       Compress_symbol(header,max[i]); 
	    }
	Compress_symbol(header,header->s->no_symbols); 

	/* ammend and save pdfs if on a no_quant run only */
	nitems=(image->columns*image->rows)/size/size;
	if (no_quant){
	   ammend_pdf(dump,loaded,q_out,max,min,nitems,size);
	   save_pdf(loaded,size);
	   printf("PDFs ammended\n");
	   exit(-1);
	}
	
	/* write compressed file to disk */
	write_compressed_file(fp,image,dump,header,max[0],min_min,max_max,q_out,size);

	/* work out compression of system */
	n_bits=8*(header->symbols-header->symbols_top);
	printf("header=%d\n",n_bits);
	for (i=0;i<size*size;i++)
	    if (q_out[i]!=-1){
	       bits=8*(dump[i]->symbols-dump[i]->symbols_top);
	       n_bits+=bits;
	       printf("%d\t%d\t%d\n",(int)(entropy[i]),bits,bits-(int)entropy[i]);
	    }
	printf("n_bits=%d %f\n",n_bits,8*(float)(image->rows*image->columns)/(float)n_bits);

	/* clean things */
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
	free(x_out);
	free(min);
	free(max);
	free(entropy);
	free_blocks(image,size);

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


        if (argc<5){
           printf("%s:
        <input> - no extention -> gets .y .u .v or .raw file
        <output> - no extention -> outputs .bct  file
        <bpp>   - compression
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

        if (argc>=5){
           if (strcmp(argv[4],"cppm")==0){
              colour=1;
              pgm=0;
              printf("Colour DCT Codec:raw\n");
           }
           if (strcmp(argv[4],"cpgm")==0){
              colour=1;
              pgm=1;
              printf("Colour DCT Codec:y u v\n");
           }
           if (strcmp(argv[4],"y")==0){
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
        /* work out bits availible */
        n_bits_y=image_y->rows*image_y->columns*atof(argv[3]);
 
        /* set colour compression strengths */
        if (colour){
           n_bits_u=(int)(0.05*(float)n_bits_y); /* 10% colour */
           n_bits_v=(int)(0.05*(float)n_bits_y);
           n_bits_y-=(n_bits_u+n_bits_v);        /* keep totals bits same */
        }
 
        /* compress image */
        sprintf(cn,"%s.bct",argv[2]);
        fp=fopen(cn,"wb");
        dct_compress(image_y,fp,loaded,cosine,(float)n_bits_y,size,no_quant);
        if (colour){
           dct_compress(image_u,fp,loaded,cosine,(float)n_bits_u,size,0);
           dct_compress(image_v,fp,loaded,cosine,(float)n_bits_v,size,0);
        }
        fclose(fp);

}

