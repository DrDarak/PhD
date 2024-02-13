#ifndef H_BLOCKS_H
#define H_BLOCKS_H

#include "struct.h"

/* Functions used externally */
void reform_image (IMAGE *);
void free_blocked_image (IMAGE *);
int setup_blocks (IMAGE *,int );
void shutdown_cosine_basis (BASIS *);
BLOCK *allocate_block_memory (IMAGE *,int ,int ,int );
BLOCK *setup_new_block (int , int , int );  
float calculate_block_error (BLOCK *, BASIS *);
void render_image (IMAGE *, BASIS *);
void render_block (BLOCK *, BASIS *); 
float calculate_scaling (int *, BASIS *);
float calculate_block_scaling (BLOCK *, BASIS *);
void fit_basis_to_block (BLOCK *, BASIS *);
void setup_cosine_basis (BASIS *, int );

#endif  /* H_BLOCKS_H*/
