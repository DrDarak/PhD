#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "struct.h"
#include "vq.h"
#include "render.h"


VQ *loadVQ(char *fname)
{
	FILE *fp;
	int i,size,nitems;
	VQ *vq,*first=NULL,*last=NULL;

	fp=fopen(fname,"rb");

	/* check for open file */
	if (!fp){
	   printf("warning: Can not open file in LoadVQ \n");
	   return NULL;
	}

	fscanf(fp,"%d %d\n",&size,&nitems);

	for (i=0;i<nitems;i++){
	    vq=newVQ(size,i);

	    /* link the list */
	    if (!i)
	       first=vq;
	    else
		last->next=vq;

	    fread(vq->pt,sizeof(int),size*size,fp);
	    normaliseVQ(vq);

	    last=vq;
	}

	fclose(fp);

	/* return top of linked list */
	return first;
}

VQ * saveVQ(char *fname,VQ *first)
{

	FILE *fp; 
        int nitems; 
        VQ *vq,*next=NULL; 
 
        fp=fopen(fname,"wb"); 
 
        /* check for open file */ 
        if (!fp){ 
           printf("warning: Can not open file in saveVQ \n"); 
           return first ;   
        } 

	nitems=0;
	vq=first;
	while (vq!=NULL){
	    normaliseVQ(vq);
	    if (vq->sum2>0){
	       nitems++;
	       vq=vq->next;
	    } 
	    else {
	         next=vq->next;
		 first=removeVQFromList(first,vq);
	         vq=next;
	    }
	}
		
	if (nitems==0)
            return NULL;

	fprintf(fp,"%d %d\n",first->size,nitems); 
 
        for (vq=first;vq!=NULL;vq=vq->next){ 
            fwrite(vq->pt,sizeof(int),vq->size*vq->size,fp); 
	}
 
        fclose(fp); 

	return first;
}

VQ *generateBlankVQ(int size,int nitems)
{
	int i,j;
        VQ *vq,*first=NULL,*last=NULL;

      	for (i=0;i<nitems;i++){ 
            vq=newVQ(size,i); 
 
            /* link the list */ 
            if (!i) 
               first=vq; 
            else 
                last->next=vq; 
 
	    /* create balnk block */
	    for (j=0;j<size*size;j++)
	        vq->pt[j]=1;

	    normaliseVQ(vq);

            last=vq; 
        } 
	
	return first;


}

void normaliseVQ(VQ *vq)
{

	int i;
	float factor=0.0;

	/* find sum of squares */
	vq->sum2=0;
	for (i=0;i<vq->size*vq->size;i++)
	    vq->sum2+=vq->pt[i]*vq->pt[i];

	if (vq->sum2==0)
	   printf("%d\n",vq->size);

	/* convert to -255 -> +255 */
	if (vq->sum2>0)
	   factor=1/sqrt((double)(vq->sum2));
	else
 	    factor=0;

	for (i=0;i<vq->size*vq->size;i++)
	    vq->pt[i]=(int)(256.0*(float)(vq->pt[i])*factor);

      	/* find new sum of squares should be ~256*256 */ 
        vq->sum2=0;
        for (i=0;i<vq->size*vq->size;i++)
            vq->sum2+=vq->pt[i]*vq->pt[i];


}

void printVQ(VQ *vq)
{

	int i,j,pos;

	printf("Index=%d\n",vq->index);
	for (j=0,pos=0;j<vq->size;j++){
	    for (i=0;i<vq->size;i++,pos++)	
	        printf("%d\t",vq->pt[pos]);
	    printf("\n");
	}

}

VQ *newVQ(int size,int index)
{
	VQ *vq;

	vq=(VQ *)calloc(1,sizeof(VQ));

	if (!vq){
	   printf("warning VQ malloc fail\n");
	   return NULL;
	}

	vq->pt=(int *)calloc(size*size,sizeof(int));
	vq->acc=(int *)calloc(size*size,sizeof(int));
	vq->size=size;
	vq->index=index;
	vq->sum2=0;
	vq->next=NULL;

	return vq;

}

void deleteVQ(VQ *vq)
{
	if (vq->pt)
	   free(vq->pt);
	if (vq->acc)
	   free(vq->acc);

	free(vq);

}

void treeDeleteVQ(VQ *first)
{

	VQ *vq,*last;

	if (first){
	   for (last=first,vq=first->next;vq!=NULL;vq=vq->next)
	       deleteVQ(last);
	   deleteVQ(last);
	}

	return;
}


VQ *addErrorBlockToVQ(VQ *first,BLOCK *block)
{
	int i,j;
	VQ *vq;
	int *pix,*vqError;

	/* move to end vq blcok */
	if (first){
	   for (vq=first;vq->next!=NULL;vq=vq->next);
           vq->next=newVQ(block->size,vq->index+1);
	   vq=vq->next;
	}
	else
	    vq=newVQ(block->size,0); 	

	if (!block->c)
	   return vq;
         
        /* make coefiicents -ve for rendering... remove approx from picture */
        for (i=0;i<block->nitems;i++){ 
            block->c[i]=rint(block->c[i]/(float)(Q*block->size));
            block->c[i]*=-(Q*block->size);
        }

	render4x4m6(block);

    	/* restore coefficents */
        for (i=0;i<block->nitems;i++) 
            block->c[i]*=-1;

	pix=block->pt;
	vqError=vq->pt;
	for (j=0;j<block->size;j++,pix+=block->jump)
            for (i=0;i<block->size;i++)
                (*vqError++)=(*pix++);

	return vq;

}

void codeBlockVQ(VQ *first,BLOCK *block)
{

        int i,j,index;
        VQ *vq=NULL;
        int *pix,*error;

        /* make coefiicents -ve for rendering... remove approx from picture */
        for (i=0;i<block->nitems;i++){
            block->c[i]=rint(block->c[i]/(float)(Q*block->size));
            block->c[i]*=-(Q*block->size);
        }

        render4x4m6(block);

        /* restore coefficents */
        for (i=0;i<block->nitems;i++)
            block->c[i]*=-1;

	vq=newVQ(block->size,0);
	error=vq->pt;
        pix=block->pt;
        for (j=0;j<block->size;j++,pix+=block->jump)
            for (i=0;i<block->size;i++)
                (*error++)=(*pix++);

	block->cf=(int)findBestVQ(first,vq,&index);
	printf("%d\n",block->cf);
	block->vq=index;

	deleteVQ(vq);

	return ;

}

float findBestVQ(VQ *first,VQ *match,int *index)
{
	VQ *vq,*best=NULL;
	float c,best_c;
	int i;

	*index=0;
	best_c=0;
	for (vq=first;vq!=NULL;vq=vq->next){
	    c=differenceVQ(vq,match);
	    if (best_c<c){
	       best_c=c;
	       best=vq;
	    }
	}

	if (!best)
	   return 0.0;
	printf("%d %d\n",best->index,match->index);	
	/* acculate best entry */
	best->nitems++;
	*index=best->index;
	
 	for (i=0;i<best->size*best->size;i++)    
	    best->acc[i]+=(int)(best_c)*match->pt[i];

	return best_c;

}

float differenceVQ(VQ *basis, VQ *match)
{
	float coeff;
	int i,sum;

	if (basis->size!=match->size){
	   printf ("size missmatch\n");
	   return 0.0;
	}
	
	sum=0;
	for (i=0;i<basis->size*basis->size;i++)
	    sum+=(basis->pt[i]*match->pt[i]);

	if (basis->sum2!=0);
	   coeff=(float)sum/sqrt((float)(basis->sum2));

	return coeff;

}

VQ *partitionVQ(VQ *first,VQ *firstError,float *sum)
{
	int i,index;
        VQ *vq=NULL;

	/* first find best matchs */
	*sum=0;
        for (vq=firstError;vq!=NULL;vq=vq->next)
	    *sum+=findBestVQ(first,vq,&index);

        for (vq=first;vq!=NULL;vq=vq->next)
	    if (vq->nitems==0)
	       /* rm unused entries */
	       first=removeVQFromList(first,vq);
	    else { 
		 /* clear acumilated data and partition*/
	         for (i=0;i<vq->size*vq->size;i++){	
	             vq->pt[i]=vq->acc[i];
		     vq->acc[i]=0;
		 }
	         vq->nitems=0; /* clear intem count */
		 normaliseVQ(vq); /* normalise , new partition */	
	    }
	return first;

}

VQ *removeVQFromList(VQ *first,VQ *remove)
{
        VQ *vq,*last,*next;

        last=NULL;

        /* scan link list for item to delete*/
        for (vq=first;vq!=NULL;vq=vq->next){
            if (vq==remove)
               /* if top of list is deleted return new lisr top */
               if (vq==first){
                  next=vq->next;
	          deleteVQ(vq);
                  return next;
               }
               else {
                    /* otherwise delete item and link
                       adjacent items together */
                    next=vq->next;
	            deleteVQ(vq);
                    last->next=next;
                    return first;
              }
              last=vq;
        }
 
        printf("Could not find item to delete\n");
	return first;
 
}

VQ *partitionImage(IMAGE *image,VQ *first)
{
	int i,cnt;
	BLOCK *block=NULL;
	VQ *vq,*error=NULL,*firstError=NULL;
	float sum;

	/* first render blocks fro form and error image*/
   	for (block=image->tree_top;block!=NULL;block=block->next)
	    if (block->size==MIN_SIZE){
	       error=addErrorBlockToVQ(error,block);
	       if (!firstError)
	          firstError=error;
	    }

            for (vq=first,cnt=0;vq!=NULL;vq=vq->next)
	        cnt++;
	    printf("%d\n",cnt);
	for (i=0;i<10;i++){
	    first=partitionVQ(first,firstError,&sum);
            for (vq=first,cnt=0;vq!=NULL;vq=vq->next)
	        cnt++;
	    printf("%f\t%d\n",sum,cnt);
	}

	return first;
}

void renderVQ(IMAGE *image,VQ *first)
{
	int i,j;
	BLOCK *block=NULL;
        VQ *vq;
	int *pix,*error;
	float tmp;

        for (block=image->tree_top;block!=NULL;block=block->next)
	    if (block->size==MIN_SIZE){
	       for (vq=first;vq!=NULL && vq->index!=block->vq;vq=vq->next);

	       if (vq && block->c && vq->sum2!=0){
	       printf("%d %d\n",block->vq,vq->index);
	           pix=block->pt;
                   error=vq->pt;
                   for (j=0;j<block->size;j++,pix+=block->jump)
                       for (i=0;i<block->size;i++){
	                   tmp=(float)(block->cf*(*error++));
	                   tmp/=sqrt((float)(vq->sum2));
                           (*pix++)+=(int)tmp;
	               }
	       }
	    }

}
/*
main ()
{
	VQ *vq,*first;

	first=loadVQ("list.vq");

	for (vq=first;vq!=NULL;vq=vq->next){

	    printVQ(vq);
	    printf("\n");

	}


}
*/
