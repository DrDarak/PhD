#include <stdio.h>
#include <math.h>


main()
{
	double N=2;
	double PI=3.141592654;
	int i,j;
	

	printf("\tblock->c[1]=\n");
	for (i=0;i<(int)N/2;i++)
	    printf("\t           +%1.8f*(float)cos10[%d]\n",
				cos(PI*((double)i+0.5)/N)*sqrt(2)/N,i);
	printf("\n");

	printf("\tblock->c[1]=\n");
	for (i=0;i<(int)N/2;i++)
	    printf("\t           +%1.8f*(float)cos01[%d]\n",
				cos(PI*((double)i+0.5)/N)*sqrt(2)/N,i);
	printf("\n");

	printf("\tblock->c[1]=\n");
	for (i=0;i<(int)N/4;i++)
	    printf("\t           +%1.8f*(float)cos20[%d]\n",
				cos(2.0*PI*((double)i+0.5)/N)*sqrt(2)/N,i);
	printf("\n");

	printf("\tblock->c[1]=\n");
 	for (i=0;i<(int)N/4;i++)
            printf("\t           +%1.8f*(float)cos02[%d]\n",
                                cos(2.0*PI*((double)i+0.5)/N)*sqrt(2)/N,i);
	printf("\n");

	printf("\tblock->c[1]=\n");
	for (j=0;j<(int)N/2;j++)
	    for (i=j;i<(int)N/2;i++)
	        printf("\t           +%1.8f*(float)(cos11[%d]+cos11[%d])\n",
			cos(PI*((double)i+0.5)/N)*cos(PI*((double)j+0.5)/N)*2.0/N,
				(int)N*j/2+i,(int)N*i/2+j);

}
