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
#include "struct.h"
#include "mem_hand.h"
#include "image_io.h"

/* macros */
#define limit(x) ((x) > 255 ? 255 : (x < 0 ? 0 :x))

void allocate_IMAGE_space(IMAGE *);

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

#define MAX_SIZE 32
void allocate_IMAGE_space(IMAGE *image)
{

	int rows,columns;

	/* rounds rows and columns to multiple of MAX_SIZE*/
	rows=image->rows;
	if (rows%MAX_SIZE)
	   rows+=(MAX_SIZE-rows%MAX_SIZE);
	
	columns=image->columns;
	if (columns%MAX_SIZE)
	   columns+=(MAX_SIZE-columns%MAX_SIZE);

        if (image->pt!=NULL)
           free_int(image->pt);
	image->pt=malloc_int(rows*columns,"image");

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
	allocate_IMAGE_space(image_y);
 
        /* allocate memory for u image */
	allocate_IMAGE_space(image_u);
 
        /* allocate memory for v image */
	allocate_IMAGE_space(image_v);
 
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
        int i,j,cnt;
	unsigned char tmp[3];
         
        fp=fopen(fname,"wb");
	NULL_FILE_error(fp,"write_ppm_file");

	/* create space for image */
         
        /* write image stats + comments */
        fputs("P6",fp);
        fprintf(fp,"\n# Created by LQDCT Image Compression (daveb 96)#");
        fprintf(fp,"\n%d %d",image_y->columns,image_y->rows);
        fprintf(fp,"\n255\n");                          /* 255 graylevels */
 
        /* write image data */
	cnt=0;
        for (j=0;j<image_y->rows;j++)
            for (i=0;i<image_y->columns;i++){
                /* load up Y/U/V */
                Y=(float)image_y->pt[cnt];
                U=(float)(image_u->pt[cnt]-128);
                V=(float)(image_v->pt[cnt++]-128);
 
                /* convert to RGB */
                R=Y+1.402581*V;
                G=Y-0.344369*U-0.714407*V;
                B=Y+1.773043*U;
 
 
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
	allocate_IMAGE_space(image);
 
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
        fprintf(fp,"\n# Created by LQDCT Image Compression (daveb 96)#");
        fprintf(fp,"\n%d %d",image->columns,image->rows);
        fprintf(fp,"\n255\n");                          /* 255 graylevels */
 
        /* write image data */
        for (i=0;i<image->rows*image->columns;i++){
	    image->pt[i]=limit(image->pt[i]);
            putc(image->pt[i],fp);
        }
 
        fclose(fp);
}
