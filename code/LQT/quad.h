/* quad tree handling header */
#ifndef H_QUAD_H
#define H_QUAD_H

#include "struct.h"

/* Functions used externally */
void encode_qtree_image (IMAGE *, HUFF *, int );
void decode_qtree_image (IMAGE *, HUFF *, int );
void fit_basis_to_image (IMAGE *, BASIS *, int );

#endif  /* H_QUAD_H*/
