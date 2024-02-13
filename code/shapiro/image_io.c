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



/* this function reads the header of an image file, and */
/* gleands from this the iamge details, nameley : */
/* size (rows and columns) and whether or not the */
/* image is colour (colour) */

FILE *read_raw_header
(char *fname,        /* file name of image file to read from */
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


/* this function reads in a ppm (colour) image file from disk and stores it in memory */
/* as a ppm file is RGB, we have to perform an RGB to YUV conversion here */

void get_ppm_file
(FILE *fp,            /* file pointer to file to read image for */
 IMAGE *image_y,      /* stores Y part of new image */
 IMAGE *image_u,      /* stores U part of new image */
 IMAGE *image_v)      /* stores V part of new image */
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
                image_y->pt[image_y->columns*j+i]=limit((int)(Y+0.5));
                image_u->pt[image_u->columns*j+i]=limit((int)(U+128.5));
                image_v->pt[image_v->columns*j+i]=limit((int)(V+128.5));
        }
 
}


/* this function writes a ppm (colour) image file out to disk. As we deal with YUV images, */
/* and a ppm file is RGB, we have to perform a YUV to RGB conversion before */
/* writing the image to disk */

void write_ppm_file
(IMAGE *image_y,      /* Y part of image */
 IMAGE *image_u,      /* U part of image */
 IMAGE *image_v,      /* V part of image */
 char *fname)         /* filename wo wite to */
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
                tmp[0]=(unsigned char)limit(R+0.5);
                tmp[1]=(unsigned char)limit(G+0.5);
                tmp[2]=(unsigned char)limit(B+0.5);

		fwrite(tmp,sizeof(unsigned char),3,fp);
           }
 
        fclose(fp);
}



/* this function reads a pgm (greyscale) image file into memory */

void get_pgm_file
(FILE *fp,         /* pointer to file to read image data from */
 IMAGE *image)     /* where image data is stored in memory */
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

/* this function writes a pgm (greyscale) image file out to disk */

void write_pgm_file
(IMAGE *image,        /* image data */
 char *fname)         /* file name to write data to */
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
