/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                    error handling                            */
/*                      reworked 3/7/96                         */
/*##############################################################*/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "file_io.h"
#include "mem_hand.h"


void allocate_dump
(PACKED *dump,
 int n_bits)
{
        dump->bits=0;
        dump->n_bits=n_bits;

        dump->sym_mask=0x01;
        dump->symbols=malloc_unsigned_char(n_bits/8,"allocate_dump symbols");
        dump->symbols_top=dump->symbols;
	dump->symbols[0]=0;

        dump->ref_mask=0x01;
        dump->refine=malloc_unsigned_char(n_bits/8,"allocate_dump refine");
        dump->refine_top=dump->refine;
	dump->refine[0]=0;

        dump->s=malloc_SYMBOL(1,"allocate dump s");
}



FILE *write_compressed_file_header
(char *fname,
 int  columns,
 int  rows,
 int  colour)
{
        FILE *fp;

        /* get file from disk */
        fp=fopen(fname,"wb");
        NULL_FILE_error(fp,"write_ppm_file");
 
        /* write image stats + comments */
        fprintf(fp,"EZ-WAVELET\n");
        fprintf(fp,"# EZW wavelet compressed image\n");
        fprintf(fp,"%d %d\n",columns,rows);
	fprintf(fp,"%d\n",colour);
        return fp;
}



void write_compressed_file
(FILE *fp,
 IMAGE *image,
 PACKED *dump)
{

        int i,refine_size,symbols_size,overflow;
        unsigned char tmp;
        unsigned char *out;
 
	NULL_FILE_error(fp,"write compressed file");

        /* write threshold to disk  - assumes 4 bytes*/
        tmp=(unsigned char)(dump->t_hold>>24);
        putc(tmp,fp);
        tmp=(unsigned char)(dump->t_hold>>16);
        putc(tmp,fp);
        tmp=(unsigned char)(dump->t_hold>>8);
        putc(tmp,fp);
        tmp=(unsigned char)(dump->t_hold);
        putc(tmp,fp);
 
        /* write symbol stream size to disk - asummes 3 bytes 1:1*/
        symbols_size=dump->symbols-dump->symbols_top;
        tmp=(unsigned char)(symbols_size>>16);
        putc(tmp,fp);
        tmp=(unsigned char)(symbols_size>>8);
        putc(tmp,fp);
        tmp=(unsigned char)(symbols_size);
        putc(tmp,fp);
        
        /* write refine stream size to disk - asummes 3 bytes 1:1*/
        refine_size=dump->refine-dump->refine_top;
        tmp=(unsigned char)(refine_size>>16);
        putc(tmp,fp);
        tmp=(unsigned char)(refine_size>>8);
        putc(tmp,fp);
        tmp=(unsigned char)(refine_size);
        putc(tmp,fp);
 
        /* write overflow to disk - 2 bytes */
        overflow=dump->bits-(refine_size+symbols_size)*8;
        tmp=(unsigned char)(overflow>>8);
        putc(tmp,fp);
        tmp=(unsigned char)(overflow);
        putc(tmp,fp);
        
        /* Total of 16 bytes of header data */
 
        /* write symbol stream to disk */
        out=dump->symbols_top;
        for (i=0;i<symbols_size;i++)
            putc(out[i],fp);
 
        /* write symbol stream to disk */
        out=dump->refine_top;
        for (i=0;i<refine_size;i++)
            putc(out[i],fp);


}


FILE *read_compressed_file_header
(char *fname,
 int  *columns,
 int  *rows,
 int  *colour)
{
        FILE *fp;
        char dummy[256];

        /* get file from disk */
        fp=fopen(fname,"rb");
        NULL_FILE_error(fp,"input image in main");

        /* read bsv line */                        
        fgets(dummy,255,fp);
 
        /* sets up colour */
        if (strncmp(dummy,"EZ-WAVELET",10)){
           printf("file is not EZW wavlet compressed FILE file\n");
           exit(-1);
        }

        /* remove comment lines */
        do {
           fgets(dummy,255,fp);
        } while (strncmp(dummy,"#",1)==0);
 
        /* read in image stats */
        sscanf(dummy,"%d %d",columns,rows);
	fgets(dummy,255,fp);
	sscanf(dummy,"%d", colour);

       return fp;
}



void read_compressed_file
(FILE *fp,
 IMAGE *image,
 PACKED *dump)
{
        int i,refine_size,symbols_size;
        int tmp;
        unsigned char *in;
 
	NULL_FILE_error(fp,"read compressed file");

        /* also allocates mem for image and compressed files */
        /* sets up standard parts of dump*/
        dump->bits=0;
        dump->s=malloc_SYMBOL(1,"read compressed file");
  
        /* allocate memory for image */
        if (image->pt!=NULL)
           free_int(image->pt);
        image->pt=malloc_int(image->columns*image->rows,"read compressed file image");
 
        /* Read threshold to disk  - assumes 4 bytes*/
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        dump->t_hold=tmp;
 
        /* Read symbol stream size to disk - asummes 3 bytes 1:1*/
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        symbols_size=tmp;
 
        /* allocate space for symbol stream */
        dump->symbols=malloc_unsigned_char(symbols_size,"read compressed file symbols");
        dump->symbols_top=dump->symbols;
        dump->sym_mask=0x01;
        dump->n_bits=8*symbols_size;
        
        /* Read refine stream size to disk - asummes 3 bytes 1:1*/
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        refine_size=tmp;
 
        /* allocate space for refine stream */
        dump->refine=malloc_unsigned_char(refine_size,"read compressed file refine");
        dump->refine_top=dump->refine;
        dump->ref_mask=0x01;
        dump->n_bits+=8*refine_size;
 
        /* read overflow */
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        dump->n_bits-=tmp;

 	/* Read Total of 16 bytes of header data */
 
        /* Read symbol stream to disk */
        in=dump->symbols_top;
        for (i=0;i<symbols_size;i++)
            in[i]=getc(fp);
 
        /* Read symbol stream to disk */
        in=dump->refine_top;
        for (i=0;i<refine_size;i++)
            in[i]=getc(fp);

}

