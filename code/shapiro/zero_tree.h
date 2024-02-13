#ifndef ZEROTREE_H
#define ZEROTREE_H

/* Image input/ouput functions */
#include "struct.h"

/* Functions used externally */
void zero_tree_code_image(IMAGE *,PACKED *,int );
void zero_tree_decode_image (IMAGE *,PACKED *,int );
void clear_markers(IMAGE *);

#endif

