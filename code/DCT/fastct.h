/*##########################################################################*/
/* fast DCT based on IEEE signal proc, 1992 #8, former yugoslavian authors. */
/* 		Orginally by Barry Sherlock   1993			    */
/* 	   hacked and slashed by David Bethel 21/2/96		            */
/*##########################################################################*/

/* structure for storing the cosines nec for DCT calc */
typedef struct {
	int N;
	int m;
	double two_over_N;
	double root2_over_rootN;
	double *c;

} COSINE;

#define INVROOT2 0.7071067814
#define PI 3.141592654

void bitrev(f,length)
double *f;
int length;
{
  	int i,j,m,half_length;
  	double temp;

	/* No action necessary if n=1 or n=2 */
  	if (length<=2) 
	   return;

  	half_length=length>>1;
  	j=0;
  	for (i=0;i<length;i++){
    	    if (i<j){
      	       temp=f[j];
     	       f[j]=f[i];
      	       f[i]=temp;
   	    }
    	    m=half_length;
    	    while (j>=m){
      	          j-=m;
      	          m=(m+1)>>1;
    	    }
    	    j=j+m;
  	}
}

void inv_sums(f,cosine)
double *f;
COSINE *cosine;
{
  	int ii,stepsize,stage,curptr;
	int nthreads,thread,step,nsteps;

  	for (stage=1;stage<=cosine->m-1;stage++){
    	    nthreads=1<<(stage-1);
    	    stepsize=nthreads<<1;
    	    nsteps=(1<<(cosine->m-stage)) - 1;
    	    for (thread=1;thread<=nthreads;thread++){
      	        curptr=cosine->N-thread; 
      	        for (step=1;step<=nsteps;step++){
        	    f[curptr]+=f[curptr-stepsize];
        	    curptr-=stepsize; 
      	        }
    	    }
  	}
}

void fwd_sums(f,cosine)
double *f;
COSINE *cosine;
{
  	int ii,stepsize,stage,curptr;
	int nthreads,thread,step,nsteps;

  	for (stage=cosine->m-1;stage>=1;stage--){
    	    nthreads=1<<(stage-1);
    	    stepsize=nthreads<<1;
    	    nsteps=(1<<(cosine->m-stage))-1;
    	    for (thread=1;thread<=nthreads;thread++){
      	        curptr=nthreads+thread-1;
      	        for (step=1;step<=nsteps;step++){
	            f[curptr]+=f[curptr+stepsize];
	            curptr+=stepsize;
      	        }
    	    }
  	}
}

void scramble(f,length)
double *f;
int length;
{

  	double temp;
  	int i,ii1,ii2,half_length,qtr_length;

  	half_length=length >> 1;
  	qtr_length=half_length >> 1;

  	bitrev(f,length);
  	bitrev(&f[0], half_length);
  	bitrev(&f[half_length], half_length);

  	ii1=length-1;
  	ii2=half_length;

  	for (i=0;i<qtr_length;i++){
    	    temp=f[ii1];
    	    f[ii1]=f[ii2];
    	    f[ii2]=temp;
    	    ii1--;
    	    ii2++;
 	}
}

void unscramble(f,length)
double *f;
int length;
{
  	double temp;
  	int i,ii1,ii2,half_length,qtr_length;

  	half_length=length >> 1;
  	qtr_length=half_length >> 1;
  	ii1=length-1;
  	ii2=half_length;

  	for (i=0;i<qtr_length;i++){
    	    temp=f[ii1];
    	    f[ii1]=f[ii2];
    	    f[ii2]=temp;
    	    ii1--;
    	    ii2++;
  	}

  	bitrev(&f[0],half_length);
  	bitrev(&f[half_length],half_length);
  	bitrev(f,length);

}

void init_cosine_array(cosine,length)
COSINE *cosine;
int length;
{
  	int i,group,base;
	int item,nitems,halfN;
  	double factor;

  	printf("FCT-- new N=%d\n",length);

	/* calculate m in length=2^n */
	for (i=1,cosine->m=0;i<length;i*=2)
	    cosine->m++;
    	cosine->N=1<<cosine->m;

	/* check that N is a power of 2 */ 
    	if (cosine->N!=length){
      	   printf("ERROR in FCT-- length %d not a power of 2\n",length);
     	   exit(1);
	}

  	if (cosine->c!=NULL) 
	   free(cosine->c);
  	cosine->c=(double *)calloc(cosine->N,sizeof(double));

  	if (cosine->c==NULL){
    	   printf("Unable to allocate c array\n");
    	   exit(1);
  	}

  	halfN=cosine->N/2;
  	cosine->two_over_N = 2.0/(double)(cosine->N);
  	cosine->root2_over_rootN = sqrt(2.0/(double)(cosine->N));

  	for (i=0;i<=halfN-1;i++) 
	    cosine->c[halfN+i]=4*i+1;

  	for (group=1;group<=cosine->m-1;group++){

    	    base=1<<(group-1);
    	    nitems=base;
    	    factor=1.0*(1<<(cosine->m-group));
    	    for (item=1;item<=nitems;item++) 
	        cosine->c[base+item-1]=factor*cosine->c[halfN+item-1];
  	}

  	for (i=1;i<cosine->N;i++) 
	    cosine->c[i]=1.0/(2.0*cos(cosine->c[i]*PI/(2.0*cosine->N)));

}

void inv_butterflies(f,cosine)
double *f;
COSINE *cosine;
{
  	int stage,ii1,ii2,butterfly;
	int ngroups,group,wingspan,increment,baseptr;
  	double cfac,T;

  	for (stage=1; stage<=cosine->m;stage++){
    	    ngroups=1<<(cosine->m-stage);
    	    wingspan=1<<(stage-1);
    	    increment=wingspan<<1;
    	    for (butterfly=1; butterfly<=wingspan; butterfly++){
      	        cfac=cosine->c[wingspan+butterfly-1];
      	        baseptr=0;
      	        for (group=1;group<=ngroups;group++){
		    ii1=baseptr+butterfly-1;
		    ii2=ii1+wingspan;
		    T=cfac*f[ii2];
		    f[ii2]=f[ii1]-T;
		    f[ii1]=f[ii1]+T;
		    baseptr += increment;
      	        }
    	    }
	}
}

void fwd_butterflies(f,cosine)
double *f;
COSINE *cosine;
{
  	int stage,ii1,ii2,butterfly;
	int ngroups,group,wingspan,increment,baseptr;
  	double cfac,T;

  	for (stage=cosine->m;stage>=1;stage--){
    	    ngroups=1<<(cosine->m-stage);
    	    wingspan=1<<(stage-1);
    	    increment=wingspan<<1;
    	    for (butterfly=1;butterfly<=wingspan;butterfly++){
      	        cfac=cosine->c[wingspan+butterfly-1];
      	        baseptr=0;
      	        for (group=1;group<=ngroups;group++){
		    ii1=baseptr+butterfly-1;
		    ii2=ii1+wingspan;
		    T=f[ii2];
		    f[ii2]=cfac*(f[ii1]-T);
		    f[ii1]=f[ii1]+T;
		    baseptr += increment;
      	        }
    	    }
  	}
}

void ifct_noscale(f,cosine,length)
double *f; 
COSINE *cosine;
int length;
{
  	if (length != cosine->N) 
	   init_cosine_array(cosine,length);

  	f[0]*=INVROOT2;
  	inv_sums(f,cosine);
  	bitrev(f,cosine->N);
  	inv_butterflies(f,cosine);
  	unscramble(f,cosine->N);
}

void fct_noscale(f,cosine,length)
double *f; 
COSINE *cosine;
int length;
{
  	if (length!=cosine->N) 
	   init_cosine_array(cosine,length);

  	scramble(f,cosine->N);
  	fwd_butterflies(f,cosine);
  	bitrev(f,cosine->N);
  	fwd_sums(f,cosine);
  	f[0] *= INVROOT2; 
}

void ifct(f,cosine,length)
double *f; 
COSINE *cosine;
int length;
{
	/* CALL THIS FOR INVERSE 1D DCT DON-MONRO PREFERRED SCALING */
  	int i;

  	for (i=0;i<cosine->N;i++) 
	    f[i] *= cosine->root2_over_rootN;

  	ifct_noscale(f,cosine,length);
}

void fct(f,cosine,length)
double *f;
COSINE *cosine;
int length;
{
        /* CALL THIS FOR FORWARD 1D DCT DON-MONRO PREFERRED SCALING */
  	int i;

  	fct_noscale(f,cosine,length);
  	for (i=0;i<cosine->N;i++) 
	    f[i] *= cosine->root2_over_rootN;
}

