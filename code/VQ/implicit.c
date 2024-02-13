/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*    	   	     Implicit Fracatal TRANSFORM 		*/
/*			14/3/97					*/
/*##############################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "mem_hand.h"
#include "blocks.h"
#include "frac.h"

/* loacal structure */
typedef struct tables_data {
        float *r1,*r2;
        int *x,*y;
        int nitems;
} FRAC_TABLES;

/* Macros */
#define myabs(x)    ( x < 0 ? -x : x)

/* prototypes */

void load_frac_tables (FRAC_TABLES *, int );
float fit_fractal_to_block (IMAGE *, BLOCK *, BASIS *);
float fit_block_to_parent (BLOCK *, BASIS *); 
void orthogonalise_block (BASIS *, BASIS *);
 void shrink_parent_onto_child (IMAGE *, BASIS *);
BASIS find_parent_location (IMAGE *, BLOCK *, BASIS *);

/* static golbal varibles */
static FRAC_TABLES *tables=NULL;
 
void fit_fractal_to_image
(IMAGE *image,
 BASIS *basis)
{
	BLOCK *block;

	if (tables==NULL){
	   tables=(FRAC_TABLES *)calloc(4,sizeof(FRAC_TABLES));
	   if (!tables){
	      printf("memory fault at fract tables\n");
	      exit(-1);
	   }	
	   load_frac_tables(tables,32);
	   load_frac_tables(tables+1,16);
	   load_frac_tables(tables+2,8);
	   load_frac_tables(tables+3,4);
	}
	
	for (block=image->tree_top;block!=NULL;block=block->next){
	    fit_fractal_to_block(image,block,basis);
	}

}

float fit_fractal_to_block
(IMAGE *image,
 BLOCK *block,
 BASIS *basis)
{
        BASIS parent;
        float error;

	if (block->size<=2){
	   block->cf=0;
	   return 0.0; /* 2x2 dct ~ lossless */
	}

        /* find centred parent block */
        parent=find_parent_location(image,block,basis);

        /* take centred parent and shrink onto child */
        parent.size=block->size;
        shrink_parent_onto_child(image,&parent);

        /* orthogonalise block */
        orthogonalise_block(basis,&parent);

        error=fit_block_to_parent(block,&parent);

        /* claer up */
        free_float(parent.pt);

        return error;
}

float fit_block_to_parent
(BLOCK *block,
 BASIS *parent)
{
        float c,cq,error;
 
        /* find scaleing of fractal */
        c=calculate_block_scaling(block,parent);
 
        /* quantise cf */
        cq=rint(c/(float)(Q*block->size));
 
        /* save quantised fractal coeff */
        block->cf=(int)cq;
 
        /* calcuate error */
        cq*=(float)(Q*block->size);
        error=2*cq*c-cq*cq;
 
        return error;
 
}

void orthogonalise_block
(BASIS *top,
 BASIS *parent)
{
        int i;
        float c,sum;
        int *pt;
        BASIS *basis;
 
        /* find compatable basis size */
        while (top->size!=parent->size){
              top=top->lower;
              if (top==NULL){
                 printf("Unable to find basis for block %d %d\n",top->size,parent->size);
                 exit(-1);
              }
        }       
 
        /* malloc tmp store for parent */
        pt=malloc_int(parent->size*parent->size,"orthogonalise block");
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
        free_int(pt);
 
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

void shrink_parent_onto_child
(IMAGE *image,
 BASIS *parent)
{
        int i,j,k,l,cnt;
        int sum;
        float *pt; 

        /* allocate mem for shrunk parent */
        pt=malloc_float(parent->size*parent->size,"shrink parent onto child");     

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

void render_fractal
(IMAGE *image,
 BASIS *basis)
{
 
        int i,j,k,pos;
        BLOCK *block;
        BASIS parent;
        float cf;
        int *tmp;
 
	/* load fractal tables if they have not been */
	if (tables==NULL){
           tables=(FRAC_TABLES *)calloc(4,sizeof(FRAC_TABLES));
           if (!tables){
              printf("memory fault at fract tables\n");
              exit(-1);
           }
           load_frac_tables(tables,32);
           load_frac_tables(tables+1,16);
           load_frac_tables(tables+2,8);
           load_frac_tables(tables+3,4);
        }

        /* load image generated by basis functions in tmp */
        tmp=malloc_int(image->columns*image->rows,"render fractal");

        for (i=0;i<image->rows*image->columns;i++)
            tmp[i]=image->pt[i];
              
        /* more than one pass of rendering cause problems */
        for (k=0;k<3;k++){
            block=image->tree_top;
            while (block!=NULL){
                  /* skip if there is no fractal in the block */
                  if (block->cf==0 || block->c==NULL){
                     block=block->next;
                     continue;
                  }
                  /* find parent position */
                  parent=find_parent_location(image,block,basis);
        	  parent.size=block->size;
 
                  /* unquantise cf */
                  cf=Q*(float)(block->size*block->cf);
 
	          /* shrink down approx parent */
                  shrink_parent_onto_child(image,&parent);

                  /* orthogonalise approx parent */
                  orthogonalise_block(basis,&parent);

		  /* render fractal onto image */ 
                  for (j=0;j<block->size;j++)
                      for (i=0;i<block->size;i++){
                          pos=(block->y+j)*image->columns+block->x+i;
                          if (i+block->x<image->columns && j+block->y<image->rows)
                             image->pt[pos]=(int)rint(cf*
                                    parent.pt[j*block->size+i]/parent.sum)+tmp[pos];
                      }

        
                  free_float(parent.pt);
                  block=block->next;
            }
        }     
        free_int(tmp);
}



void load_frac_tables
(FRAC_TABLES *frac,
 int size)
{
        int i,nitems;
        char cn[50];
        FILE *fp;
 
        /* select input name */
        sprintf(cn,"search.%d",size);
 
        /* open file and find length */
        fp=fopen(cn,"rb");
        fscanf(fp,"%d\n",&nitems);
 
        /* initalize tables */
        frac->nitems=nitems;
        frac->x=malloc_int(nitems,"load frac tables x");
        frac->y=malloc_int(nitems,"load frac tables y");
        frac->r1=malloc_float(nitems,"load frac tables r1");
        frac->r2=malloc_float(nitems,"load frac tables r2");
        
        /* load tables */
        for (i=0;i<nitems;i++)
            fscanf(fp,"%f\t%f\t%d\t%d\n",&frac->r1[i],&frac->r2[i],
                                      &frac->x[i],&frac->y[i]);
 
}

BASIS default_parent_location
(IMAGE *image,
 BLOCK *block)
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
        if (parent.y+block->size*2>=image->rows)
           parent.y=image->rows-block->size*2;

	return parent;
}

BASIS find_parent_location
(IMAGE *image, 
 BLOCK *block,
 BASIS *basis)
{
 
        int i,px,py,x;
        int best_i;
        float cx,cy,c2x,c2y;
        float ar1,ar2,ar3,r1,r2,r3;
        float rxy,ryx,r2xx,r2yy,inf;
        float best_error,error;
        FRAC_TABLES *frac;
	BASIS parent;
 
        inf=999999999;
 
        /* only search if blcok size is more than 2x2 */
        if (block->size<4 || block->size>32)
	   return default_parent_location(image,block);

	switch (block->size){
	       case(32): frac=tables;
			 break;
	       case(16): frac=tables+1;
			 break;
	       case(8):  frac=tables+2;
			 break;
	       case(4):  frac=tables+3;
			 break;
	       default: printf("could not find frac tables\n");	
		        exit(-1);
	}
 
        /* unquantise coefficents */
        cx=block->c[1]*Q*block->size;
        c2x=block->c[2]*Q*block->size;
        cy=block->c[3]*Q*block->size;
        c2y=block->c[5]*Q*block->size;
 
        /* establish ratios */
        if (cx==0){
           ryx=inf;
           r2xx=inf;
        }     
        else {
             ryx=cy/cx;
             r2xx=c2x/cx;
        }     
 
        if (cy==0){
           rxy=inf;
           r2yy=inf;
        }     
        else {
             rxy=cx/cy;
             r2yy=c2y/cy;
        }     
 
        /* descide edge direction */
        if (myabs(cx)>myabs(cy)){
           r1=r2xx;
           r2=ryx;
           r3=r2yy;
        }     
        else {
           r1=r2yy;
           r2=rxy;
           r3=r2xx;
        }     

        /* take absolute value of ratios */
        ar1=myabs(r1);
        ar2=myabs(r2);
        ar3=myabs(r3);
 
        /* set default levels for px,py */
        px=block->size/2;
        py=block->size/2;
	parent.x=block->x-px;
	parent.y=block->y-py;
	
 
        /* search for best fit of parent block */
        best_error=inf;
	best_i=0;
        for (i=0;i<frac->nitems;i++){
            error=(frac->r1[i]-ar1)*(frac->r1[i]-ar1)+(frac->r2[i]-ar2)*(frac->r2[i]-ar2);
            if (error<best_error){
               best_error=error;
               best_i=i;
            }
        }     
        
        if (best_error==inf)
	   return default_parent_location(image,block);
              
        /* load up parent positions */
        px=frac->x[best_i];
        py=frac->y[best_i];
 
        /* find symetries */
        /* reflect in y-axis */
        if (ar1!=r1)
           px=block->size-px;

        /* reflect in x-axis */
        if ( (ar2!=r2&&ar1==r1) || (ar2==r2&&ar1!=r1))
           py=block->size-py;

        /* reflect in xy diagonal */
        if (myabs(cy)>myabs(cx)){
           x=px;
           px=py;
           py=x;
        }     
 
        /* Find co-ord of corner of the parent */
        parent.x=block->x-px;;
        if (parent.x<0)
           parent.x=0;
        if (parent.x+block->size*2>=image->columns)
           parent.x=image->columns-block->size*2;
              
        parent.y=block->y-py;
        if (parent.y<0)
           parent.y=0;
        if (parent.y+block->size*2>=image->rows)
           parent.y=image->rows-block->size*2;
              

	return parent;
        /*
        printf("orig=%d\t%d\t",block->x-block->size/2,block->y-block->size/2);
        printf("new=%d\t%d\tsize=%d\n",parent.x,parent.y,block->size);
        */
 
}


