/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/* 		      image input/output			*/
/*			reworked 2/7/96				*/
/*##############################################################*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "image_io.h"

FILE *read_raw_header
(char *fname,
 int  *rows,
 int  *columns,
 int *colour)
{
	FILE *fp;
	char dummy[256];

	/* get file from disk */
        fp=fopen(fname,"rb");
        NULL_FILE_error(fp,"input image in main");
 
        /* remove P5/6 line */
        fgets(dummy,255,fp);

        /* sets up colour */
        if (!strncmp(dummy,"P5",2))
           *colour=0;
        else if (!strncmp(dummy,"P6",2))
                *colour=1;
             else {
                  printf("file is not PGM/PPM (P5/6) file\n");
                  exit(-1);
             }
 
        /* remove comment lines */
        do {
           fgets(dummy,255,fp);
        } while (strncmp(dummy,"#",1)==0);
 
        /* read in image stats */          
        sscanf(dummy,"%d %d",columns,rows);

	/* remove 255 line */
        fgets(dummy,255,fp);

	return fp;
}


void get_ppm_file
(FILE *fp,
 IMAGE *image_y,
 IMAGE *image_u,
 IMAGE *image_v)
{
 
        /* Reads colour image in Raw format*/
        int i,j;
	unsigned char tmp[3];
        float R,G,B;
        float Y,U,V;
 
        /* write stats into v/u images - sub sampled */
        image_u->columns=image_y->columns;
        image_u->rows=image_y->rows;
 
        image_v->columns=image_y->columns;
        image_v->rows=image_y->rows;
 
        /* allocate memory for y image */
        if (image_y->pt!=NULL)
           free_int(image_y->pt);
	image_y->pt=malloc_int(image_y->rows*image_y->columns,"get_ppm_file y file");
 
        /* allocate memory for u image */
        if (image_u->pt!=NULL)
           free_int(image_u->pt);
	image_u->pt=malloc_int(image_u->rows*image_u->columns,"get_ppm_file u file");
 
        /* allocate memory for v image */
        if (image_v->pt!=NULL)
           free_int(image_v->pt);
	image_v->pt=malloc_int(image_v->rows*image_v->columns,"get_ppm_file v file");
 
        /* read in image data -> file is 3 times as large as image (RGB)*/
        for (j=0;j<image_y->rows;j++)
            for (i=0;i<image_y->columns;i++){
                /* read in Red / Blue / Green */
	        fread(tmp,sizeof(unsigned char),3,fp);
                R=(float)tmp[0];
                G=(float)tmp[1];
                B=(float)tmp[2];
 
                /* convert to YUV */
                Y= 0.2990*R+0.5870*G+0.1140*B;
                U=-0.1686*R-0.3311*G+0.4997*B;
                V= 0.4998*R-0.4185*G-0.0813*B;
 
                /* load into image files */
                image_y->pt[image_y->columns*j+i]=limit((int)rint(Y));
                image_u->pt[image_u->columns*j+i]=limit((int)rint(U+128.0));
                image_v->pt[image_v->columns*j+i]=limit((int)rint(V+128.0));
        }
 
}

void write_ppm_file
(IMAGE *image_y,
 IMAGE *image_u,
 IMAGE *image_v,
 char *fname)
{
 
        FILE *fp;
        float R,G,B;
        float Y,U,V;
        int i,j;
	unsigned char tmp[3];
         
        fp=fopen(fname,"wb");
	NULL_FILE_error(fp,"write_ppm_file");
         
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
                U=(float)(image_u->pt[image_u->columns*j+i]-128);
                V=(float)(image_v->pt[image_v->columns*j+i]-128);
 
                /* convert to RGB */
                R=Y+0.000060*U+1.402581*V;
                G=Y-0.344369*U-0.714407*V;
                B=Y+1.773043*U-0.000130*V;
 
 
                /* save to file */
                tmp[0]=(unsigned char)limit(rint(R));
                tmp[1]=(unsigned char)limit(rint(G));
                tmp[2]=(unsigned char)limit(rint(B));

		fwrite(tmp,sizeof(unsigned char),3,fp);
           }
 
        fclose(fp);
}

void get_pgm_file
(FILE *fp,
 IMAGE *image)
{
        /* Reads image in Raw format*/
        int i;
 
        /* allocate memory for image */
        if (image->pt!=NULL)
           free_int(image->pt);
	image->pt=malloc_int(image->rows*image->columns,"get_pgm_file y file");
 
        /* read in image data */
        for (i=0;i<image->rows*image->columns;i++)
            image->pt[i]=getc(fp);
 
}

void write_pgm_file
(IMAGE *image,
 char *fname)
{
        FILE *fp;
        int i;
 
        fp=fopen(fname,"wb");
	NULL_FILE_error(fp,"write_pgm_file");
 
        /* write image stats + comments */
        fputs("P5",fp);
        fprintf(fp,"\n# Created by Wavelet Image Compression (daveb 96)#");
        fprintf(fp,"\n%d %d",image->columns,image->rows);
        fprintf(fp,"\n255\n");                          /* 255 graylevels */
 
        /* write image data */
        for (i=0;i<image->rows*image->columns;i++){
	    image->pt[i]=limit(image->pt[i]);
            putc(image->pt[i],fp);
        }
 
        fclose(fp);
}
