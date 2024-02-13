/* Quadtree header */
#ifndef H_QUAD_H
#define H_QUAD_H

#include "struct.h"

/* Functions used externally */
void encode_qtree_image (IMAGE *, PACKED *, int );
void decode_qtree_image (IMAGE *, PACKED *, int );
void form_quad_tree (IMAGE *);

#endif
