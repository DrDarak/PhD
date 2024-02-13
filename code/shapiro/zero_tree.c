/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*               zero tree compression functions                */
/*                      reworked 2/7/96                         */
/*##############################################################*/

/*****************************************************************/
/* This module codes and decodes wavelet transformed images using */
/* an approach closely based upon the EZW method proposed by      */
/* Jerome Shapiro in the following paper:                          */
/* JM Shapiro "Embedded Image Coding Using Zerotrees of Wavelet    */
/* coefficients", IEEE Trans. Signal Process., 41(12), 1993, pp. 3445-62   */
/******************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "zero_tree.h"
#include "arith.h"


/* Internal functions */
void tree(IMAGE *,PACKED *,int ,int ,int ,int ,int );
int untree(IMAGE *,PACKED *,int ,int ,int ,int ,int ,int );
int zerotree_root(IMAGE *,int ,int ,int );
int getmax(IMAGE *);
void mark_tree(IMAGE *,int ,int );
void mark_untree(IMAGE *,int ,int );


/* defines */
#define BIT1                 (0x80000000) /* Most signif bit, sign bit */
#define BIT2                 (0x40000000) /* second most signif bit */
#define BIT3                 (0x20000000) /* third most signif bit */
					  /* Bits 2 and 3 are together used to */
               				  /* mark a coefft as significant or  */
					  /* insignificant */
#define BITS23               (0x60000000) /* Bits 2 and 3 are both set  */
#define INITIALIZE_THRESHOLD (0x08000000) /* Initial significance threshold */

/* The following three are used during decode: */
#define POS_SIG              (0x40000000) /* Coefft of zero magnitude and */
					  /* marked as significant.       */
#define NEG_SIG              (0xdfffffff) /* Coefft of magnitude -1 and */
					  /* marked as significant.     */
#define ZERO                 (0x20000000) /* Coefft of zero magnitude and */
					  /* marked as insignificant      */

/* Symbols used to encode zerotrees: */
#define ROOT 0 /* Root of a zerotree */
#define ISIG 1 /* Isolated insignificant coefft, i.e. not zerotree root. */
#define PSIG 2 /* Significant positive coefft */
#define NSIG 3 /* Significant negative coefft */

#define ROOT2 1.414213562
#define TRUE  1
#define FALSE 0

/***************************************************************/
/* zero_tree_code_image():                                 */
/* code wavelet transformed image by EZW method.           */ 
/* Called from code.c                                     */
/* Parameters:                                           */
/*	IMAGE *image  pointer to a data structure holding the wavelet  */ 
/*                    coeffts of the image                            */
/*	PACKED *dump  pointer to data structure holding information   */
/*		      about the entropy coding                       */
/*	int scales    number of scales of wavelet transform used      */
/*****************************************************************/

void zero_tree_code_image
(IMAGE *image,
 PACKED *dump,
 int scales)
{
        int i,max_c,t_hold;
        int x_size,y_size;
 
        max_c=getmax(image); /* Get maximum wavelet coefficient */
        
        /* find approprated threshold to start at */
        t_hold=INITIALIZE_THRESHOLD;
        while (!(max_c >= t_hold || max_c < -t_hold)) /* Negated significance */
              t_hold >>= 1;
        dump->t_hold=t_hold;
        
        /* scan subbands in order specified in Fig 5 of [Shapiro] and refine
           threshold until target compression reached */
        while (t_hold>0 && dump->bits<dump->n_bits){
              /*printf ("Zerotree: threshold - %6d\n", t_hold);*/
             
              x_size=(image->columns)>>scales;
              y_size=(image->rows)>>scales;
 
              /* search top LL rectangle once */
              tree(image,dump,t_hold,x_size,y_size,0,0);
 
              /*Search all other scales from lowest to high */
              for (i=0;i<scales;i++,x_size*=2,y_size*=2){
 
                  /* HiLow (Top Right at scale)  */
                 tree (image,dump,t_hold,x_size,y_size,x_size,0);
 
                  /* HiHi  (Lower Right at scale) */
                 tree (image,dump,t_hold,x_size,y_size,x_size,y_size);
         
                 /* LowHi (Lower Left at scale)  */
                 tree (image,dump,t_hold,x_size,y_size,0,y_size);
 
              }
 
              /* move down to next zero tree threshold */
              t_hold>>=1;
        }
 
        /* send end of file symbol */
        Compress_symbol(dump,END_OF_STREAM);
 
}

/***************************************************************/
/* zero_tree_decode_image():  */
/* code EZW compressed image to yield wavelet transformed image. */
/* Called from decode.c */
/* Parameters:  */
/*	IMAGE *image  pointer to a data structure holding the wavelet  */
/*                    coeffts of the image  */
/*	PACKED *dump  pointer to data structure holding information  */
/*		      about the entropy coding  */
/*	int scales    number of scales of wavelet transform used  */
/*****************************************************************/

void zero_tree_decode_image
(IMAGE *image,
 PACKED *dump,
 int scales)
{
        int i,t_hold;
        int x_size,y_size,symbol;

        /* Load inital threshold */
        t_hold=dump->t_hold;

        /* load 1st symbol */
        symbol=0;

        /* scan subbands in order specified in Fig 5 of [SDhapiro] */
        while (t_hold>0 && dump->bits<dump->n_bits && symbol!=END_OF_STREAM){
        /*            printf ("Zerotree: threshold - %6d\n", t_hold);
        */
                                        
              x_size=(image->columns)>>scales;
              y_size=(image->rows)>>scales;

              /* search top LL rectangle once */
              untree(image,dump,t_hold,x_size,y_size,0,0,symbol);

              /*Search all other scales from lowest to high */    
              for (i=0;i<scales;i++,x_size*=2,y_size*=2){

                  /* HiLow (Top Right at scale)  */       
                 untree(image,dump,t_hold,x_size,y_size,x_size,0,symbol);

                  /* HiHi  (Lower Right at scale) */                      
                 untree(image,dump,t_hold,x_size,y_size,x_size,y_size,symbol);

                 /* LowHi (Lower Left at scale)  */                            
                 untree(image,dump,t_hold,x_size,y_size,0,y_size,symbol);

              }                                                           

              /* move down to next zero tree threshold */
              t_hold>>=1;
        }

}         
 
/****************************************************************
 tree():
 scan a specified subband generating zerotree data and encoding
 the resultant symbols.
 Follows the flow diagram of Fig 6 of [Shapiro]
 
 BIT1,BIT2,BIT3 have the following significance:
	BIT1: MSBit of coefficient, indicates whether coefft is
	      positive or negative.
	BIT2 and BIT3:
		= 01: marks the coefficient as zero (insignificant) at
		      current scale.
		= 10: marks the coefficient as significant at current
		      scale.
		= 00: Coefft not marked. Coefft is positive.
		= 11: Coefft not marked. Coefft is negative.

 the entropy coding makes use of four symbols to encode zerotrees,
 as described in [Shapiro]:
	NSIG: significant coefficient, negative.
	PSIG: significant coefficient, positive.
	ISIG: insignificant coefft which has significant descendents
            (ie insignificant but not predictacbly insignificant)
	ROOT: Coefft is the root of a zerotree of insig coeffts

 Called from zero_tree_code_image above.

 Parameters:
	IMAGE *image:  contains the wavelet coeffts 
 	PACKED *dump:  comtains entropy coding info.
	int t_hold:    current threshold
	int x_size,y_size: dimensions of subband to be processed.
	int x_start,Y_start: location of subband to be processed.
****************************************************************/

void tree
(IMAGE *image,
 PACKED *dump,
 int t_hold,
 int x_size,
 int y_size,
 int x_start,
 int y_start)
{
 
        int x,y,x_max,y_max,temp;
 
        /*printf("Scanning Scale Beginning at (%3d,%3d) Threashold: %7d\n",
             x_start, y_start, t_hold); */
 
        x_max=x_start+x_size;
        y_max=y_start+y_size;
 
        for (y=y_start;y<y_max;y++)
            for (x=x_start;x<x_max;x++) {
 
                /* Stop at required file size */
                if (dump->bits>=dump->n_bits)
                   break;
 
                temp=image->pt[image->columns*y+x];
 
                if (!(temp & BIT2) && (temp & BIT3)){
		   /* Coefft has been marked insignificant. */ 
                   /* i.e. coefft is predictably insignificant (Fig 6) */
                   /* Remove marker, accounting for the sign of the coefft. */
                   /* (in preparation for next threshold) */
                   if (temp & BIT1)
                      image->pt[image->columns*y+x] |=  BITS23;
                   else
                       image->pt[image->columns*y+x] &= ~BITS23;
 
                   /* skip to next point in the image */
                   continue;
                }
 
                /* check if the ceofficient MARKED as significant */
                if ((temp & BIT2) && !(temp & BIT3)){
		   /* Coefft has been marked significant. */ 
                   /* Refine coefficient -> send 0/1 to output buffer */
                   if (temp & t_hold)
                      *(dump->refine) |= dump->ref_mask;
 
                   /* increment number of bits used */
                   dump->bits++;
 
                   /* check for overflow */
                   if (dump->ref_mask==0x80){
                      dump->refine++;
 
                      /* clear new byte of data */
                      *dump->refine=0;
 
                      /* reset mask */
                      dump->ref_mask=0x01;
                   }
                   /* otherwise increment mask */
                   else
                       dump->ref_mask<<=1;
 
                   /* skip to next point in the image */
                   continue;
                }

                /* Check if bit is significant  IE New significance*/
                if (temp >= t_hold || temp < -t_hold){
 
                   /* Mark it as significant */
                   image->pt[image->columns*y+x] |=  BIT2;
                   image->pt[image->columns*y+x] &= ~BIT3;
 
                   /* send NSIG or PSIG symbol */
                   Compress_symbol(dump,((temp >= 0)? PSIG : NSIG));
 
                   /* skip to next point in the image */
                   continue;
                }
 
                /* Coefft is insignificant.  */
                /* If coefft is a zerotree root, mark its descendants
                   insignificant and send ROOT symbol */ 
                if ((x_start == 0 && y_start == 0)?
                    (zerotree_root(image, t_hold, x+x_size, y       ) &&
                     zerotree_root(image, t_hold, x,        y+y_size) &&
                     zerotree_root(image, t_hold, x+x_size, y+y_size))
                     :
                    (zerotree_root(image, t_hold, x, y))){
                   if (x_start == 0 && y_start == 0){
                      mark_tree(image, x+x_size, y       );
                      mark_tree(image, x,        y+y_size);
                      mark_tree(image, x+x_size, y+y_size);
                    }
                  else {
                       mark_tree(image, 2*x,   2*y  );
                       mark_tree(image, 2*x+1, 2*y  );
                       mark_tree(image, 2*x,   2*y+1);
                       mark_tree(image, 2*x+1, 2*y+1);
                  }
                  /* send zero tree ROOT symbol to the buffer */
                  Compress_symbol(dump,ROOT);
                }
                else {
                     /* insignificant coefft with significant descendants */
                     /* Send ISIG  symbol */
                     Compress_symbol(dump,ISIG);
                }
            }
               
}

/****************************************************************
 untree():
 receive entropy coded symbols, generating a subband of a wavelet 
 transformed recovered image.

 Follows Fig 6 of [Shapiro].
 
 BIT1,BIT2,BIT3 have the following significance:
	BIT1: MSBit of coefficient, indicates whether coefft is
	      positive or negative.	BIT2 and BIT3:
		= 01: marks the coefficient as zero (insignificant) at
		      current scale.
		= 10: marks the coefficient as significant at current
		      scale.
		= 00: Coefft not marked. Coefft is positive.
		= 11: Coefft not marked. Coefft is negative.

 the entropy coding makes use of four symbols to encode zerotrees,
 as described in [Shapiro]:
	NSIG: significant coefficient, negative.
	PSIG: significant coefficient, positive.
	ISIG: insignificant coefft which has significant descendents
            (ie insignificant but not predictacbly insignificant)
	ROOT: Coefft is the root of a zerotree of insig coeffts

 Called from zero_tree_decode_image above.

 Parameters:
	IMAGE *image:  contains the wavelet coeffts 
 	PACKED *dump:  comtains entropy coding info.
	int t_hold:    current threshold
	int x_size,y_size: dimensions of subband to be processed.
	int x_start,Y_start: location of subband to be processed.
      int symbol:    first symbol to be decoded. Thereafter used
                     to hold current symbol.

 Returns:  the last symbol decoded.
****************************************************************/

int untree
(IMAGE *image,
 PACKED *dump,
 int t_hold,
 int x_size,
 int y_size,
 int x_start,
 int y_start,
 int symbol)
{
        int x,y,x_max,y_max,temp;
 
        /*
        printf("Scanning Scale Beginning at (%3d,%3d) Threashold: %7d\n",
            x_start, y_start, t_hold);
        */
 
        x_max=x_start+x_size;
        y_max=y_start+y_size;
 
        for (y=y_start;y<y_max;y++)
            for (x=x_start;x<x_max;x++){
                /* Stop at required file size */
                if (dump->bits>=dump->n_bits || symbol==END_OF_STREAM)
                   break;
 
                temp=image->pt[image->columns*y+x];
 
                /* check for zero marker */
                if (!(temp & BIT2) && (temp & BIT3)){
		   /* Coefft has been marked insignificant. */ 
                   /* i.e. coefft is predictably insignificant (Fig 6) */
                   /* Remove marker, accounting for the sign of the coefft. */
                   /* (in preparation for next threshold) */
                   if (temp & BIT1) /* Negative coefft */
                      image->pt[image->columns*y+x] |=  BITS23; /* Remove marker*/
                   else /* Positive coefft */
                      image->pt[image->columns*y+x] &= ~BITS23; /* Remove marker */
 
                   /* skip to next data point */
 
                   continue;
                }
 
                /* check if coefficent is marked significant */
                if ((temp & BIT2) && !(temp & BIT3)) {
                   /* Coefft is significant, so */ 
                   /* refine coefficent data */
                   if (dump->ref_mask & *(dump->refine))
                      image->pt[image->columns*y+x] |=  t_hold;
                   else
                      image->pt[image->columns*y+x] &= ~t_hold;
 
                   /* increament number of bits for refine */
                   dump->bits++;
 
                   /* check for overflow */
                   if (dump->ref_mask==0x80){
                      dump->refine++;
                      /* reset mask */
                      dump->ref_mask=0x01;
                   }
                   /* otherwise increment mask */
                   else
                       dump->ref_mask<<=1;
 
                   /* guess the rest */
                   if (temp & BIT1)
                      image->pt[image->columns*y+x] &= ~(t_hold >> 1);
                   else
                      image->pt[image->columns*y+x] |=  (t_hold >> 1);
           
                   /* skip to next point in the image */
                   continue;
                }

 		switch (symbol=Expand_symbol(dump)){
                       /* Root of a tree of insignificant coeffts: set them
 			  all to zero */
                       case ROOT: if (x_start==0 && y_start==0){
                                     mark_untree(image,x+x_size,y);
                                     mark_untree(image,x,y+y_size);
                                     mark_untree(image,x+x_size,y+y_size);
                                  }
                                  else {
                                       mark_untree(image,2*x,2*y);
                                       mark_untree(image,2*x+1,2*y);
                                       mark_untree(image,2*x,2*y+1);
                                       mark_untree(image,2*x+1,2*y+1);
                                  }
                                  break;
 
		       /* Predictably insignificant: */
                       case ISIG: break;
 
		       /* Newly significant (positive) */
                       case PSIG: image->pt[image->columns*y+x] = POS_SIG;
                                  image->pt[image->columns*y+x] |= t_hold;
                                  image->pt[image->columns*y+x] |= (t_hold >> 1);      /* Guess */
                                  break;
 
		       /* Newly significant (negative) */
                       case NSIG: image->pt[image->columns*y+x] = NEG_SIG;
                                  image->pt[image->columns*y+x] &= ~t_hold;
                                  image->pt[image->columns*y+x] &= ~(t_hold >> 1);      /* Guess */
                                  break;
 
                       case END_OF_STREAM: break;
 
                       default:   printf("Tree Symbol Not Recognised \n");
                }
 
        }
 
        /* return last symbol for end of stream */
        return symbol;
 
}

/**************************************************************************
 int zerotree_root(image, t_hold, x,y):
 Returns TRUE if the coefficient located at the point (x,y) is the
 root of a zerotree of insignificant coeffts, at threshold t_hold.
 Otherwise returns FALSE.
**************************************************************************/

int zerotree_root
(IMAGE *image, 
 int t_hold,
 int x,
 int y)
{

	int i,j,x2,y2,x3,y3,x4,y4;
        int *pt;
         
        x2=x*2; y2=y*2;
        x3=x*4; y3=y*4;
        x4=x*8; y4=y*8;
 
        /* Consider first level */
        /* return a TRUE if the point has no children */
        if (x>=image->columns || y>=image->rows)
           return (TRUE);
                            
	/* If point is significant or has been marked as significant then it
           is not a zerotree root*/
        pt=image->pt+image->columns*y+x;
        if ((*pt >= t_hold || *pt < -t_hold) && (!(*pt & BIT2) || (*pt & BIT3)) )
           return (FALSE);
            
        /* Similarly for 2nd level */
        if (x2>=image->columns || y2>=image->rows)
           return (TRUE);
        pt=image->pt+image->columns*y2+x2;
 
        for (j=0;j<2;j++){                 
            if (y2+j>=image->rows)
               continue;
            for (i=0;i<2;i++){
                if (x2+i<image->columns)
		   if ((*pt >= t_hold || *pt < -t_hold) && 
		       (!(*pt & BIT2) || (*pt & BIT3)))
           	      return (FALSE); 
                pt++;
            }
            pt+=image->columns-2;
        }    

        /* Similarly for 3rd level */
        if (x3>=image->columns || y3>=image->rows)
           return (TRUE);
        pt=image->pt+image->columns*y3+x3;

        for (j=0;j<4;j++){                 
            if (y3+j>=image->rows)
               continue;
            for (i=0;i<4;i++){
                if (x3+i<image->columns)
		   if ((*pt >= t_hold || *pt < -t_hold) && 
                       (!(*pt & BIT2) || (*pt & BIT3)))
                      return (FALSE); 
                pt++;
            }
            pt+=image->columns-4;
        }    

	/* Similarly for 4th level */
        if (x4>=image->columns || y4>=image->rows)
           return (TRUE);
        pt=image->pt+image->columns*y4+x4;

        for (j=0;j<8;j++){                 
            if (y4+j>=image->rows)
               continue;
            for (i=0;i<8;i++){
                if (x4+i<image->columns)
		   if ((*pt >= t_hold || *pt < -t_hold) && 
                       (!(*pt & BIT2) || (*pt & BIT3)))
                      return (FALSE); 
                pt++;
            }
            pt+=image->columns-8;
        }    

        /* If we reach this point, we have a zerotree root */
        return (TRUE);



        /*  Old code: recursively check all children for possible roots 
        if (zerotree_root(image,t_hold,2*x,2*y)==FALSE)
           return (FALSE);

        if (zerotree_root(image,t_hold,2*x+1,2*y)==FALSE)
           return (FALSE);

        if (zerotree_root(image,t_hold,2*x+1,2*y+1)==FALSE)
           return (FALSE);

        if (zerotree_root(image,t_hold,2*x,2*y+1)==FALSE)
           return (FALSE);
	*/

}

/*******************************************************************
 Returns the maximum coefft in the image
*******************************************************************/
 
int getmax
(IMAGE *image)
{
        int i,max;
        
        max=0;     
        for (i=0;i<image->columns*image->rows;i++)
            if (max<image->pt[i])
               max=image->pt[i];
        
        return max;              
 
}

/*******************************************************************
 mark_tree():
 Mark all coefficients in the tree descending from (x,y) as 
 insignificant, except those coeffts that are already marked significant.
**********************************************************************/

void mark_tree
(IMAGE *image,
 int x,
 int y)
{

	int i,j,x2,y2,x3,y3,x4,y4;
        int *pt;
         
        x2=x*2; y2=y*2;
        x3=x*4; y3=y*4;
        x4=x*8; y4=y*8;
 
        /* First level */
        if (x>=image->columns || y>=image->rows)
           return;

	/* If not already marked significant */
        if (!(image->pt[image->columns*y+x] & BIT2) || (image->pt[image->columns*y+x] & BIT3)) {
           /* Mark as insignificant */
           image->pt[image->columns*y+x] &= ~BIT2;
           image->pt[image->columns*y+x] |=  BIT3;
        }
            
        /* Similarly for 2nd level */
        if (x2>=image->columns || y2>=image->rows)
           return;
        pt=image->pt+image->columns*y2+x2;
 
        for (j=0;j<2;j++){
            if (y2+j>=image->rows)
               continue;
            for (i=0;i<2;i++){
                if (x2+i<image->columns)
                   if (!(*pt & BIT2) || (*pt & BIT3)) {
	              /* mark ar zero */
                      *pt &= ~BIT2;
                      *pt |=  BIT3;
                   }
                pt++;
            }
            pt+=image->columns-2;
        }
            
        /* Similarly for 3rd level */
        if (x3>=image->columns || y3>=image->rows)
           return;
        pt=image->pt+image->columns*y3+x3;
 
        for (j=0;j<4;j++){
            if (y3+j>=image->rows)
               continue;
            for (i=0;i<4;i++){
                if (x3+i<image->columns)
		   if (!(*pt & BIT2) || (*pt & BIT3)) { 
                      /* mark ar zero */
                      *pt &= ~BIT2; 
                      *pt |=  BIT3; 
                   } 
                pt++;
            }
            pt+=image->columns-4;
        }   
 
        /* Similarly for 4th level */
        if (x4>=image->columns || y4>=image->rows)
           return;
        pt=image->pt+image->columns*y4+x4;
 
        for (j=0;j<8;j++){
            if (y4+j>=image->rows)
               continue;
            for (i=0;i<8;i++){
                if (x4+i<image->columns)
		   if (!(*pt & BIT2) || (*pt & BIT3)) { 
                      /* mark ar zero */
                      *pt &= ~BIT2; 
                      *pt |=  BIT3; 
                   } 

                pt++;
            }
            pt+=image->columns-8;
        }    

 

	/* Old code: recuresive method 
        mark_tree(image,2*x,2*y);
        mark_tree(image,2*x+1,2*y);
        mark_tree(image,2*x,2*y+1);
        mark_tree(image,2*x+1,2*y+1); */
 
}

/*********************************************************************
 Set all wavelet coeffts in the tree descending from (x,y) to
 zero.  Used during decoding.
********************************************************************/

void mark_untree
(IMAGE *image,
 int x,
 int y)
{
	int i,j,x2,y2,x3,y3,x4,y4;
	int *pt;
	
	x2=x*2; y2=y*2;
	x3=x*4; y3=y*4;
	x4=x*8; y4=y*8;

	/* zero first level */
        if (x>=image->columns || y>=image->rows)
           return;
            
        if (image->pt[image->columns*y+x] == 0)
           image->pt[image->columns*y+x]=ZERO;

	/* zero 2nd level */
	if (x2>=image->columns || y2>=image->rows) 
           return;
	pt=image->pt+image->columns*y2+x2;

	for (j=0;j<2;j++){
	    if (y2+j>=image->rows)
	       continue;
	    for (i=0;i<2;i++){
		if (x2+i<image->columns)
                   if (*pt == 0) *pt=ZERO;
	        pt++;
	    }
	    pt+=image->columns-2;
	}
	    
	/* zero 3rd level */
	if (x3>=image->columns || y3>=image->rows)  
           return; 
        pt=image->pt+image->columns*y3+x3;

	for (j=0;j<4;j++){
            if (y3+j>=image->rows) 
               continue; 
            for (i=0;i<4;i++){
                if (x3+i<image->columns) 
                   if (*pt == 0) *pt=ZERO; 
                pt++;    
            }    
            pt+=image->columns-4; 
        } 

	/* zero 4th level */
	if (x4>=image->columns || y4>=image->rows)   
           return; 
        pt=image->pt+image->columns*y4+x4;

	for (j=0;j<8;j++){
            if (y4+j>=image->rows) 
               continue; 
            for (i=0;i<8;i++){
                if (x4+i<image->columns) 
                   if (*pt == 0) *pt=ZERO; 
                pt++;    
            }    
            pt+=image->columns-8; 
        } 

	/* Old code: recursive method  
        mark_untree(image,2*x,2*y);
        mark_untree(image,2*x+1,2*y);
        mark_untree(image,2*x,2*y+1);
        mark_untree(image,2*x+1,2*y+1); */
 
}
 
/**************************************************
 Remove all markers from coeffts in image.
**************************************************/

void clear_markers
(IMAGE *image)
{
        int i;
 
        for (i=0;i<image->columns*image->rows;i++)
            if (image->pt[i] & BIT1)
               image->pt[i] |= BITS23;
            else
                image->pt[i] &= ~BITS23;
}
 


