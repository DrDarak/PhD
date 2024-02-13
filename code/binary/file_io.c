/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                    file input / ouput                        */
/*                      reworked 28/8/96                        */
/*			modifiewd to binary 19/11/96		*/
/*##############################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mem_hand.h"
#include "file_io.h"

void allocate_dump
(PACKED *dump,
 int n_bits)
{
 
 
        dump->sym_mask=0x01;
        dump->symbols=malloc_unsigned_char((n_bits/8),"allocate dump");
        dump->symbols_top=dump->symbols;
        dump->symbols[0]=0; /* clear 1st symbol for encode */
	dump->bits=0;
	dump->max_bits=n_bits;
 
}

 
void write_compressed_file
(FILE *fp,
 IMAGE *image,
 PACKED *qtree,
 int comp)
{
 
        int out_size;
        unsigned char tmp;
        unsigned char *out;

	/* check for valid file pt */
	NULL_FILE_error(fp,"write compressed file"); 
 
	fprintf(fp,"BATH BINARY FILE\n");
	fprintf(fp,"# created by bath binary compressor (daveb) 96\n");
	fprintf(fp,"%d %d\n",image->columns,image->rows);
	fprintf(fp,"%d\n",comp);
 
        /* output size of compressed streams */
        /*quad tree */
        out_size=qtree->symbols-qtree->symbols_top+1;
        tmp=(unsigned char)(out_size>>16);
        putc(tmp,fp);
        tmp=(unsigned char)(out_size>>8);
        putc(tmp,fp);
        tmp=(unsigned char)(out_size);
        putc(tmp,fp);

        /* output compressed streams */
        /* qtree*/
        out_size=qtree->symbols-qtree->symbols_top+1;
        out=qtree->symbols_top;
	fwrite(out,sizeof(unsigned char),out_size,fp);
              
              
}

 
int read_compressed_file_header
(FILE *fp,
 IMAGE *image)
{
	int comp;
        char dummy[256];

        NULL_FILE_error(fp,"input compressed file");

	/* remove FILE ID line */
        fgets(dummy,255,fp);

        /* sets up colour */
        if (strncmp(dummy,"BATH BINARY FILE",16)){
           printf("file is not BATH BINARY FILE file\n");
           exit(-1);
        }

        /* remove comment lines */
        do {
           fgets(dummy,255,fp);
        } while (strncmp(dummy,"#",1)==0);

        /* read in image stats */
        sscanf(dummy,"%d %d",&(image->columns),&(image->rows));

        /* read image compresion */
        fgets(dummy,255,fp);
        sscanf(dummy,"%d",&comp);

	return comp;

}

void read_compressed_file
(FILE *fp,
 PACKED *qtree)
{
        int qtree_size;
        int tmp;
        unsigned char *in;
         
	/* check for NULL file pt */
	NULL_FILE_error(fp,"read compressed file"); 

        /* find nitems in streams */
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        qtree_size=tmp;
        allocate_dump(qtree,qtree_size*8);
 
        /* read in streams */
        in=qtree->symbols_top;
	fread(in,sizeof(unsigned char),qtree_size,fp);
              
}
