/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                    file input / ouput                        */
/*                      reworked 28/8/96                        */
/*##############################################################*/

#include <stdio.h>
#include <math.h>
#include <string.h>
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
 HUFF *qtree,
 int size)
{
 
        int out_size;
        unsigned char tmp;
        unsigned char *out;

	/* check for valid file pt */
	NULL_FILE_error(fp,"write compressed file"); 
 
        /* ouput image size */
        tmp=(unsigned char)(image->columns>>8);
        putc(tmp,fp);
        tmp=(unsigned char)(image->columns);
        putc(tmp,fp);
 
        tmp=(unsigned char)(image->rows>>8);
        putc(tmp,fp);
        tmp=(unsigned char)(image->rows);
        putc(tmp,fp);
 
        /* output size of compressed streams */
 
        /*quad tree */
        out_size=qtree->dump->symbols-qtree->dump->symbols_top+1;
        tmp=(unsigned char)(out_size>>8);
        putc(tmp,fp);
        tmp=(unsigned char)(out_size);
        putc(tmp,fp);
 
        /* output compressed streams */
        /* qtree*/
        out_size=qtree->dump->symbols-qtree->dump->symbols_top+1;
        out=qtree->dump->symbols_top;
	fwrite(out,sizeof(unsigned char),out_size,fp);
              
}

 
void read_compressed_file
(FILE *fp,
 IMAGE *image,
 HUFF *qtree,
 int size)
{
        int qtree_size,in_size;
        int tmp;
        unsigned char *in;
 
         
	/* check for NULL file pt */
	NULL_FILE_error(fp,"read compressed file"); 

        /* read in image size */
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        image->columns=tmp;
 
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        image->rows=tmp;
 
        /* find nitems in streams */
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        qtree_size=tmp;
        allocate_dump(qtree->dump,qtree_size*8);
 
        /* read in streams */
        in=qtree->dump->symbols_top;
	fread(in,sizeof(unsigned char),qtree_size,fp);
              
}
