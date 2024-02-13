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
#include "struct.h"
#include "mem_hand.h"
#include "huff.h"
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

void allocate_packed_streams
(BASIS *top,
 HUFF *frac,
 int n_blocks,
 int bits)
{
 
	int i;
	float *bpc,bpb;
	BASIS *basis;

	bpc=malloc_float(top->nitems,"allocate packed straems");
        
	/* find approximate bits required for store image */
 	basis=top; 
	bpb=0;
	i=0;
        while (basis!=NULL){
              bpc[i]=huffman_bpc(basis->huff); 
	printf("%f\n",bpc[i]);
	      bpb+=bpc[i];
	      i++;
              basis=basis->next;
        }

#ifdef FRACTAL
	bpb+=huffman_bpc(frac);
#endif

	/* make sure there are enough bits to cover lowest blocking */
	if ((int)(bpb*(float)n_blocks)>bits){
	   printf("WARNING: Bits increased to cover image \n");
	   bits=(int)(bpb*(float)n_blocks);
	}
 
	/* allocateing stream */
 	basis=top; 
        allocate_dump(basis->huff->dump,bits*2);  
	
	/* used for seperat streams
	basis=top;
	i=0;
        while (basis!=NULL){
	      bits_coeff=(int)((bpc[i++]/bpb)*(float)bits);
              allocate_dump(basis->huff->dump,bits_coeff*2);  
              basis=basis->next;
        }
	*/

	free_float(bpc);
 
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
        fprintf(fp,FILE_TYPE);
        fprintf(fp,"\n");
        fprintf(fp,"# DCT compressor\n");
        fprintf(fp,"%d %d\n",columns,rows);
        fprintf(fp,"%d\n",colour);
        return fp;
}

 
void write_compressed_file
(FILE *fp,
 IMAGE *image,
 BASIS *basis,
 HUFF *qtree,
 int size)
{
 
        int out_size;
        unsigned char tmp;
        unsigned char *out;

	/* check for valid file pt */
	NULL_FILE_error(fp,"write compressed file"); 
 
        /* output size of compressed streams */
        /*quad tree */
        out_size=qtree->dump->symbols-qtree->dump->symbols_top+1;
        tmp=(unsigned char)(out_size>>8);
        putc(tmp,fp);
        tmp=(unsigned char)(out_size);
        putc(tmp,fp);
 
        /* basis vectors */
        out_size=basis->huff->dump->symbols-basis->huff->dump->symbols_top+1;
        tmp=(unsigned char)(out_size>>16);
        putc(tmp,fp);
        tmp=(unsigned char)(out_size>>8);
        putc(tmp,fp);
        tmp=(unsigned char)(out_size);
        putc(tmp,fp);
 
        /* output compressed streams */
        /* qtree*/
        out_size=qtree->dump->symbols-qtree->dump->symbols_top+1;
        out=qtree->dump->symbols_top;
	fwrite(out,sizeof(unsigned char),out_size,fp);
              
        /* basis vectors */
        out=basis->huff->dump->symbols_top;
        out_size=basis->huff->dump->symbols-basis->huff->dump->symbols_top+1;
	fwrite(out,sizeof(unsigned char),out_size,fp);
 
              
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
        if (strncmp(dummy,FILE_TYPE,4)){
           printf("file is not " FILE_TYPE " compressed file type\n");
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
 BASIS *basis,
 HUFF *qtree,
 int size)
{
        int qtree_size,in_size;
        int tmp;
        unsigned char *in;
 
         
	/* check for NULL file pt */
	NULL_FILE_error(fp,"read compressed file"); 

        /* find nitems in streams */
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        qtree_size=tmp;
        allocate_dump(qtree->dump,qtree_size*8);
 
	/* find nitems in coeff stream */
        tmp=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        tmp<<=8;
        tmp|=getc(fp);
        in_size=tmp;
        allocate_dump(basis->huff->dump,in_size*8);
 
        /* read in streams */
        in=qtree->dump->symbols_top;
	fread(in,sizeof(unsigned char),qtree_size,fp);
              
        in=basis->huff->dump->symbols_top;
	fread(in,sizeof(unsigned char),in_size,fp);
              
}
