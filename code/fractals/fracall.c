/*##############################################################*/
/*			David Bethel				*/
/*		       Bath University 				*/
/*		      daveb@ee.bath.ac.uk			*/
/*		    Generic Fract coder				*/
/*		        + decoder  		                */
/*		  	   25/4/96				*/
/*##############################################################*/

/* routines:
	render blocks
	render fractal
	fit_fractal_block
	(new) fit_parent_to_block
	orthogonalise block
	shrink block
	+ may need to mod find parent block....
	and setup blocks has been changed.... replace in orginal */ 
	

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include "macro.h"

/* Structure Definitions */
/* Block definitions */
typedef struct basis_data {
        float *pt;
        float sum;
        int size;
        float q;
        struct basis_data *next;  /* next basis vector */
        struct basis_data *lower; /* indicates lower size
                                     - only vaild for root pointers*/
        int min,max;  /* min/max size of quantised coefficients */

	/* fractal */
        int nitems;  /* number of basis function in lower root*/
		     /* OR number of parent used (fractal only) */
        float error;  /* error caused byt fractal blocks*/
        int x,y;     /* only used for fractal parent blocks */
	/* VQ */
	float *acc;

} BASIS;

typedef struct block_data {
	int *pt; /* Block data */
	int x,y;
	int size; /* size of block - assumes square */
	float *c;   /* basis functions */
	int nitems; /* may not be neccessary*/	
	int cf;  /* fractal coefficent */
	float error;    
	int px,py;  /* offset from centre parent location 
			only used in normal searaching */
	BASIS *parent; /* only used in limited search */
	struct block_data *next; 
} BLOCK;

/* Image struct containing image data and size of image */
typedef struct image_data {
	int *pt; /* store of image before it is placed in blocks*/
	BLOCK *tree_top; /* pointer to the top of the linked 
				     list of image blocks*/
	BASIS *parent_top; /* top of parent list */
	int columns;
	int rows;
	int greylevels;

	/* only in sub image divition */
	int x,y;	/* position in subimage */
	struct image_data *next; 
	
} IMAGE;

/*definitions */

#define PI 3.142
#define Q 4.0
#define BITSVQ 8 /* number of bits used to describe vq block coefficents */
#define POLY 0 /* switchs poly only on(1) /off(0)  0 - implies fractal*/
#define SEARCH 3 /* switchs searching on off(0) normal(1) limited(2) VQ(3)*/

/* prototypes */
void quantise_vq_list();
void unsplit_image();
void split_image();
float partition_vq();
float find_best_vq_block();
float improve_vq_list();
void setup_vq_list();
float mse();
BASIS * delete_basis_from_list();
float normal_search_for_fractal();
float limited_search_for_fractal();
float fit_block_to_parent();
void get_pgm_file();
void write_pgm_file();
void reform_image();
void setup_blocks();
BLOCK *allocate_block_memory();
void setup_cosine_basis();
float calculate_scaling();
void fit_basis_to_block();
void fit_basis_to_image();
float calculate_block_error();
void render_image();
void render_block();
void quantise_image();
void quantise_block();
void unquantise_image();
void unquantise_block();
void append_block_to_list();
void shrink_parent_onto_child();
float fit_fractal_to_block();
void orthogonalise_block();
BASIS find_parent_location();
void render_block_into_image();
int compress_image();

/* programes */
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
	} while (strncmp(dummy,"#",1)==0);

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
void reform_image(image)
IMAGE *image;
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
	      for (i=0;i<block->size;i++)
	          for (j=0;j<block->size;j++)
		      if (i+block->x<image->columns && j+block->y<image->rows)
	                 image->pt[image->columns*(j+block->y)+i+block->x]=
			          block->pt[block->size*j+i];

	      /* move pointer to next position */
	      block=block->next;
	}

}

/* block image with fixed block size */
void setup_blocks(image,basis,size)
IMAGE *image;
BASIS *basis;
int size;
{
	int i,j,ii,jj,tmp,flag,avg,cnt;
	BLOCK *block,*last;
	BASIS *parent,*last_parent;
	
	last=NULL;
	for (j=0;j<image->rows;j+=size)	
	    for (i=0;i<image->columns;i+=size){

		/* allocate block memory */
	        block=allocate_block_memory(size,i,j);

		/* load image into block */
		flag=0; avg=0; cnt=0;
	        for (ii=0;ii<size;ii++)
	            for (jj=0;jj<size;jj++){
		        tmp=image->columns*(j+jj)+i+ii;
		        if ((ii+i)>=image->columns || (jj+j)>=image->rows){
			   block->pt[size*jj+ii]=-1;
			   flag=1;
		        }
			else {
		            block->pt[size*jj+ii]=image->pt[tmp];
		            avg+=image->pt[tmp];
			    cnt++;
		        }
		    }
		/* if out of image replace with average of block */
		if (flag) 
		   for (ii=0;ii<size;ii++)
                       for (jj=0;jj<size;jj++)
			   if (block->pt[size*jj+ii]==-1)
			      block->pt[size*jj+ii]=avg/cnt;

		/* load top of tree into image  struct */
	        if (i==0 && j==0)
	           image->tree_top=block;
		else
		    last->next=block; /* form link list if applicable */

		/* load last pointer for next pass */
		last=block;

	    }	

	/* find fractal parent s*/
	if (POLY==0 && SEARCH==2){
	   /* setup first parent as null */
	   parent=(BASIS *)malloc(sizeof(BASIS));
	   parent->pt=(float *)calloc(size*size,sizeof(float));
           parent->size=size;
           parent->sum=1;
	   parent->nitems=0;
	   parent->error=0;
	   image->parent_top=parent;
	   
	   block=image->tree_top;
	   while (block!=NULL){
		 parent->next=(BASIS *)malloc(sizeof(BASIS));
		 last_parent=parent;
		 parent=parent->next;
	         parent->sum=0;

		 /* only use parents which have a resonable power
		    NB: compare to block size since sum is sqrt */
		 while (parent->sum<(float)(block->size)){
                       /* find centred parent block */
                       *parent=find_parent_location(image,block);

                       /* take centred parent and shrink onto child */
                       parent->size=block->size;
                       shrink_parent_onto_child(image,parent);

                       /* orthogonalise block */
                       orthogonalise_block(basis,parent);

		       /* at the end of the block list break out if no parent
			  is found and use last parent to end list */
		       if (block->next==NULL){
			  if (parent->sum<(float)(block->size)){
			     parent=last_parent;
                             /* free unused basis */
                             free(parent->next->pt);
                             free(parent->next);
			  }
		          block=block->next;
			  break;
		       }
		       /* increment list */
		       block=block->next;
	 	 }

	         parent->error=0;
	         parent->nitems=0;
	   }
	   parent->next=NULL;	 
	}
	else
            image->parent_top=NULL;

	/* keep the hole image tempory image */

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
	pt->c=NULL;
	pt->cf=0;
	pt->px=0;
	pt->py=0;
	
	/* allocate mem for section of image inside the block */
	pt->pt=(int *)malloc(sizeof(int)*size*size);

	return pt;

}

void setup_cosine_basis(basis,size)
BASIS *basis;
int size;
{
	int i,j,k,l,s,cnt;
	BASIS *next,*lower;

	float sum,u;
	int mask[4][4]={{1,1,1,0},
			{1,1,0,0},
			{1,0,0,0},
			{0,0,0,0}};

	lower=basis;
	for (s=size;s>=2;s/=2){

            /* continue vertical linked list */
            if (s!=size){
	       basis=(BASIS *)malloc(sizeof(BASIS));    
               lower->lower=basis;
	       lower=basis;
            }  

	    cnt=0;
	    for (l=0;l<4;l++)
	        for (k=0;k<4;k++){
		    /* mask unwanted enteries */
		    if (mask[l][k]==0)
		       continue;

		    /* do not form basis larger than block size*/
		    if (k>=s || l>=s)
		       continue;

		    /* continue horizontal linked list */
		    if (!(k==0 && l==0)){
		       next=(BASIS *)malloc(sizeof(BASIS));
		       basis->next=next;
		       basis=next;
		    }

		    /* clear foward pointers */
		    basis->next=NULL;
		    basis->lower=NULL;

	            /* setup basis */
	            basis->size=s;
	            basis->nitems=0; /* only > 0 for root level */
	            basis->pt=(float *)malloc(sizeof(float)*s*s); 
		    basis->min=0;
		    basis->max=0;

		    u=2.0/(float)s;
		    /* scaling */
		    if (k==0)
		       u/=sqrt(2.0);
		    if (l==0)
		       u/=sqrt(2.0);

		    /* setup cosine basis */
		    sum=0.0;
	            for (j=0;j<s;j++)
	                for (i=0;i<s;i++){
			    basis->pt[i+s*j]=u*cos(PI*(float)k*((float)i+0.5)/(float)s)
					     *cos(PI*(float)l*((float)j+0.5)/(float)s);
			    sum+=basis->pt[i+s*j]*basis->pt[i+s*j];
			}
		    
		    basis->sum=sum; /* equal to block size for dct but for others? */

		    /* setup quantisation of basis function */
		    basis->q=Q*s; /* simple method */
		    
		    /* increament number of basis functions */
		    cnt++;
		}

		/* load number of basis function into root pointer*/
		lower->nitems=cnt;
	}
}

float calculate_scaling(pt,basis)
int *pt;
BASIS *basis;
{
	int i,j;
	double sum,a;

	/* find sum */
	sum=0;
	for (i=0;i<basis->size*basis->size;i++)
	    sum+=(float)(pt[i])*basis->pt[i];

	/* calcuate scaling of basis function */
	if (basis->sum==0.0)
	   a=0;
	else
	    a=sum/basis->sum;
	
	return (float)a;

}

void fit_basis_to_block(block,basis)
BLOCK *block;
BASIS *basis;
{

	int i;

	/* first find compatable basis size */
	while (basis->size!=block->size){
	      basis=basis->lower;
	      if (basis==NULL){
		 printf("Unable to find basis for block\n");
		 exit(-1);
	      }
	}

	/* allocate space for coeffcients */
	block->c=(float *)malloc(sizeof(float)*basis->nitems);
	block->nitems=basis->nitems;

	/* calcuate basis scaling / coeffiecients for each basis function */
	i=0;
	while (basis!=NULL){
	      block->c[i++]=calculate_scaling(block->pt,basis);
	      basis=basis->next;
	}
	
}
/* important routinue */

void fit_basis_to_image(image,basis,grad)
IMAGE *image;
BASIS *basis;
float grad;
{
	BASIS *parent,*tmp,*last;
	BLOCK *block;
	float error,cnt,bits;
	float frac_error;
	int i;

	error=0;
	/* code orginal tree of image */
	block=image->tree_top;
	while (block!=NULL){
	      fit_basis_to_block(block,basis);
	      block->error=calculate_block_error(block,basis);
	      error+=block->error;
	      quantise_block(block,basis);
	      if (POLY==0){
		 if (SEARCH==0)
	            frac_error=fit_fractal_to_block(image,block,basis);
		 if (SEARCH==1)
		    frac_error=normal_search_for_fractal(image,block,basis);
		 if (SEARCH==2)
	            frac_error=limited_search_for_fractal(image,block,basis);
	      }
	      block=block->next;
	}

	if (SEARCH==2 && grad!=0.0){
	   printf("Gradient based pruning\n");
	   /* cnt number of occurences of parents */
	   cnt=0;
	   parent=image->parent_top;
	   while (parent!=NULL){
	         cnt+=parent->nitems;
		 parent=parent->next;
	   }

	   /* now work out which parents should be kept based on gradient */
           parent=image->parent_top;
           while (parent!=NULL){
		 if (parent->nitems==0){
		    last=parent;
                    parent=parent->next;
		    image->parent_top=delete_basis_from_list(image->parent_top,last);
		    continue;
		 }
		 /*work out mse/bpp */
		 bits=-(float)(parent->nitems)*log((float)(parent->nitems)/cnt);
		 bits/=log(2);
	
		 /* if the gradient is not higher than normal then delete item */
		 if (grad>(parent->error/bits)){
		    /*printf("%p %f %f %f\n",parent,(parent->error)/bits,grad);*/
		    last=parent;
                    parent=parent->next;
                    image->parent_top=delete_basis_from_list(image->parent_top,last);
                    continue;
                 }

		 /* reset existing symbols */
		 parent->nitems=0;	
		 parent->error=0;	
                 parent=parent->next;
           }


	   /* do limited search on pruned list */
	   block=image->tree_top;
	   while (block!=NULL){
                 limited_search_for_fractal(image,block,basis);
                 block=block->next;
	   }

        }

}

void setup_vq_list(image,basis)
IMAGE *image;
BASIS *basis;
{

	int i,cnt=0;
	BLOCK *block;
	BASIS *vq,*last;

	/* set up first vq as null */
	block=image->tree_top;
        vq=(BASIS *)malloc(sizeof(BASIS));
        vq->size=block->size;
        vq->pt=(float *)calloc(vq->size*vq->size,sizeof(float));
	vq->acc=(float *)calloc(vq->size*vq->size,sizeof(float));
        vq->sum=1;
        vq->nitems=0;
        vq->error=0;
        image->parent_top=vq;

	while (block!=NULL){
              vq->next=(BASIS *)malloc(sizeof(BASIS));
              last=vq;
              vq=vq->next;
	      /* setup vq  */
              vq->sum=0;
              vq->nitems=0;
              vq->error=0;
	      vq->pt=NULL;
	      vq->acc=NULL;
	      
              /* only use blocks which have a resonable power
                 NB: compare to block size since sum is sqrt */
              while (vq->sum<(float)(block->size)){
                    vq->size=block->size;
		    /* malloc space for basis functions */
		    if (vq->pt!=NULL)
   		       free(vq->pt);
                    vq->pt=(float *)calloc(vq->size*vq->size,sizeof(float));
		    if (vq->acc!=NULL)
   		       free(vq->acc);
                    vq->acc=(float *)calloc(vq->size*vq->size,sizeof(float));

	            /* load up vq block */
                    for (i=0;i<vq->size*vq->size;i++)
                        vq->pt[i]=(float)(block->pt[i]);

                    /* orthogonalise block */
                    orthogonalise_block(basis,vq);

	            /* at the end of the block list break out if no parent
                        is found and use last parent to end list */
                    if (block->next==NULL){
		       if (vq->sum<(float)(block->size)){
                          vq=last;
			  /* free unused basis */
		          free(vq->next->pt);
		          free(vq->next->acc);
		          free(vq->next);
		       }
	               block=block->next;
                       break;
                    }

	            block=block->next;
	      }
	      cnt++;

	}
	/*end list*/
	vq->next=NULL;
	printf("%d\n",cnt);

}

float improve_vq_list(image,basis,grad)
IMAGE *image;
BASIS *basis;
float grad;
{
	int i;
	BLOCK *block;
	BASIS *vq;
	float total_error;
	
	/* clear accumilators */ 
	vq=image->parent_top;
	while (vq!=NULL){
	      for (i=0;i<vq->size*vq->size;i++)
		  vq->acc[i]=0;
	      vq->nitems=0;
	      vq->error=0;
	      vq=vq->next;
	}

	/* create partitions */
	block=image->tree_top;
	while (block!=NULL){
	      find_best_vq_block(image,basis,block);
	      /* move onto next block */
	      block=block->next;
	}
	
	total_error=partition_vq(image,basis,grad);
	total_error=partition_vq(image,basis,grad);
	total_error=partition_vq(image,basis,grad);
	total_error=partition_vq(image,basis,grad);
	total_error=partition_vq(image,basis,grad);
	total_error=partition_vq(image,basis,grad);
	total_error=partition_vq(image,basis,grad);
	total_error=partition_vq(image,basis,grad);
	total_error=partition_vq(image,basis,grad);
	total_error=partition_vq(image,basis,grad);
	quantise_vq_list(image);

	return total_error;
}

float partition_vq(image,basis,grad)
IMAGE *image;
BASIS *basis;
float grad;
{
	int i,nitems;
        BLOCK *block;
        BASIS *vq,*last;
        float c,bits,cnt,total_error;


	/* form new vq blocks from partition */
	cnt=0;
        block=image->tree_top;
	while (block!=NULL){
              cnt++;
	      block=block->next;
	}

	/* never delete null block pointer */
	nitems=0;
 	vq=image->parent_top->next;
        while (vq!=NULL){
	      for (i=0;i<vq->size*vq->size;i++)
	          vq->pt[i]=vq->acc[i];
	      /* orthogonalise accumluated data block 
					-should have litte effect */
              orthogonalise_block(basis,vq);

	      /* remove blocks which are not used */
	      if (vq->nitems==0){
                 last=vq;
                 vq=vq->next;
                 image->parent_top=delete_basis_from_list(image->parent_top,last);
                 continue;
              }

	      /* remove blocks with too little power */
	      bits=-(float)(vq->nitems)*log((float)(vq->nitems)/cnt);
	      bits/=log(2);
	      bits+=BITSVQ*vq->size*vq->size;
	      
              if (vq->error/bits<grad){
                 last=vq;
		 vq=vq->next;
                 image->parent_top=delete_basis_from_list(image->parent_top,last);
                 continue;
              }
	      nitems++;
              vq=vq->next;
        }
	printf("%d\n",nitems);

	/* fit basis to new partition */
	/* clear accumilators */
        vq=image->parent_top;
        while (vq!=NULL){
              for (i=0;i<vq->size*vq->size;i++)
                  vq->acc[i]=0;
	      vq->nitems=0;
	      vq->error=0;
              vq=vq->next;
        }

	total_error=0;
 	block=image->tree_top;
        while (block!=NULL){
	      c=find_best_vq_block(image,basis,block);
	      /* increament error reduction due to block */
	      total_error+=c*c;
	      /* store quantised coefficient */
	      block->cf=(int)rint(c/(Q*block->size));

              block=block->next;
        }

	return total_error;
}

float find_best_vq_block(image,basis,block)
IMAGE *image;
BASIS *basis;
BLOCK *block;
{
	
	int i;
	float c,cq,best_error;
	BASIS *vq,tmp,*best;

	vq=image->parent_top;
        best=vq;
        best_error=0;
        while (vq!=NULL){
              c=calculate_scaling(block->pt,vq);
              if (c*c>best_error){
                 best_error=c*c;
                 best=vq;
              }
              vq=vq->next;
        }
        /* save quantised c */
        block->parent=best;
	best->nitems++;

        /* acculmulate best vq */
        tmp.pt=(float *)calloc(best->size*best->size,sizeof(float));
        tmp.size=best->size;

        /* load up block */
        for (i=0;i<best->size*best->size;i++)
            tmp.pt[i]=(float)(block->pt[i]);

        /* orthogonalise block */
        orthogonalise_block(basis,&tmp);

        /* calculate best scaling */
        c=calculate_scaling(block->pt,best);
	cq=(float)(Q*best->size)*rint(c/(float)(Q*best->size));	
	best->error+=2*cq*c-cq*cq;

        /* accumulate block*/
        for (i=0;i<best->size*best->size;i++)
            best->acc[i]+=c*(tmp.pt[i]/tmp.sum);
        free(tmp.pt);
	
	return c;

} 
void quantise_vq_list(image)
IMAGE *image;
{
	int i;
	float max,bits;
	BASIS *vq;

	bits=pow(2,BITSVQ)/2;

	vq=image->parent_top;
        while (vq!=NULL){
	      /* find max */
	      max=0;
	      for (i=0;i<vq->size*vq->size;i++)
		  if (myabs(vq->pt[i]/vq->sum)>max)
		     max=(myabs(vq->pt[i]/vq->sum));
	      if (max==0){
		 vq=vq->next;
	         continue;
	      }
	      /* quantise block */
	      for (i=0;i<vq->size*vq->size;i++)
                  vq->pt[i]=(vq->sum*max/bits)*rint((vq->pt[i]*bits)/(vq->sum*max));

	      /* move to next block */
	      vq=vq->next;
        }

}

int calculate_image_bits
(IMAGE *image,
 BASIS *top)
{
	int i,j;
	float min,max,cnt;
	float bits,*bins;
	BASIS *basis;
	BASIS *parent;
	BLOCK *block;

	/* scan through basis functions */
	basis=top;
	i=0;
	bits=0;
	while (basis!=NULL){
	      /* first search for c min/max */
	      block=image->tree_top;
	      max=0;
	      min=0;
	      while (block!=NULL){
		    if (block->c[i]>max)
		       max=block->c[i]; 
		    if (block->c[i]<min)
		       min=block->c[i]; 
		    block=block->next;
	      } 

	      /* now find number of bits required */
	      bins=(float *)calloc((int)max-(int)min+1,sizeof(float));
	      cnt=0;
	      block=image->tree_top;
              while (block!=NULL){
		    bins[(int)(block->c[i])-(int)min]++;
		    cnt++;
                    block=block->next;
              }
	      for (j=0;j<(int)max-(int)min+1;j++)
		  if (bins[j]!=0)
		     bits-=bins[j]*log(bins[j]/cnt);
	      free(bins);

	      /* next coefficent */
	      i++;
	      basis=basis->next;
	} 

	/* fractal */
	if (POLY==0){
           /* first search for cf min/max */
           block=image->tree_top;
           max=0;
           min=0;
           while (block!=NULL){
	         if (block->cf>max)
                    max=block->cf;
                 if (block->cf<min)
                    min=block->cf;
                 block=block->next;
           }

           /* now find number of bits required */
           bins=(float *)calloc((int)max-(int)min+1,sizeof(float));
           cnt=0;
           block=image->tree_top;
           while (block!=NULL){
                 bins[(int)(block->cf)-(int)min]++;
                 cnt++;
                 block=block->next;
           }
           for (j=0;j<(int)max-(int)min+1;j++)
	       if (bins[j]!=0)
                  bits-=bins[j]*log(bins[j]/cnt);
           free(bins);
	}

	/* calcualte number of bits for normal search */
	if (POLY==0 && SEARCH==1){
           /* first search for x offset min/max */
           block=image->tree_top;
           max=0;
           min=0;
           while (block!=NULL){
                 if (block->px>max)
                    max=block->px;
                 if (block->px<min)
                    min=block->px;
                 block=block->next;
           }

           /* now find number of bits required */
           bins=(float *)calloc((int)max-(int)min+1,sizeof(float));
           cnt=0;
           block=image->tree_top;
           while (block!=NULL){
                 bins[(int)(block->px)-(int)min]++;
                 cnt++;
                 block=block->next;
           }
           for (j=0;j<(int)max-(int)min+1;j++)
               if (bins[j]!=0)
                  bits-=bins[j]*log(bins[j]/cnt);
           free(bins);

	   /* first search for y offset min/max */
           block=image->tree_top;
           max=0;
           min=0;
           while (block!=NULL){
                 if (block->py>max)
                    max=block->px;
                 if (block->py<min)
                    min=block->px;
                 block=block->next;
           }

           /* now find number of bits required */
           bins=(float *)calloc((int)max-(int)min+1,sizeof(float));
           cnt=0;
           block=image->tree_top;
           while (block!=NULL){
                 bins[(int)(block->py)-(int)min]++;
                 cnt++;
                 block=block->next;
           }
           for (j=0;j<(int)max-(int)min+1;j++)
               if (bins[j]!=0)
                  bits-=bins[j]*log(bins[j]/cnt);
           free(bins);
	}

	/* calcualte number of bits for limited search or VQ... same idea */
	if (POLY==0 && (SEARCH==2 || SEARCH==3) ){
	   /* first search for c min/max */
           parent=image->parent_top;
	   cnt=0;
           while (parent!=NULL){
		 cnt+=parent->nitems;
                 parent=parent->next;
           }
	   parent=image->parent_top;
           while (parent!=NULL){
	         if (parent->nitems!=0)
                    bits-=(float)(parent->nitems)*log((float)(parent->nitems)/cnt);
                 parent=parent->next;
           }
	}
 
	/* scale bits */
	bits/=log(2); 
	
	return (int)bits;

}

float calculate_block_error(block,basis)
BLOCK *block;
BASIS *basis;
{

	int i,j;
	float c,cq;
	float error;
	
 
        /* first find compatable basis size */
        while (basis->size!=block->size){
              basis=basis->lower;
              if (basis==NULL){
                 printf("Unable to find basis for block\n");
                 exit(-1);
              }  
        }

	/* first calculate entire error */
	error=0;
	for (i=0;i<block->size*block->size;i++)
	    error+=(float)(block->pt[i]*block->pt[i]);

	/* calcuate reduction in error caused by each coeff  */
        i=0;
        while (basis!=NULL){
              c=block->c[i++];
	      cq=basis->q*rint(c/basis->q);
	      error-=(2*cq*c-cq*cq);
	      basis=basis->next;
              
        }

	return myabs(error);
}

void render_image(image,basis)
IMAGE *image;
BASIS *basis;
{
	BLOCK *block;
 
        block=image->tree_top;
        while (block!=NULL){
              render_block(block,basis);
              block=block->next;
        }
}

void render_block(block,basis)
BLOCK *block;
BASIS *basis;
{

        int i,j,k;
	float pix;
	int *ptr;
	int cnt;
	BASIS *top;
 
        /* first find compatable basis size */
        while (basis->size!=block->size){
              basis=basis->lower;
              if (basis==NULL){
                 printf("Unable to find basis for block\n");
                 exit(-1);
              }
        }

	/* clear image */
        for (i=0;i<block->size*block->size;i++)
	    block->pt[i]=0;	

        k=0;
        top=basis;
        ptr=block->pt;
        cnt=0;
        for (j=0;j<block->size;j++)
            for (i=0;i<block->size;i++,cnt++){
                pix=0;
                for (basis=top,k=0;basis!=NULL;basis=basis->next,k++)
                    pix+=block->c[k]*basis->pt[cnt]/basis->sum;
                (*ptr++)=pix;
            }


}         

void quantise_image(image,basis)
IMAGE *image;
BASIS *basis;
{
       BLOCK *block;
 
        block=image->tree_top;
        while (block!=NULL){
              quantise_block(block,basis); 
              block=block->next;
        }
}

void quantise_block(block,basis)
BLOCK *block; 
BASIS *basis;
{
        int i,j;   
 
        /* first find compatable basis size */
        while (basis->size!=block->size){
              basis=basis->lower;
              if (basis==NULL){
                 printf("Unable to find basis for block\n");
                 exit(-1);
              }    
        }
 
        i=0; 
        while (basis!=NULL){
              block->c[i]=rint(block->c[i]/basis->q);
	      i++;
              basis=basis->next;
        }
 
}        

void unquantise_image(image,basis)
IMAGE *image;
BASIS *basis;
{
       	BLOCK *block;

        block=image->tree_top;
        while (block!=NULL){
              unquantise_block(block,basis);
              block=block->next;
        }
}

void unquantise_block(block,basis)
BLOCK *block; 
BASIS *basis;
{

        int i,j; 

        /* first find compatable basis size */
        while (basis->size!=block->size){
              basis=basis->lower;
              if (basis==NULL){
                 printf("Unable to find basis for block\n");
                 exit(-1);
              }
        }

        i=0;                
        while (basis!=NULL){
              block->c[i++]*=basis->q;
              basis=basis->next;
        }
}         

void append_block_to_list(first,second)
BLOCK *first,*second;
{
	BLOCK *next;

	/* place append block after block block */
	next=first->next;
	first->next=second;
	second->next=next;

}

BASIS * delete_basis_from_list(top,delete)
BASIS *top;
BASIS *delete;
{
        BASIS *basis,*last,*next;

	last=NULL;

	/* scan link list for item to delete*/
	basis=top;
	while (basis!=NULL){
	      if (basis==delete)
		 /* if top of list is deleted return new lisr top */
		 if (basis==top){
		    next=basis->next;
		    free(basis);
		    return next;
		 }
	         else {
		      /* otherwise delete item and link 
			adjacent items together */
		      next=basis->next;
		      /* kill basis struct */
		      if (basis->pt!=NULL)
		         free(basis->pt);
		      free(basis);
		      last->next=next;
		      return top;
		 }
	      last=basis;
	      basis=basis->next;
	}

	printf("Could not find item to delete\n");
	exit(-1);

}

float limited_search_for_fractal(image,block,basis)
IMAGE *image;
BLOCK *block;
BASIS *basis;
{

	BASIS *parent,*best_parent;
	float error,best_error;

	/* error is the impreovemt in MSE of image */
	best_error=0;
	best_parent=image->parent_top;
	parent=image->parent_top;
	while (parent!=NULL){
              error=fit_block_to_parent(block,parent);
	      if (error>best_error){
		 best_parent=parent;
		 best_error=error;
	      }	
	      parent=parent->next;
	}
	/*
	printf("%f\t",best_error);
	printf("%f\t%p\n",error,best_parent);
	*/
	/* leave correct fractal coefficent in block*/
	error=fit_block_to_parent(block,best_parent);

	/* update best parent */
        best_parent->error+=error;
	best_parent->nitems++;
	block->parent=best_parent;

}

float normal_search_for_fractal(image,block,basis)
IMAGE *image;
BLOCK *block;
BASIS *basis;
{
	BASIS parent;
	int i,j,cf;
	float error,best_error;
	int max_search=4,search_size=4;
	int best_i,best_j;

	/* set centre to be best parent position */
	best_error=0;
	best_i=0;
	best_j=0;
	cf=0;

	/* search for best parent position */
	for (i=-max_search*search_size;i<=max_search*search_size;i+=search_size)
	    for (j=-max_search*search_size;j<=max_search*search_size;j+=search_size){
                parent=find_parent_location(image,block);
	        parent.size=block->size;
		/* offset parent */
		parent.x+=i;
		parent.y+=j;

		/* skip if out of range */
		if (parent.x<0 || parent.y<0 || parent.x>=image->columns || parent.y>=image->rows) 
		   continue;

	        /* take centred parent and shrink onto child */
	        shrink_parent_onto_child(image,&parent);

	        /* orthogonalise block */
	        orthogonalise_block(basis,&parent);

		/* skip if block has no real image context */
	        if (parent.sum<(float)(block->size))
		   continue;

		/* fit the block to the parent */
	        error=fit_block_to_parent(block,&parent);

		/* recored best parent position */
		if (error>best_error){
		   best_i=i;
		   best_j=j;
		   best_error=error;
		   cf=block->cf;
		}
		
	        /* claer up */
	        free(parent.pt);
	     }
	
	/* save best positons */
	block->px=best_i;
	block->py=best_j;
	/* restore best fractal coeff */
	block->cf=cf;

	return best_error;
}

float fit_fractal_to_block(image,block,basis)
IMAGE *image;
BLOCK *block;
BASIS *basis;
{
        BASIS parent;
        float error;

        /* find centred parent block */
        parent=find_parent_location(image,block);

        /* take centred parent and shrink onto child */
        parent.size=block->size;
        shrink_parent_onto_child(image,&parent);

        /* orthogonalise block */
        orthogonalise_block(basis,&parent);

        error=fit_block_to_parent(block,&parent);

        /* claer up */
        free(parent.pt);

        return error;
}

float fit_block_to_parent(block,parent)
BLOCK *block;
BASIS *parent;
{
	float c,cq,error;

	/* find scaleing of fractal */
        c=calculate_scaling(block->pt,parent);

	/* quantise cf */
	cq=rint(c/(float)(Q*block->size));

	/* save quantised fractal coeff */
	block->cf=(int)cq;

	/* calcuate error */
	cq*=(float)(Q*block->size);
	error=2*cq*c-cq*cq;

	return error;

}

BASIS find_parent_location(image,block)
IMAGE *image;
BLOCK *block;
{
	   BASIS parent;

           /* Find co-ord of corner of the parent */
           parent.x=block->x-(block->size/2);
           if (parent.x<0)
              parent.x=0;
           if (parent.x+block->size*2>=image->columns)
              parent.x=image->columns-block->size*2;

           parent.y=block->y-(block->size/2);
           if (parent.y<0)
              parent.y=0;
           if (block->py+block->size*2>=image->rows)
              parent.y=image->rows-block->size*2;

	   return parent;
}

void orthogonalise_block(top,parent)
BASIS *top;
BASIS *parent;
{
	int i;
	float c,sum;
	int *pt;
	BASIS *basis;

        /* find compatable basis size */
        while (top->size!=parent->size){
              top=top->lower;
              if (top==NULL){
                 printf("Unable to find basis for block\n");
                 exit(-1);
              }
	}

	/* malloc tmp store for parent */
	pt=(int *)calloc(parent->size*parent->size,sizeof(int));
	/* make sure parent and ponter are same */
	for (i=0;i<parent->size*parent->size;i++){
	    pt[i]=(int)rint(parent->pt[i]);
	    parent->pt[i]=(float)pt[i];
	}

	/* Gramm Smitt orthogonalisation */
	basis=top;
        while (basis!=NULL){
              c=calculate_scaling(pt,basis);
              for (i=0;i<basis->size*basis->size;i++)
                  parent->pt[i]-=c*basis->pt[i];
              basis=basis->next;
        }

	/* free tmp store of parent */
	free(pt);
 
        sum=0;
        for (i=0;i<top->size*top->size;i++)
            sum+=parent->pt[i]*parent->pt[i];

	/* 
        for (i=0;i<top->size*top->size;i++)
	    if (sum==0)
               parent->pt[i]=0;
	    else
                parent->pt[i]/=sqrt(sum);
	*/

	parent->sum=sqrt(sum);
	
}

void shrink_parent_onto_child(image,parent)
IMAGE *image;
BASIS *parent;
{
	int i,j,k,l,cnt;
	int sum;
	float *pt; 

	/* allocate mem for shrunk parent */
	pt=(float *)malloc(sizeof(float)*parent->size*parent->size);	
	assert(pt);

	cnt=0;
	for (j=0;j<2*parent->size;j+=2)
	    for (i=0;i<2*parent->size;i+=2){
	        sum=0;
	        for (l=0;l<2;l++)
	            for (k=0;k<2;k++)
		        sum+=image->pt[(j+l+parent->y)*image->columns 
				       +i+l+parent->x];
		/* MUST be integer average ??? dies otherwise*/
	        pt[cnt++]=(float)(sum/4);	
	    }

	parent->pt=pt;

}

void render_fractal(image,basis,size)
IMAGE *image;
BASIS *basis;
int size;
{
	int i,j,k,pos,iter;
	BLOCK *block;
	BASIS parent;
	
	float cf;
	int *tmp;

	if (SEARCH==3)
	   iter=1;
	else
	    iter=3;

	/* load image generated by basis functions in tmp */
	tmp=(int *)malloc(sizeof(int)*image->columns*image->rows);
	for (i=0;i<image->rows*image->columns;i++)
	    tmp[i]=image->pt[i];

	/* more than one pass of rendering cause problems */
	for (k=0;k<iter;k++){
	    block=image->tree_top;
	    while (block!=NULL){
	          /* skip if there is no fractal in the block */
	          if (block->cf==0){
	             block=block->next;
		     continue;
	          }
	      
		  if (SEARCH==0){
	             parent=find_parent_location(image,block);
                     parent.size=block->size;
		  }

		  if (SEARCH==1){
	             parent=find_parent_location(image,block);
                     parent.size=block->size;
		     parent.x+=block->px;
		     parent.y+=block->py;
		  }
		  
		  if (SEARCH==2 || SEARCH==3)
		     parent=*(block->parent);

	          if ((block->parent!=image->parent_top && SEARCH==2)
                      || SEARCH==0 || SEARCH==1){
                     /* shrink down approx parent */
                     shrink_parent_onto_child(image,&parent);

                     /* orthogonalise approx parent */
                     orthogonalise_block(basis,&parent);
		  }
	

	          /* unquantise cf */
	          cf=Q*(float)(block->size*block->cf);

	          /* render fractal onto image */ 
	          for (j=0;j<block->size;j++)
	              for (i=0;i<block->size;i++){
			  pos=(block->y+j)*image->columns+block->x+i;
			  if (i+block->x<image->columns && j+block->y<image->rows)
		             image->pt[pos]=(int)rint(cf*
			            parent.pt[j*block->size+i]/parent.sum)+tmp[pos];
		      }
	
		  if ((block->parent!=image->parent_top && SEARCH==2) 
		      || SEARCH==0 || SEARCH==1)
		     free(parent.pt);

	          block=block->next;
	    }
	}
	free(tmp);


}

float mse(image1,image2)
IMAGE *image1;
IMAGE *image2;
{
	int i,sum,diff;
	float error;

        sum=0;
        for (i=0;i<image1->rows*image1->columns;i++){
            diff=image1->pt[i]-image2->pt[i];
            diff*=diff;
            sum+=diff;
        }

	error=(float)sum/(float)(image1->rows*image1->columns);

	return error;
}

int compress_image(image,basis,size,grad)
IMAGE *image;
BASIS *basis;
int size;
float grad;
{

	int bits,cnt;
	BASIS *vq;

	setup_blocks(image,basis,size);

	fit_basis_to_image(image,basis,grad);

	if (SEARCH==3 && POLY==0){
	   setup_vq_list(image,basis);
	   printf("error=%f\n",improve_vq_list(image,basis,grad));
	}

	bits=calculate_image_bits(image,basis);

	if (SEARCH==3 && POLY==0){
	   cnt=0;
	   vq=image->parent_top;
	   while (vq!=NULL){
		 cnt++;
	         vq=vq->next;  
	   }
	   /* accounts for blocks been stored */
	   bits+=size*size*cnt*BITSVQ;
	}

        unquantise_image(image,basis);

        render_image(image,basis);

        reform_image(image,size);

	if (POLY==0)
           render_fractal(image,basis,size);

	return bits;
}

void unsplit_image(image)
IMAGE *image;
{

	int i,j;
	IMAGE *sub_image;
	

	sub_image=image->next;
	while (sub_image!=NULL){
	      for (j=0;j<sub_image->rows;j++)
	          for (i=0;i<sub_image->columns;i++)
		      image->pt[(j+sub_image->y)*image->columns+sub_image->x+i]=
			       sub_image->pt[j*sub_image->columns+i];
	      sub_image=sub_image->next;
	}

}
void split_image(image,sub_image_size)
IMAGE *image;
int sub_image_size;
{

	int i,j,ii,jj,nitems;
	IMAGE *sub_image;

	sub_image=image;
	for (j=0;j<image->rows;j+=sub_image_size)
	    for (i=0;i<image->columns;i+=sub_image_size){
	   	sub_image->next=(IMAGE *)calloc(1,sizeof(IMAGE));     
	        sub_image=sub_image->next;
	        sub_image->x=i;
	        sub_image->y=j;
		sub_image->columns=image->columns-i;
		if (sub_image->columns>sub_image_size)
		   sub_image->columns=sub_image_size;
		sub_image->rows=image->rows-j;
		if (sub_image->rows>sub_image_size)
		   sub_image->rows=sub_image_size;
		/* load in image data */
	        sub_image->pt=(int *)calloc(sub_image->columns*sub_image->rows,sizeof(int));
		for (jj=0;jj<sub_image->rows;jj++)
		    for (ii=0;ii<sub_image->columns;ii++)
		        sub_image->pt[jj*sub_image->columns+ii]=
			    image->pt[(j+jj)*image->columns+ii+i];        
	    }
	sub_image->next=NULL;


}

void main(argc,argv)
int argc;
char *argv[];
{
	int i,j,size;
        IMAGE *image,*orig,*sub_image;
        BASIS *basis;
	float grad;
	int bits;
 
        if (argc<4){
           printf("%s: <input> <output> <size> [grad]\n",argv[0]);
           exit(-1);
        }
         
	/* setup  basis structures */
        basis=(BASIS *)malloc(sizeof(BASIS));
        size=atoi(argv[3]);

	/* define grad - MSE/bpp independent of size =error/bits*/
	if (argc>4)
	   grad=atof(argv[4]);
	else
	    grad=0;

        setup_cosine_basis(basis,size);
 
	/* make space for images */
        image=(IMAGE *)malloc(sizeof(IMAGE));
        image->pt=NULL;
        get_pgm_file(image,argv[1]);

	if ((SEARCH==2 || SEARCH==3) &&POLY==0){
	   split_image(image,128*size/4);
	   bits=0;
	   sub_image=image->next;
	   while (sub_image!=NULL){
		 bits+=compress_image(sub_image,basis,size,grad);
		 printf("bits=%d\n",bits);
	         sub_image=sub_image->next;
	   }
	   unsplit_image(image);
	}
	else
	    bits=compress_image(image,basis,size,grad);

	printf("bits=%d\n",bits);
 
        write_pgm_file(image,argv[2]);

        orig=(IMAGE *)malloc(sizeof(IMAGE));
        orig->pt=NULL;
        get_pgm_file(orig,argv[1]);
	printf("MSE=%f\n",mse(image,orig));	

}

