/* quantise header */
#ifndef H_QUANTISE_H
#define H_QUANTISE_H

#include "struct.h"

/* Functions used externally */
void quantise_image (IMAGE *, BASIS *);
void unquantise_image (IMAGE *, BASIS *);
void quantise_block (BLOCK *, BASIS *);

#endif  /* H_QUANTISE_H*/
