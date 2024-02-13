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
#include "mem_hand.h"
#include "huff.h"

/* Internal functions */
int input_bit (PACKED *);
void output_bit (PACKED *, int);
void traverse_tree (LEAF *, int , int );
void setup_leaf (LEAF *, int , int );
LEAF * allocate_leaf_mem (int , int );
LEAF * delete_element (LEAF *, LEAF *);
LEAF * search_list (LEAF *);
int uncompress_stream (PACKED *, LEAF *);

/* defines */
#define MAX_RUN 16

/* macros */
#define myabs(x)    ( x < 0 ? -x : x)

/*------------------------------------------*/
/* sets up huffman structures and tree to   */
/* allow efficien huffman coding 	    */
/*------------------------------------------*/

void setup_huffman_tree
(PACKED *dump,
 FILE *fp)
{
        int i,max,min,n;
 
	/* load max/min from file */
        fscanf(fp,"%d\t%d\n",&min,&max);

	/* store max / min in packed arry */
        dump->min=min;
        dump->max=max;
 
        /* set up huff symbol stream */
        dump->huff=malloc_LEAF((max-min+1),"setup huff tree");
 
        /* read huffman tree link list from file */
        for (i=min;i<max;i++){
            fscanf(fp,"%d\n",&n);
            setup_leaf(&(dump->huff[i-min]),n,i);
            dump->huff[i-min].orig=&(dump->huff[i-min+1]);
            dump->huff[i-min].next=dump->huff[i-min].orig;
        }     
        fscanf(fp,"%d\n",&n);
        setup_leaf(&(dump->huff[max-min]),n,max);

        /* initialise huffman tables */                 
        dump->huff_top=construct_huff_tree(dump->huff);
 
}

/*------------------------------------------*/
/* Writes a flat pdf to the standard pdf    */
/* file on disk. Use for reformatting the   */
/* pdf tables				    */
/*------------------------------------------*/
 
void wipe_huffman_pdf
(PACKED *dump,
 FILE *fp)
{
 
      int i;
 
        fprintf(fp,"%d\t%d\n",dump->min,dump->max);
        for (i=dump->min;i<=dump->max;i++)
            fprintf(fp,"%d\n",1);
 
}

/*------------------------------------------*/
/* Writes an existing pdf to standard pdf   */   
/* file on disk. Use for training the       */   
/* pdf tables                               */   
/*------------------------------------------*/

void save_huffman_pdf
(PACKED *dump,
 FILE *fp)
{

        int i,max,half_int,scale;
	
	/* find max entery in the list */
	max=0;
	for (i=dump->min;i<=dump->max;i++){
	    if (dump->huff[i-dump->min].nitems>max)
	       max=dump->huff[i-dump->min].nitems;
	    if (dump->huff[i-dump->min].nitems<1){
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
	   for (i=dump->min;i<=dump->max;i++) 
	       if (dump->huff[i-dump->min].nitems/scale>1)
	           dump->huff[i-dump->min].nitems/=scale;
	}

	/* write out pdftable to file */
        fprintf(fp,"%d\t%d\n",dump->min,dump->max);
        for (i=dump->min;i<=dump->max;i++)
            fprintf(fp,"%d\n",dump->huff[i-dump->min].nitems);

}                   

/*------------------------------------------*/
/* used to close down huffman coder         */
/* encoding and decoding                    */
/*------------------------------------------*/
 
void free_huffman_tree
(PACKED *dump)
{

	free_tree(dump->huff_top); /* frees huffman tree */
	free_LEAF(dump->huff); /* frees linear list of symbols */

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
(PACKED *dump)
{
	int i,nitems,bits;
	float entropy;

	nitems=0;
	bits=0;
	for (i=dump->min;i<=dump->max;i++){
            nitems+=dump->huff[i-dump->min].nitems;
	    bits+=dump->huff[i-dump->min].nitems*dump->huff[i-dump->min].length;
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
	huff_pt->symbol=0;
	huff_pt->length=0;

}

/*------------------------------------------*/
/* This routine uses huffman tree to 	    */
/* compress a symbol			    */
/*------------------------------------------*/

int compress_symbol
(PACKED *dump,
 int symbol)
{

	int mask;
	LEAF *huff;

	if (symbol>dump->max || symbol<dump->min){
	   printf("ERROR: Symbol to be compressed is out of range\n");
	   exit(-1);
	}
	/* find appropate symbol to compress*/
	huff=dump->huff+symbol-dump->min;
	
	mask=1;
	while (mask<(1<<huff->length)){
	      output_bit(dump,(int)(mask & (huff->symbol)));
	      mask<<=1;
	}

	huff->nitems++;

	return (huff->length);

}

/*------------------------------------------*/
/* This routine runes the recursive call    */ 
/* which decodeds the huffman stream 	    */
/*------------------------------------------*/

int uncompress_symbol
(PACKED *dump)
{
        int index;
 	LEAF *huff_pt;

	huff_pt=dump->huff_top;
 
	/* recursively use this routinue until bottom of tree reached */
        if (huff_pt->zero_pt==NULL){
           huff_pt->nitems++;
           return huff_pt->index;
        }
 
	/* input another bits from the stream and traverse huff tree */
        if (input_bit(dump)){
           index=uncompress_stream(dump,huff_pt->one_pt);
	}
        else {
           index=uncompress_stream(dump,huff_pt->zero_pt);
	}
            
        return index;
 
}

/*------------------------------------------*/
/* This routine moves down from the top of  */
/* huffman tree until  the appropate symbol */
/* has been decompressed                    */
/*------------------------------------------*/
 
int uncompress_stream
(PACKED *dump,
 LEAF *huff_pt)
{
        int index;

        /* recursively use this routinue until bottom of tree reached */
        if (huff_pt->zero_pt==NULL){
           huff_pt->nitems++;
           return huff_pt->index;
        }

        /* input another bits from the stream and traverse huff tree */
        if (input_bit(dump)){
           index=uncompress_stream(dump,huff_pt->one_pt);
        }
        else {
           index=uncompress_stream(dump,huff_pt->zero_pt);
        }     
        
        return index;
 
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

int input_bit
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

