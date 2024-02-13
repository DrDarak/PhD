/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*    Arithmetric coder Hacked and slasher by Daveb 31/1/96     */
/*         Original code form 'THE DATA COMPRESSION BOOK'       */
/*                      reworked 3/7/96                         */
/*##############################################################*/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "arith.h"
#include "mem_hand.h"


/* Internal functions */
void initialize_model (SYMBOL *s);
void convert_int_to_symbol(int ,SYMBOL *);
void convert_int_to_symbol(int ,SYMBOL *);
void update_model (int ,SYMBOL *);
void encode_symbol(PACKED *);
void get_symbol_scale(SYMBOL *);
int convert_symbol_to_int(int ,SYMBOL *);
short get_current_count(SYMBOL *);
void remove_symbol_from_stream(PACKED *);

/* defines */
#define MAXIMUM_SCALE   16383  /* Maximum allowed frequency count  */


/*
 * This routine must be called to initialize the encoding process.
 * The high register is initialized to all 1s, and it is assumed that
 * it has an infinite string of 1s to be shifted into the lower bit
 * positions when needed.
 */

void initialize_arithmetic_encoder
(SYMBOL *s)
{
    	s->low=0;
    	s->high=0xffff;
    	s->underflow_bits=0;

	/* inital context */
	s->context=0;

    	initialize_model(s);
}

/*
 * The next five routines implement the modeling functions for the order-1
 * adpative model.  The first routine initializes the model, which consists
 * of allocating 256 context tables.  Note that even though there are
 * 257 symbols in the alphabet, after taking into account the END_OF_STREAM
 * symbol, we only need 256 tables.  END_OF_STREAM doesn't get a table of
 * its own, since it will never be used as a context.
 */

void initialize_model
(SYMBOL *s)
{
    	int context;
    	int i;

    	for (context=0;context<END_OF_STREAM;context++){

            s->totals[context]=malloc_int(END_OF_STREAM+2,"initialize model");

            for (i=0;i<=(END_OF_STREAM+1);i++)
                s->totals[context][i]=i;
    	}
}

/*
 * Convert an integer to a symbol works just like the old arithmetic
 * coding routine in ARITH.C.  The only difference is that we have to
 * pass a context number so the routine knows which context table to
 * get the counts and scale from.
 */
void convert_int_to_symbol
(int c,
 SYMBOL *s)
{
    	s->scale=s->totals[s->context][END_OF_STREAM+1];
    	s->low_count=s->totals[s->context][c];
    	s->high_count=s->totals[s->context][c+1];
}

/*
 * This routine is called to increment the counts for the current
 * context.  It is called after a character has been encoded or
 * decoded.  It has to increment all the counts, then check to see
 * if we have hit the maximum allowable count.  If we have, the
 * counts are rescaled, making sure that none of them have fallen to
 * zero.
 */

void update_model
(int symbol,
 SYMBOL *s)
{
    int i;

    for (i=symbol+1;i<=(END_OF_STREAM+1);i++)
        s->totals[s->context][i]++;

    if (s->totals[s->context][END_OF_STREAM+1]<MAXIMUM_SCALE)
       return;
    for (i=1;i<=(END_OF_STREAM+1);i++) {
        s->totals[s->context][i]/=2;
        if (s->totals[s->context][i]<=s->totals[s->context][i-1])
           s->totals[s->context][i]=s->totals[s->context][i-1]+1;
    }
}

/*
 * This routine is called to encode a symbol.  The symbol is passed
 * in the SYMBOL structure as a low count, a high count, and a range,
 * instead of the more conventional probability ranges.  The encoding
 * process takes two steps.  First, the values of high and low are
 * updated to take into account the range restriction created by the
 * new symbol.  Then, as many bits as possible are shifted out to
 * the output stream.  Finally, high and low are stable again and
 * the routine returns.
 */

void encode_symbol
(PACKED *dump)
{
    	long range;
	SYMBOL *s;

	/* load short cut */
	s=dump->s;

  	/* These three lines rescale high and low for the new symbol.*/
    	range = (long) ( s->high-s->low ) + 1;
    	s->high=s->low + (unsigned short)
                ((range * s->high_count)/s->scale-1);
    	s->low=s->low + (unsigned short)
               ((range * s->low_count)/s->scale);

 	/* This loop turns out new bits until high and low are far enough*/
 	/* apart to have stabilized.*/
 
    	for ( ; ; ){

 	    /* If this test passes, it means that the MSDigits match, and can*/
 	    /* be sent to the output stream.*/
 
       	    if ((s->high & 0x8000)==(s->low & 0x8000)) {
               OutputBit(dump,s->high & 0x8000);
               while (s->underflow_bits>0){
                     OutputBit(dump,~(s->high) & 0x8000 );
                     s->underflow_bits--;
               }
            }
 	    /* If this test passes, the numbers are in danger of underflow, because*/
 	    /* the MSDigits don't match, and the 2nd digits are just one apart.    */
            else if ((s->low & 0x4000) && !(s->high & 0x4000)){
                    s->underflow_bits+=1;
                    s->low &= 0x3fff;
                    s->high |= 0x4000;
            } 
	    else
                return;

            s->low <<= 1;
            s->high <<= 1;
            s->high |= 1;
    	}
}

void OutputBit
(PACKED *dump,
unsigned short bit)
{
	/* output to file - writes if 1 leaves if 0 */
	if (bit)
	   *(dump->symbols) |= dump->sym_mask; 

	/* check for overflow */
	if (dump->sym_mask==0x80){
	   dump->symbols++;

	   /* clear new byte of data */
	   *dump->symbols=0;

	   /* reset mask */
	   dump->sym_mask=0x01;
	}
	/* otherwise increment mask */
	else 
	    dump->sym_mask<<=1;

	/* increament bits used */
	dump->bits++;

}

/*
 * At the end of the encoding process, there are still significant
 * bits left in the high and low registers.  We output two bits,
 * plus as many underflow bits as are necessary.
 */

void flush_arithmetic_encoder
(PACKED *dump)
{
    	OutputBit(dump,(dump->s)->low & 0x4000);
    	(dump->s)->underflow_bits++;
    	while ((dump->s)->underflow_bits-- >0)
              OutputBit(dump,~((dump->s)->low) & 0x4000);
	/*
    	OutputBits( stream, 0L, 16 );
	*/
}


void Compress_symbol
(PACKED *dump,
 int symbol)
{

        convert_int_to_symbol(symbol,dump->s);

        encode_symbol(dump);

        update_model(symbol,dump->s);
	
	/* update context */
        (dump->s)->context=symbol;
    

        if (symbol==END_OF_STREAM)	
    	   flush_arithmetic_encoder(dump);
}

/*
 * This is the expansion routine.  It reads in bytes from the compressed
 * file and writes them out to the text file.
 */

int Expand_symbol
(PACKED *dump)
{
    	int count;
    	int c;

        get_symbol_scale(dump->s);

        count=get_current_count(dump->s);

        c=convert_symbol_to_int(count,dump->s);

        remove_symbol_from_stream(dump);

	update_model(c,dump->s);

	(dump->s)->context=c;

	/* return symbol */
	return c;
}


/*
 * Getting the current scale for the symbol just means pulling the number
 * out of the chosen context table.
 */

void get_symbol_scale
(SYMBOL *s)
{
    	s->scale=s->totals[s->context][END_OF_STREAM+1];
}

/*
 * Converting a symbol to an int means looking through the current
 * context for a symbol that spans the range that the SYMBOL parameter
 * has.
 */

int convert_symbol_to_int
(int count,
 SYMBOL *s)
{
    int c;

    for (c=0;count>=s->totals[s->context][c+1];c++)
        ;
    s->high_count=s->totals[s->context][c+1];
    s->low_count=s->totals[s->context][c];
    return( c );
}

/*
 * When decoding, this routine is called to figure out which symbol
 * is presently waiting to be decoded.  This routine expects to get
 * the current model scale in the s->scale parameter, and it returns
 * a count that corresponds to the present floating point code:
 *
 *  code = count / s->scale
 */

short get_current_count
(SYMBOL *s)
{
    long range;
    short count;

    range=(long)(s->high-s->low)+1;
    count=(short)((((long)(s->code-s->low)+1)*s->scale-1)/range);

    return(count);
}

/*
 * This routine is called to initialize the state of the arithmetic
 * decoder.  This involves initializing the high and low registers
 * to their conventional starting values, plus reading the first
 * 16 bits from the input stream into the code value.
 */

void initialize_arithmetic_decoder
(PACKED *dump)
{
    	int i;

	/* grab first bit */
    	(dump->s)->code = 0;
    	for (i=0;i<16;i++) {
            (dump->s)->code <<= 1;
            (dump->s)->code += InputBit(dump);
    	}

	/* set limits */
    	(dump->s)->low=0;
    	(dump->s)->high=0xffff;

	/* set inital context */
    	(dump->s)->context=0;

    	initialize_model(dump->s);

}

int InputBit 
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

	/* increament number of bits used */
	dump->bits++;

	/* check if there is a bit at the mask position or not */
	if (tmp)
	   return 1;
	else
	    return 0;
}

/*
 * Just figuring out what the present symbol is doesn't remove
 * it from the input bit stream.  After the character has been
 * decoded, this routine has to be called to remove it from the
 * input stream.
 */

void remove_symbol_from_stream
(PACKED *dump)
{
    	long range;
	SYMBOL *s;

	/* loas short cut */
	s=dump->s;
   
	/* First, the range is expanded to account for the symbol removal.*/
    	range= (long)(s->high-s->low) + 1;
    	s->high=s->low + (unsigned short)((range*s->high_count)/s->scale-1 );
    	s->low=s->low + (unsigned short)((range*s->low_count)/s->scale );

 	/* Next, any possible bits are shipped out.*/
    	for ( ; ; ) {
 	    /* If the MSDigits match, the bits will be shifted out.*/
	    if ((s->high & 0x8000)==(s->low & 0x8000)){
            }
            /* Else, if underflow is threatening, shift out the 2nd MSDigit.*/
            else if ((s->low & 0x4000)==0x4000 && (s->high & 0x4000)==0){
                    s->code ^= 0x4000;
                    s->low  &= 0x3fff;
                    s->high |= 0x4000;
            } 
	    else
                return; /* Otherwise, nothing can be shifted out, so I return.*/

            s->low <<= 1;
            s->high <<= 1;
            s->high |= 1;
            s->code <<= 1;
            s->code += InputBit(dump);
        }
}
