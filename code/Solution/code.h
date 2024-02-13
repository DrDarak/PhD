#ifndef CODE_H
#define CODE_H
/* main wavelet functions */
#include "struct.h"

/*
  types
*/
typedef enum huff_type {normal,wipe,append,fix} HUFF_TYPE;
typedef enum quant_type {read,create,update} QUANT_TYPE;

/*
  Routines defined within code.c:
*/
void imgcpy(IMAGE *dest,IMAGE *source);
float get_mse(IMAGE *original, IMAGE *image);
void create_optimal_qtable(IMAGE *image,
                           QUANT *quant,
                           float xt,
                           int scales);

void improve_qtables(IMAGE *image,
                     QUANT *quant,
                     QUANT_TYPE mode,
                     float xt,
                     HUFF *dc,
                     HUFF *ac,
                     int scales);

void compress_image(IMAGE *image,
                    HARDCODED_WAVELET *c,
                    QUANT *quant,
                    HUFF_TYPE type,
                    QUANT_TYPE mode,
                    float xt,
                    HUFF *dc,
                    HUFF *ac,
                    int scales,
                    int doReform,
                    int doHuff);

void decompress_image
(IMAGE *image,
 HARDCODED_WAVELET *c,
 QUANT *quant,
 HUFF *dc,
 HUFF *ac,
 int scales,
 int undoReform,
 int undoHuff);

#endif /* CODE_H */
