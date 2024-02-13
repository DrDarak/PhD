/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/* 		      image input/output			*/
/*			reworked 2/7/96				*/
/* 		   changed to block loading Oct 96 		*/
/*##############################################################*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mem_hand.h"
#include "image_io.h"

FILE *read_pgm_header
(char *fname,
 IMAGE *image)
{
	FILE *fp;
	char dummy[256];

	/* get file from disk */
        fp=fopen(fname,"rb");
        NULL_FILE_error(fp,"input image in main");
 
        /* remove P5/6 line */
        fgets(dummy,255,fp);

        /* sets up colour */
        if (strncmp(dummy,"P5",2)){
           printf("file is not PGM (P5) file\n");
           exit(-1);
	}
 
        /* remove comment lines */
        do {
           fgets(dummy,255,fp);
        } while (strncmp(dummy,"#",1)==0);
 
        /* read in image stats */          
        sscanf(dummy,"%d %d",&(image->columns),&(image->rows));

	/* remove 255 line */
        fgets(dummy,255,fp);

	return fp;
}

FILE *write_pgm_header
(char *fname,
 IMAGE *image)
{
        FILE *fp;
 
        /* get file from disk */
        fp=fopen(fname,"wb");
        NULL_FILE_error(fp,"output image in main");

        /* write image stats + comments */
        fputs("P5",fp);
        fprintf(fp,"\n# Created by binary Image Compression (daveb 96)#");
        fprintf(fp,"\n%d %d",image->columns,image->rows);
        fprintf(fp,"\n255\n");                          /* 255 graylevels */

        return fp;

}

int read_pgm_file
(FILE *fp,
 IMAGE *image,
 int rows)
{
        /* Reads image in Raw format*/
        int nitems,i;
 
        /* allocate memory for image */
        if (image->pt!=NULL)
           free_unsigned_char(image->pt);
	image->pt=malloc_unsigned_char(rows*image->columns,"get rows from image ");
 
        /* read in image data */
	nitems=fread(image->pt,sizeof(unsigned char),rows*image->columns,fp);

	for (i=0;i<nitems;i++)
	    if (image->pt[i]>128)
	       image->pt[i]=1;
	    else
	        image->pt[i]=0;

	return nitems;
}

void write_pgm_file
(FILE *fp,
 IMAGE *image,
 int rows)
{
	int i;
 
	for (i=0;i<image->columns*rows;i++)
	    if (image->pt[i])
	       image->pt[i]=255;
	    else
	        image->pt[i]=0;

        /* write image data */
	fwrite(image->pt,sizeof(unsigned char),rows*image->columns,fp);
 
}
