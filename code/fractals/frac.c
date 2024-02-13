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
	and setup blocks has been changed.... replace orginal */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include "macro.h"

/* Structure Definitions */
/* Block definitions */
typedef struct block_data {
	int *pt; /* Block data */
	int x,y;
	int size; /* size of block - assumes square */
	float *c;   /* basis functions */
	int nitems; /* may not be neccessary*/	
	int cf;  /* fractal coefficent */
	int px,py;  /* parent location */
	float error;
	struct block_data *next; 
} BLOCK;

/* Image struct containing image data and size of image */
typedef struct {
	int *pt; /* store of image before it is placed in blocks*/
	BLOCK *tree_top; /* pointer to the top of the linked 
				     list of image blocks*/
	int columns;
	int rows;
	int greylevels;
} IMAGE;

typedef struct basis_data {
	float *pt;
	float sum2;
	int size;
	float q;
	struct basis_data *next;  /* next basis vector */
	struct basis_data *lower; /* indicates lower size 
				     - only vaild for root pointers*/
	int min,max;  /* min/max size of quantised coefficients */
	int nitems;  /* number of basis function in lower root*/
	int x,y;     /* only used for fractal parent blocks */
} BASIS;

/*definitions */

#define PI 3.142
#define Q 1.5
#define POLY 0 /* switchs poly only on(1) /off(0) */


/* prototypes */
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
void compress_image();

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
        fprintf(fp,"\n# Created by fractal Image Compression (daveb 96)");
        fprintf(fp,"\n%d %d",image->columns,image->rows);
        fprintf(fp,"\n255\n");                          /* 255 graylevels */
 
	/* write image data */
        for (i=0;i<image->rows*image->columns;i++){
            if (image->pt[i]>255)
               image->pt[i]=255;
            if (image->pt[i]<0)
               image->pt[i]=0;
            tmp=(unsigned char)(image->pt[i]);
            putc(tmp,fp);
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
void setup_blocks(image,size)
IMAGE *image;
int size;
{
	int i,j,ii,jj,tmp,flag,avg,cnt;
	BLOCK *block,*last;
	
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

	/* kepp the hole image tempory image */

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
	pt->px=x;
	pt->py=y;
	
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

	float sum2,u;
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
		    sum2=0.0;
	            for (j=0;j<s;j++)
	                for (i=0;i<s;i++){
			    basis->pt[i+s*j]=u*cos(PI*(float)k*((float)i+0.5)/(float)s)
					     *cos(PI*(float)l*((float)j+0.5)/(float)s);
			    sum2+=basis->pt[i+s*j]*basis->pt[i+s*j];
			}
		    
		    basis->sum2=sum2; /* equal to block size for dct but for others? */

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
	a=sum/basis->sum2;
	
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

void fit_basis_to_image(image,basis)
IMAGE *image;
BASIS *basis;
{
	BLOCK *block;
	float error;
	float frac_error;

	error=0;
	/* code orginal tree of image */
	block=image->tree_top;
	while (block!=NULL){
	      fit_basis_to_block(block,basis);
	      block->error=calculate_block_error(block,basis);
	      error+=block->error;
	      quantise_block(block,basis);
	      if (POLY==0)
	         frac_error=fit_fractal_to_block(image,block,basis);
	      block=block->next;
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
	      printf("bits=%f\n",bits/log(2.0));
	      free(bins);

	      /* next coefficent */
	      i++;
	      basis=basis->next;
	} 

	/* fractal */
	if (POLY==0){
           /* first search for c min/max */
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

        int i,j;
 
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

	/* render image from basis function + scaling */
	j=0;
	while (basis!=NULL){
              for (i=0;i<block->size*block->size;i++)  
		  block->pt[i]+=block->c[j]*basis->pt[i];    
	      j++;
              basis=basis->next;
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

float fit_fractal_to_block(image,block,basis)
IMAGE *image;
BLOCK *block;
BASIS *basis;
{
	BASIS parent;
	float c,cq,error;

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
           block->px=block->x-(block->size/2);
           if (block->px<0)
              block->px=0;
           if (block->px+block->size*2>=image->columns)
              block->px=image->columns-block->size*2;

           block->py=block->y-(block->size/2);
           if (block->py<0)
              block->py=0;
           if (block->py+block->size*2>=image->rows)
              block->py=image->rows-block->size*2;

	   parent.x=block->px;
	   parent.y=block->py;

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
	for (i=0;i<parent->size*parent->size;i++)
	    pt[i]=(int)rint(parent->pt[i]);
	    

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
 
        for (i=0;i<top->size*top->size;i++)
            parent->pt[i]/=sqrt(sum);

	parent->sum2=1;
	
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

	int i,j,k,pos;
	BLOCK *block;
	BASIS parent;
	int *pt;
	float cf;
	int *tmp;

	/* load image generated by basis functions in tmp */
	tmp=(int *)malloc(sizeof(int)*image->columns*image->rows);
	for (i=0;i<image->rows*image->columns;i++)
	    tmp[i]=image->pt[i];

	/* more thn one pass of rendering cause problems */
	for (k=0;k<3;k++){
	    block=image->tree_top;
	    while (block!=NULL){
	          /* skip if there is no fractal in the block */
	          if (block->cf==0){
	             block=block->next;
		     continue;
	          }
	      
	          /* find parent position */
	          parent=find_parent_location(image,block);

	          /* unquantise cf */
	          cf=Q*(float)(block->size*block->cf);

	          /* shrink down approx parent */
	          parent.size=block->size;
	          shrink_parent_onto_child(image,&parent);

	          /* orthogonalise approx parent */
	          orthogonalise_block(basis,&parent);

	          /* render fractal onto image */
	          for (j=0;j<block->size;j++)
	              for (i=0;i<block->size;i++){
			  pos=(block->y+j)*image->columns+block->x+i;
			  if (i+block->x<image->columns && j+block->y<image->rows) 
		             image->pt[pos]=(int)(cf*parent.pt[j*block->size+i])+
					    tmp[pos];
		  }
	
	          free(parent.pt);
	          block=block->next;
	    }
	}
	free(tmp);


}

void compress_image(image,basis,size)
IMAGE *image;
BASIS *basis;
int size;
{

	int bits;

	setup_blocks(image,size);

	fit_basis_to_image(image,basis);

	bits=calculate_image_bits(image,basis);
	printf("bits=%d\n",bits);

        unquantise_image(image,basis);

        render_image(image,basis);

        reform_image(image,size);

	if(POLY==0)
           render_fractal(image,basis,size);

}

void main(argc,argv)
int argc;
char *argv[];
{
	int i,j,size,scales,colour,pgm;
        int n_bits_y,n_bits_u,n_bits_v;
        IMAGE *image;
        BASIS *basis,*scan;
        FILE *fp;
 
        if (argc<4){
           printf("%s: <input> <output> <size>\n",argv[0]);
           exit(-1);
        }
         

	/* setup  basis structures */
        basis=(BASIS *)malloc(sizeof(BASIS));
        size=atoi(argv[3]);
        setup_cosine_basis(basis,size);
 
	/* make space for images */
        image=(IMAGE *)malloc(sizeof(IMAGE));
        image->pt=NULL;
        get_pgm_file(image,argv[1]);

	compress_image(image,basis,size);
 
        write_pgm_file(image,argv[2]);

}

