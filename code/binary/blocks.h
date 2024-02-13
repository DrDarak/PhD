/* Blocks header */
#ifndef H_BLOCKS_H
#define H_BLOCKS_H

#include "struct.h"

/* Functions used externally */
void reform_image (IMAGE *);
void free_blocked_image (IMAGE *);
int setup_blocks (IMAGE *,int );
BLOCK *allocate_block_memory (int ,int ,int );
BLOCK *setup_new_block (int , int , int );  

#endif
