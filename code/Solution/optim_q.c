/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                  optimal block quantiser                     */
/*                   spring 96 +6/12/96                         */
/*##############################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "miscw.h"
#include "struct.h"
#include "mem_hand.h"
#include "huff.h"
#include "bitstrm.h"

#include "optim_q.h"

#ifdef __cplusplus
#  include <memleak.h> /* must be the last .h file included */
#endif

#if defined(__cplusplus)
inline
#endif
int quantise(int c, int q)
{
 return(q > 0 ? c/q : 0);
}

/****************************************************************/
/*
  Routines defined & only used within this module
*/
void huffman_encode_block(BLOCK      *block,
                          int        *q_out,
                          HUFF       *dc,
                          HUFF       *ac,
						                    BitBuffer&  dump);

/****************************************************************/

void read_quantisation
(char *fname,
 QUANT *quant)
{

	int i;
	FILE *fp;
	
	/* open and read quant file */
	fp=fopen(fname,"rb");
	NULL_FILE_error(fp,"read quant");
	/* read quant file */
	fscanf(fp,"%f\n",&(quant->bpp));
	for (i=0;i<quant->size;i++){
	    fscanf(fp,"%d\t%d\n",quant->q_out+i,quant->nitems+i);
#ifdef __cplusplus
     Assert( (quant->q_out[i] == -1)    ||
             ( (quant->q_out[i] > 0) &&
               (quant->q_out[i] < 800)  ) );
     Assert( (quant->nitems[i] == 0) ||
             (quant->q_out[i] > 0)     );
#endif
	    quant->q[i]=(float)(quant->q_out[i]);
	}
	fclose(fp);
	
}

void huffman_encode_image(IMAGE *image,
                          int   *q_out,
                          HUFF  *dc,
                          HUFF  *ac)
{
 BLOCK *block;

 block=image->tree_top;

 while (block!=NULL)
       {
        huffman_encode_block(block,q_out,dc,ac,image->dump);
        block = block->next;
       }
}

static
/* inline */                     
void huffman_encode_block(BLOCK      *block,
                          int        *q_out,
                          HUFF       *dc,
                          HUFF       *ac,
                          BitBuffer&  dump)
{
 int cq,n,i,run,context;
 int *blockPtr = block->pt; // might be a useful speed up :/
 const int acrun = ac->run;       // another speed up :/
 const int bsize = block->size;   // another ?

 /* record start of tree data */
 block->dump_size = dump.DirectTellBit();

 /* next compress block */
 cq = quantise(blockPtr[0],q_out[0]);
 compress_symbol(dc,dump,cq);

 /* setup for run length */
 context = 32768;
 run     = 0;

 for (i=1;i<bsize;)
     {
      cq = quantise(blockPtr[i],q_out[i]);
      
      if (cq==context)	// another run
         {
          run++;
         }
      else
         {
          context = cq;	// change context

          if (run)
             {
              compress_run(ac,dump,run); // compress run
              run = 0;
             }

          compress_symbol(ac,dump,cq); // compress coeff which 
                                       // changed context
         }
      
      i++;          // increment to next position	in wavelet tree
      
      /* check for jumps(ac->run-1) and stops (ac->run) and max runs */
      
      if (run>=acrun-2) // was run+1>=acrun-1
         {
          if (!context)	   // only interested in zeros
              while (n=check_for_jump(block,q_out,i-run))
                    {
                     if (n<i)   // uses a max run size to skip subands if 
                         break; // it is more effective than jump

                     i   = n;
                     run = 0;

                     if (n==bsize)
                        {
                         compress_run(ac,dump,acrun);   // stop symbol 
                         break;
                        }
      
                     // default case ... jump symbol
                     compress_run(ac,dump,acrun-1);   // jump symbol
                    }
      
          if (run)
             {
              compress_run(ac,dump,run); // compress max run .. context has
                                         // not changed
              run = 0;
             }
         }
     }

 if (run)
     compress_run(ac,dump,acrun); /* stop symbol */

 block->n_compressed_bits=dump.DirectTellBit()-block->dump_size; // record length of
                                                                  // compressed block
 return;
}                                             

int check_for_jump(BLOCK *block,
                   int   *q_out,
                   int    x)
{
	int n;
 int *blockPtr = block->pt + x;

 q_out += x;

	n = find_jump_position(x); // find limit for wavelet

	for ( ;x<n;x++)
		   //if (quantise(*blockPtr++,*q_out++))
     if((*q_out++ > 0) && *blockPtr++)
		      return 0;		// returns NULL if there is no jump
	
	return n;				// return limit if a jump exists
}

int find_jump_position(int x)
{
	int n,N;

	/* works out postion limit in a wavelet structure */

 // Find how many bits x occupies
	for (n=0;x>>n;n++);

 n--;   // 'divide' by 2 sort of
 n&=~1; // strip of bit 0 if there is one.

	n=1<<n;
	for (N=n+n;N<=x;N+=n);

	return N;
}

void huffman_decode_image
(IMAGE *image,
 int *q_out,
 HUFF *dc,
 HUFF *ac)
{
        BLOCK *block;
 
        block=image->tree_top;
        while (block!=NULL){
	          huffman_decode_block(block,q_out,dc,ac,image->dump);
              block=block->next;
        }                        

}         

void huffman_decode_block(BLOCK      *block,
                          int        *q_out, 
                          HUFF       *dc, 
                          HUFF       *ac,
                          BitStream&  dump)
{
 int i,j,n,run,run_symbol,context;
 int *blockPtr = block->pt;
 const int bsize = block->size;

 context = 32768;

 blockPtr[0] = q_out[0]*uncompress_symbol(dc,dump);

 for (i=1;i<bsize;)
     {
      run_symbol = uncompress_run_symbol(ac,dump,run);

      if (run)
         {
          if (run_symbol<ac->run-1)
             {
              for (j=0;j<run_symbol;j++)
                   blockPtr[i++] = context;  // decode standard run
             }
          else
             {
              if (run_symbol==ac->run)
                 {
                  for (j=i;j<block->size;j++) // could be problem
                       blockPtr[i++] = 0;		 // decode stop symbol
                 }
              else
                 {
                  n=find_jump_position(i);
                  for (j=i;j<n;j++)			// could be problem
                       blockPtr[i++] = 0;		// decode jump symbol
                 }
             } 
         }
      else
         {
          if (q_out[i] > 0)
              context = q_out[i]*run_symbol;
          else
              context = 0;

          blockPtr[i++] = context;
         }
     }
}



