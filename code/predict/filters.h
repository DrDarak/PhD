#ifndef FILTERS_H
#define FILTERS_H

/* Image input/ouput functions */
#include "struct.h"

/* Functions used externally */
void wavelet_inv_filter_image (IMAGE *, int);
void wavelet_filter_image (IMAGE *, int);

#endif
