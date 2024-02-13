/* Rate distortion Switch header */
#ifndef H_RD_SWITCH_H
#define H_RD_SWITCH_H

#include "struct.h"
void switch_fractals (IMAGE *, HUFF *, float );
void run_length_code_fractal (HUFF *,int , int ,int );
int run_length_decode_fractal (HUFF *,int );

#endif  /* H_MEM_HAND_H*/
