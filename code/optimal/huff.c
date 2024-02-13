/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                    Huffman encoder                           */
/*                      reworked 29/8/96                        */
/*##############################################################*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "huff.h"

typedef enum pdf_type {normal,blank} PDF_TYPE;

/*------------------------------------------*/
/* sets up huffman structures and tree to   */
/* allow efficien huffman coding 	    */
/*------------------------------------------*/
/* FILE STRUCTURE 	

PDF[_CREATE]
# N comments
min\tmax\trun\n
1
1
(max-min+run+1) long
:
:
END_PDF

*/

void setup_huffman_tree
(HUFF *huff,
 PACKED *dump,
 FILE *fp)
{
        int i,run,max,min,n;
	PDF_TYPE type;
	char dummy[256];

	/* default seting */
	type=normal;

	/* check files is ok to use */
	NULL_FILE_error(fp,"setup huffman tree");

	/* attach stream pointer */
	huff->dump=dump;

	/* take a line from file */
	fgets(dummy,255,fp);
	if (strncmp(dummy,"PDF",3)){
	     printf("PDF tables read error\n");
	     exit(-1);
	}
	/* sets flag to create blank pdf tabels */
	if (!strncmp(dummy,"PDF_CREATE",10))
	   type=blank;

	/* remove comment lines */
        do {
           fgets(dummy,255,fp);
        } while (strncmp(dummy,"#",1)==0);

	/* load max/min from file */
        sscanf(dummy,"%d\t%d\t%d\n",&min,&max,&run);

	/* store max / min in packed arry */
        huff->min=min;
        huff->max=max;
        huff->run=run;
 
        /* set up huff symbol stream */
        huff->huff=malloc_LEAF((max-min+1+run),"setup huff tree");
 
        /* read huffman tree link list from file */
        for (i=min;i<max;i++){
	    /* read pdf in or set to 1 */
	    if (type==normal)
               fscanf(fp,"%d\n",&n);
	    else
		n=1;
	    /* setup the leaf link list */
            setup_leaf(&(huff->huff[i-min]),n,i);
            huff->huff[i-min].orig=&(huff->huff[i-min+1]);
            huff->huff[i-min].next=huff->huff[i-min].orig;
        }     
	if (run>0){
	   /* read in run lengths 1->run and max symbol */
           for (i=0;i<run;i++){
	       /* read pdf in or set to 1 */
	       if (type==normal)
                  fscanf(fp,"%d\n",&n);
	       else
		   n=1;

	       /* setup the leaf link list */
	       if (i==0)
                  setup_leaf(&(huff->huff[i+max-min]),n,max);
	       else {
                   setup_leaf(&(huff->huff[i+max-min]),n,i);
		   huff->huff[i+max-min].run=1;
	       }

               huff->huff[i+max-min].orig=&(huff->huff[i+max-min+1]);
               huff->huff[i+max-min].next=huff->huff[i+max-min].orig;
	   }
	   if (type==normal)
              fscanf(fp,"%d\n",&n);
	   else
	       n=1;
           setup_leaf(&(huff->huff[run+max-min]),n,run);
	}
	else {
	     if (type==normal)
                fscanf(fp,"%d\n",&n);
	     else
	         n=1;
             setup_leaf(&(huff->huff[i-min]),n,i);
	}

	/* must terminate pdf data with END_PDF .. provents error overflow*/
        fgets(dummy,255,fp);
	if (strncmp(dummy,"END_PDF",7)){
	     printf("PDF tables read error\n");
	     exit(-1);
	}

        /* initialise huffman tables */                 
        huff->huff_top=construct_huff_tree(huff->huff);
 
}

/*------------------------------------------*/
/* Writes a flat pdf to the standard pdf    */
/* file on disk. Use for reformatting the   */
/* pdf tables				    */
/*------------------------------------------*/
 
void wipe_huffman_pdf
(HUFF *huff,
 FILE *fp)
{
 
      int i;
 
	fprintf(fp,"PDF\n");
	fprintf(fp,"# pdf wipe by program\n");
        fprintf(fp,"%d\t%d\t%d\n",huff->min,huff->max,huff->run);
        for (i=huff->min;i<=huff->max+huff->run;i++)
            fprintf(fp,"%d\n",1);
	fprintf(fp,"END_PDF\n");
 
}

/*------------------------------------------*/
/* Writes an existing pdf to standard pdf   */   
/* file on disk. Use for training the       */   
/* pdf tables                               */   
/*------------------------------------------*/

void save_huffman_pdf
(HUFF *huff,
 FILE *fp)
{

        int i,max,half_int,scale;
	
	/* find max entery in the list */
	max=0;
	for (i=huff->min;i<=huff->max+huff->run;i++){
	    if (huff->huff[i-huff->min].nitems>max)
	       max=huff->huff[i-huff->min].nitems;
	    if (huff->huff[i-huff->min].nitems<1){
	       printf("Integer overflow, can not append huffman tables\n");
	       return;
	    }
	}

	/* may varry with platform */
	half_int=(1<<16);
	scale=max/half_int;
	if (scale<2)
	   scale=2;
	
	/* reduce all enteries in table to maintain integers */
	if (max>half_int){
	   printf("Modifying huffman tables\n");
	   for (i=huff->min;i<=huff->max+huff->run;i++) 
	       if (huff->huff[i-huff->min].nitems/scale>1)
	           huff->huff[i-huff->min].nitems/=scale;
	}

	/* write out pdftable to file */
	fprintf(fp,"PDF\n");
	fprintf(fp,"# pdf appended by program\n");
        fprintf(fp,"%d\t%d\t%d\n",huff->min,huff->max,huff->run);
        for (i=huff->min;i<=huff->max+huff->run;i++)
            fprintf(fp,"%d\n",huff->huff[i-huff->min].nitems);
	fprintf(fp,"END_PDF\n");

}                   

/*------------------------------------------*/
/* used to close down huffman coder         */
/* encoding and decoding                    */
/*------------------------------------------*/
 
void free_huffman_tree
(HUFF *huff)
{

	free_tree(huff->huff_top); /* frees huffman tree */
	free_LEAF(huff->huff); /* frees linear list of symbols */

}

/*------------------------------------------*/
/* used to Free the huffman tree form for   */
/* encoding and decoding 		    */
/*------------------------------------------*/

void free_tree
(LEAF *huff_pt)
{
	
	if (huff_pt->zero_pt==NULL)
	   return ; /* MUST not free here (free linear array as a whole) */

        free_tree(huff_pt->zero_pt);
        free_tree(huff_pt->one_pt);

	free_LEAF(huff_pt);

	return ;
}

/*------------------------------------------*/
/* Estimates from PDF average bits per	    */
/* coefficent for a particular stream dump  */
/*------------------------------------------*/

float huffman_bpc
(HUFF *huff)
{
	int i,nitems,bits;
	float entropy;

	nitems=0;
	bits=0;
	for (i=huff->min;i<=huff->max;i++){
            nitems+=huff->huff[i-huff->min].nitems;
	    bits+=huff->huff[i-huff->min].nitems*huff->huff[i-huff->min].length;
	}
	
	entropy=(float)bits/(float)nitems;

	return entropy;

}

/*------------------------------------------*/
/* This routine takes a list of symbols	    */
/* with their associated number of 	    */
/* occurences (pdf) and forms the huffman   */
/* tree. The huffman symbols and lengths    */
/* are placed in the original list and the  */
/* top of the tree (for decode) is returned */
/*------------------------------------------*/

LEAF * construct_huff_tree
(LEAF *huff_top)
{
	LEAF *huff_pt,*first_pt,*second_pt;

	/* scan through data to creat huff tree */
	while(huff_top->next!=NULL){
	      /* search list for min values and delete enteries from list*/
	      first_pt=search_list(huff_top);
	      huff_top=delete_element(first_pt,huff_top);

	      second_pt=search_list(huff_top);
	      huff_top=delete_element(second_pt,huff_top);

	      /* create space for new data combined from previous 2 items*/
	      huff_pt=allocate_leaf_mem(first_pt->nitems+second_pt->nitems,-1);

	      /* setup tree pointers */
	      huff_pt->zero_pt=first_pt;
	      huff_pt->one_pt=second_pt;

	      /* append data to top of list */
	      huff_pt->next=huff_top;
	      huff_top=huff_pt;
	}

	/* asigns symbols and lengths to input data */
	traverse_tree(huff_top,0,0);
	
	return huff_top;

}

/*------------------------------------------*/
/* This routine takes the tree and assignes */
/* the appropriate huffman  symbols to the  */
/* original root list			    */				    
/*------------------------------------------*/

void traverse_tree
(LEAF *huff_pt,
 int symbol,
 int length)
{
	if (huff_pt->zero_pt==NULL ){
	   huff_pt->length=length;
	   huff_pt->symbol=symbol;
	   if (length>=sizeof(int)*8){
	      printf("interger overflow in huffman tables lengths\n");
	      exit(-1);
	   }
	   return ;
	}
        traverse_tree(huff_pt->zero_pt,symbol,length+1);
        traverse_tree(huff_pt->one_pt,symbol | (1<<length),length+1);

	return ;
}

/*------------------------------------------*/
/* This removes items from the list that is */
/* scan to form the huffman tree BUT does   */
/* not free them as they are used as nodes  */
/* in the final tree			    */
/*------------------------------------------*/

LEAF * delete_element
(LEAF *del_pt,
 LEAF *huff_top)
{
	LEAF *huff_pt,*last_pt;

	/* return top pointer if it has changed */
	huff_pt=huff_top;
	if (del_pt==huff_top){
	   /* mends the hole in the list */
	   huff_top=huff_pt->next;
	   huff_pt->next=NULL;
	   return huff_top;
	}

	/* increament to next position */
	while (huff_pt!=NULL){
	      last_pt=huff_pt;
	      huff_pt=last_pt->next;
	      if (huff_pt==del_pt){
		 /* mends the hole in the list */
		 last_pt->next=huff_pt->next;
		 huff_pt->next=NULL;
		 return huff_top;
	      }
	}

	printf("error in delete element\n");
	exit(-1);

}

/*------------------------------------------*/
/* This routine searchs the huff list to    */
/* the largest nitems in one symbol and     */
/* returns the pointer to that symbol 	    */
/*------------------------------------------*/

LEAF * search_list
(LEAF *huff_pt)
{
	LEAF *min_pt;
	int min;

	min_pt=huff_pt;
	min=1024*1024; /* large image likely to be encountered */
	/* scan active list */
	while (huff_pt!=NULL){
	      if (huff_pt->nitems<min){
		 min=huff_pt->nitems;
		 min_pt=huff_pt;
	      }
	      huff_pt=huff_pt->next;

	}

	return min_pt;

}

/*------------------------------------------*/
/* Allocates memoery for a huffman leaf */
/*------------------------------------------*/

LEAF * allocate_leaf_mem
(int nitems,
 int index)
{
	LEAF *huff_pt;

	huff_pt=malloc_LEAF(1,"allocate leaf");

	setup_leaf(huff_pt,nitems,index);

	return huff_pt;

}

/*------------------------------------------*/
/* sets leaf to inital conditions 	    */
/*------------------------------------------*/

void setup_leaf
(LEAF *huff_pt,
 int nitems,
 int index)
{
	huff_pt->next=NULL;
	huff_pt->orig=NULL;
	huff_pt->zero_pt=NULL;
	huff_pt->one_pt=NULL;
	huff_pt->nitems=nitems;
	huff_pt->index=index;
	huff_pt->run=0;
	huff_pt->symbol=0;
	huff_pt->length=0;

}

/*------------------------------------------*/
/* This routine uses huffman tree to 	    */
/* compress a symbol			    */
/*------------------------------------------*/

int compress_symbol
(HUFF *huff_stream,
 int symbol)
{

	int mask;
	LEAF *huff;

	if (symbol>huff_stream->max || symbol<huff_stream->min){
	   printf("ERROR: Symbol to be compressed is out of range\n");
	   printf("symbol[%d]",symbol);
	   if (symbol>huff_stream->max)
	      symbol=huff_stream->max;
	   else 
	       symbol=huff_stream->min;
	   printf(" ammended to %d\n",symbol);
	}
	/* find appropate symbol to compress*/
	huff=huff_stream->huff+symbol-huff_stream->min;
	
	/* output symbol to bitstream */
	mask=1;
	while (mask<(1<<huff->length)){
	      output_bit(huff_stream->dump,(int)(mask & (huff->symbol)));
	      mask<<=1;
	}

	/* save context symbol */
	huff_stream->context=huff->index;

	/* increament number of occurences for symbol */
	huff->nitems++;

	return (huff->length);

}
/*------------------------------------------*/
/* This routine uses huffman tree to        */
/* compress a runlength symbol              */
/*------------------------------------------*/
 
int compress_run
(HUFF *huff_stream,
 int symbol)
{
 
        int mask;
        LEAF *huff;
 
        if (symbol>huff_stream->run || symbol<1){
           printf("ERROR: Run lenght Symbol to be compressed is out of range\n");
	   printf("run length[%d]",symbol);
	   if (symbol>huff_stream->max)
	      symbol=huff_stream->max;
	   else 
	       symbol=huff_stream->min;
	   printf(" ammended to %d\n",symbol);
        }

        /* find appropate symbol to compress*/
        huff=huff_stream->huff+symbol+huff_stream->max-huff_stream->min;
         
	/* output symbol to bitstream */
        mask=1;
        while (mask<(1<<huff->length)){
              output_bit(huff_stream->dump,(int)(mask & (huff->symbol)));
              mask<<=1;
        }

	/* increament number of occurences for symbol */
        huff->nitems++;
 
        return (huff->length);
 
}

/*------------------------------------------*/
/* This routine decodeds the huffman stream */
/*------------------------------------------*/

int uncompress_symbol
(HUFF *huff_stream)
{
 	LEAF *huff_pt;

	huff_pt=huff_stream->huff_top;
 
	for (;;){ 
	
	    /* continue until symbol decoded */
	    if (huff_pt->zero_pt==NULL){
               huff_pt->nitems++;
               return huff_pt->index;
	    }

            if (input_bit(huff_stream->dump))
	       huff_pt=huff_pt->one_pt;
	    else
	        huff_pt=huff_pt->zero_pt;
	}
            
 
}

/*------------------------------------------*/
/* This routine decodeds possible 	    */
/*			 the huffman stream */
/*------------------------------------------*/
int uncompress_run_symbol
(HUFF *huff_stream,
 int *run)
{
        LEAF *huff_pt;
 
        huff_pt=huff_stream->huff_top;
	*run=0; /* no run */
 
        for (;;){
        
            /* continue until symbol decoded */
            if (huff_pt->zero_pt==NULL){
               huff_pt->nitems++;
	       *run=huff_pt->run;
               return huff_pt->index;
            }
 
            if (input_bit(huff_stream->dump))
               huff_pt=huff_pt->one_pt;
            else
                huff_pt=huff_pt->zero_pt;
        }
 
 
}


/*--------------------------------------------*/
/* Outputs a bits to the PACKED symbol stream */
/*--------------------------------------------*/

void output_bit
(PACKED *dump,
 int bit)
{
        /* output to file - writes if 1 leaves if 0 */
        if (bit)
           *(dump->symbols) |= dump->sym_mask;

	dump->bits++;
 
        /* check for overflow */
        if (dump->sym_mask==0x80){
	   /* checks memory exists to store next byte */ 
	   if (dump->bits>=dump->max_bits){
	      printf("dump size exceeded panic\n");
	      exit(-1);
	   }
           dump->symbols++;
 
           /* clear new byte of data */
           *dump->symbols=0;
 
           /* reset mask */
           dump->sym_mask=0x01;
        }
        /* otherwise increment mask */
        else
            dump->sym_mask<<=1;

 
}

/*-----------------------------------------*/
/* Inputs the next bits from the PACKED    */
/*			 symbol stream     */ 
/*-----------------------------------------*/

inline int input_bit
(PACKED *dump)
{
        int tmp;
 
        tmp=(dump->sym_mask) & *(dump->symbols);
 
        /* check for overflow */
        if (dump->sym_mask==0x80){
           dump->symbols++;
            
           /* reset mask */
           dump->sym_mask=0x01;
        }   
        /* otherwise increment mask */
        else
            dump->sym_mask<<=1;

	dump->bits++;
 
        /* check if there is a bit at the mask position or not */
        if (tmp)
           return 1;
        else
            return 0;
}

