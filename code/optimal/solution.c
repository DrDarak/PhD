/*##############################################################*/
/*		Multiple fuction maximum value finder 		*/
/*								*/
/* 			David Bethel 				*/
/* 			   1/6/95				*/
/*		14/11/95 smooth graphs installed		*/
/*	 	hacked and slashed 26/2/96			*/
/*##############################################################*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "solution.h"

void allocate_mem_coeff_planes
(COEFF_PLANES *pt,
 int nitems)
{

        pt->e=malloc_float(nitems,"allocate mem coeff planes e");
        pt->x=malloc_float(nitems,"allocate mem coeff planes x");
        pt->de=malloc_float(nitems-1,"allocate mem coeff planes de");
        pt->dx=malloc_float(nitems-1,"allocate mem coeff planes dx");
        pt->q=malloc_int(nitems,"allocate mem coeff planes q");
        pt->nitems=nitems;

}


void solve
(COEFF_PLANES *data,
 float *x_out,	/* optimum induvidual compressions */
 float xt,	/* total compression availible */
 float accuracy, 	/* how close approximation needs to be to xt */
 int n)		/* number of coeffcient planes */
{
	int i,j,cnt,best_j;
	float avg_grad,new_xt,last_xt,step,residual;
	float *de; 	/* gradients corresponding to x_out */
	float new_e,e,best_error,diff;

	/* allocate mem for temp output */
	de=malloc_float(n,"solve de");

	/* now work out gradient between each loaded point */
	calculate_gradient(data,n);

	/* smooth gradient points so that they always decress */
	for (i=0;i<n;i++)
	    smooth_gradient(data+i);


	/* split compression to make 1st guess */
	for (i=0;i<n;i++)
	    x_out[i]=xt/(float)n;
	
	/* find the average gradient from all these compressions */
	avg_grad=0;
	for (i=0;i<n;i++){
	    de[i]=find_gradient(data+i,x_out[i]);
	    avg_grad+=de[i];
	}
	avg_grad/=(float)n;

	/* set up inital vale for step  by findng rate of change of 
					grad on C0 at average grad (+ve) */
	step=data->de[0]*COMP_STEP;

	
	new_xt=0;
	do {

	   /* find compression for present gradient */
	   last_xt=new_xt;
	   new_xt=0;
	   for (i=0;i<n;i++){
	       x_out[i]=find_compression(data+i,avg_grad);
	       new_xt+=x_out[i];
	   }

	   /* descide which way to step gradient */
   	   if ((new_xt<xt && step>0) || (new_xt>xt && step<0))
	      step*=-1;
	   
	   /* find if new position has passed correct one */
	   if ((last_xt>xt && new_xt<xt) || (last_xt<xt && new_xt>xt))
	      step/=2;

	   /* decrease the step until there are 
			no -ve gradients (which is impossible)*/
	   while (avg_grad+step<0)
		 step/=2;
		
	   for (i=0;i<n;i++){
	       if (x_out[i]==0.0)
		  break;
	   }
	   avg_grad+=step;
		
	} while (myabs((xt-new_xt)) > accuracy && myabs(step)>0.01);


	/* ammend any failiors with 'manual' tunning */ 
	if (myabs((xt-new_xt)) > accuracy){
	   /* first find how much tunning is required */
	   residual=xt-new_xt;
	   /* tune in 10 steps */
	   for (i=0;i<10;i++){
	       /* find bigest improvement and ammend */
	       best_error=256*256;
	       best_j=0;
	       for (j=0;j<n;j++) 
	           if (x_out[j]>0.0){
		      e=find_error(data+j,x_out[j]);
		      new_e=find_error(data+j,x_out[j]+residual/10.0);
		      printf("%f\t",new_e-e);
		      diff=new_e-e;

		      if (diff<best_error){
			 best_j=j;
			 best_error=diff;
		      }
  		   }
	       printf("\nbest_j=%d\t%f\n",best_j,x_out[best_j]);
	       x_out[best_j]+=residual/10.0;
	   }
	}

	/* find active number of planes and recalc actual compression new_xt */
	cnt=0;
	new_xt=0;
	for (i=0;i<n;i++)
	    if (x_out[i]>0.0){
	       new_xt+=x_out[i];
	       cnt++;
	    }
	    else
		x_out[i]=0.0;

	/* distrubute small residual compression between active planes */
	for (i=0;i<n;i++)
	    if (x_out[i]>0.0)
	       x_out[i]+=(xt-new_xt)/(float)cnt;

	free_float(de);

}

void calculate_gradient
(COEFF_PLANES *data,
 int n)
{
        int i,j;
        float tmp;
 
        /* calcuate all gradient data from previous data */
        for (i=0;i<n;i++){
            for (j=0;j<data[i].nitems-1;j++){
                tmp=(data[i].e[j+1]-data[i].e[j])/(data[i].x[j+1]-data[i].x[j]);
                data[i].de[j]=myabs(tmp);
                data[i].dx[j]=(data[i].x[j+1]+data[i].x[j])/2;
            }
        }
 
}

void smooth_gradient
(COEFF_PLANES *pt)
{
        int i,j,pass,cnt,min;
	float de0,de1,de2,scale,de_old,min_de;
 

	/* make last point smallest */
	min=0;
	min_de=100000;
        for (i=0;i<pt->nitems-1;i++)
	    if (pt->de[i]<min_de){
	       min_de=pt->de[i];
	       min=i;
	    }
	if (min!=pt->nitems-2)
	   pt->de[pt->nitems-2]=pt->de[min]/2;
	
        /* smooth the gradients when necessary */
	cnt=0;
	do {
	  pass=1;
          for (i=1;i<pt->nitems-1;i++){

	      /* skip to next point if next 3 points obey e0>e1>e2>e3 */
	      if (pt->de[i-1]>pt->de[i] 
		    && (pt->de[i]>pt->de[i+1] || i+1>=pt->nitems-1) 
		    && (pt->de[i+1]>pt->de[i+2] || i+2>=pt->nitems-1)) 
	         continue;

	      /* set all varible to zero */
	      scale=0;
	      de0=0;	
	      de1=0;
	      de2=0;
	      de_old=pt->de[i-1];

	      /* work new point out from old point and proceeding 3 points
		 accounting for end of data stream */

	      for (j=0;j<3;j++){
		  if (j+i>=pt->nitems-1)	
		     break;
	   	  switch (j){
		         case 0: de0=pt->de[i]-de_old;
				 scale+=1;
                                 break;
		         case 1: de1=pt->de[i+1]-de_old;
				 scale+=0.5;
                                 break;
		         case 2: de2=pt->de[i+2]-de_old;
				 scale+=0.25;
                                 break;
		  }
	      }

	      pt->de[i]=de_old+(1/scale)*(de0+de1/4+de2/12);
	     
	      /* if a new point is recalcuated - graph does not obej e0>e1....>en
		 for the whole graph therefore need to continue iteration */
	      pass=0;
          }
	  cnt++;
	} while (pass==0 && cnt<20);

}
 
float find_compression
(COEFF_PLANES *pt,
 float grad)
{
	int i;
	float tmp;

	/* check gradient is inside area of knolledge */
	if (pt->de[0]>pt->de[pt->nitems-2]){
	   if (pt->de[0]<grad)
	      return 0;
	   if (pt->de[pt->nitems-2]>grad){
	      tmp=pt->dx[pt->nitems-2]+(pt->de[pt->nitems-2]-grad)
		                       *(pt->dx[pt->nitems-2]/pt->de[pt->nitems-2]);
	      return tmp;
	   }
	}
	else {
	   if (pt->de[0]>grad){
	      tmp=pt->dx[0]+(pt->de[0]-grad)*(pt->dx[0]/pt->de[0]);
	      return tmp;
	   }
	   if (pt->de[pt->nitems-2]<grad)
	      return 0;
	}

	/* search data for nearest match to gradient and interpolate */
	for (i=0;i<pt->nitems-2;i++){
	    if ((pt->de[i]>grad && pt->de[i+1]<grad) || (pt->de[i]<grad && pt->de[i+1]>grad)){
	       tmp=pt->dx[i]+(grad-pt->de[i])*(pt->dx[i+1]-pt->dx[i])
			   		     /(pt->de[i+1]-pt->de[i]);
	       return tmp;
	    }
	    if (pt->de[i]==grad)
	       return pt->dx[i];
	    if (pt->de[i+1]==grad)
	       return pt->dx[i+1];
	}

	printf("error - did not rturn compression \n");
	exit(-1);
}

float find_error
(COEFF_PLANES *pt,
 float x)
{
	int i;
	float tmp;

	/* check compression is inside area of knolledge */
	if (pt->x[0]<pt->x[pt->nitems-2]){
	   if (pt->x[0]>x){
	      tmp=x*(pt->e[0])/(pt->x[0]);
	      return tmp;
	   } 
	   if (pt->x[pt->nitems-2]<x)
	      return pt->e[pt->nitems-2];;
	}
	else {
	   if (pt->x[0]<x)
	      return pt->e[0];;
	   if (pt->x[pt->nitems-2]>x){
	      tmp=x*(pt->e[pt->nitems-2])/(pt->x[pt->nitems-2]);
	      return tmp;
	    }

	}

	/* search data for nearest match to compression and interpolate */
	for (i=0;i<pt->nitems-2;i++){
	    if ((pt->x[i]<x && pt->x[i+1]>x) || (pt->x[i]>x && pt->x[i+1]<x)){
	       tmp=pt->e[i]+(x-pt->x[i])*(pt->e[i+1]-pt->e[i])
			   		     /(pt->x[i+1]-pt->x[i]);
	       return tmp;
	    }
	    if (pt->x[i]==x)
	       return pt->e[i];
	    if (pt->x[i+1]==x)
	       return pt->e[i+1];
	}

	printf("error - did not return error \n");
	exit(-1);
}

float find_gradient
(COEFF_PLANES *pt,
 float x)
{
	int i;
	float tmp;

	/* check compression is inside area of knolledge */
	if (pt->dx[0]<pt->dx[pt->nitems-2]){
	   if (pt->dx[0]>x)
	      return pt->de[0];
	   if (pt->dx[pt->nitems-2]<x)
	      return 0;
	}
	else {
	   if (pt->dx[0]<x)
	      return 0;
	   if (pt->dx[pt->nitems-2]>x)
	      return pt->de[pt->nitems-2];

	}

	/* search data for nearest match to compression and interpolate */
	for (i=0;i<pt->nitems-2;i++){
	    if ((pt->dx[i]<x && pt->dx[i+1]>x) || (pt->dx[i]>x && pt->dx[i+1]<x)){
	       tmp=pt->de[i]+(x-pt->dx[i])*(pt->de[i+1]-pt->de[i])
			   		     /(pt->dx[i+1]-pt->dx[i]);
	       return tmp;
	    }
	    if (pt->dx[i]==x)
	       return pt->de[i];
	    if (pt->dx[i+1]==x)
	       return pt->de[i+1];
	}

	printf("error - did not return gradient \n");
	exit(-1);
}
