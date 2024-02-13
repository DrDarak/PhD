/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                   	 quadtree                               */
/*                      reworked 29/8/96                        */
/*##############################################################*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "huff.h"
#include "rd_swtch.h"

void switch_fractals
(IMAGE *image,
 HUFF *frac,
 float rate)
{
	int bits;
	float error;
	BLOCK *block;

	for (block=image->tree_top;block;block=block->next){
	    error=(float)(block->size*block->cf)*Q; 
	    error*=error;
	    bits=estimate_compress_symbol(frac,block->cf);

	    if (bits)
	       if (error/(float)bits < rate) 
		  block->cf=0;
	}

}

void run_length_code_fractal
(HUFF *frac,
 int symbol,
 int initialise, /* initialises run length coding */
 int stop)
{
	static int context,run;

	if (initialise){
	   run=0;
	   context=64000; /* large number*/
	   return;
	}

	/* used to flush run length coder */
	if (stop){
	   if (run)
	      compress_run(frac,run);
	   return;
	}

	if (symbol==context){
	   run++;
	}
	else {
	     context=symbol;
	     if (run){
		compress_run(frac,run);
		run=0;
	     }
	     compress_symbol(frac,symbol);
	}
	if (run>=frac->run){
	   compress_run(frac,run);
	   run=0;
	}
}

int run_length_decode_fractal
(HUFF *frac,
 int initialise) /* initialises run length coding */
{
        static int context,run;
	int run_symbol,run_length;

        if (initialise){
           run=0;
           context=64000; /* large number*/
	   return 0;
        }

	/* udate ongoing runs */
	if (run){
	   run--;
	   return context;
	}

	/* decompress run symbol from stream */
	run_symbol=uncompress_run_symbol(frac,&run_length);

	/* return context and start run countdown if run */
	if (run_length){
	   run=run_symbol-1;
	   return context;
	}

	/* save 'last' symbol */
	context=run_symbol;

	/* return symbol */
	return run_symbol; 
}



