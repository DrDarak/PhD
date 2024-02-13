/* fast rendering / coding functions */
#ifndef H_RENDER_H
#define H_RENDER_H

#include "struct.h"

/* external prototypes */
void render2x2m4 (BLOCK *);
void render4x4m6 (BLOCK *);
void render8x8m6 (BLOCK *);
void render16x16m6 (BLOCK *);
void render32x32m6 (BLOCK *);
void codeNxNm6 (BLOCK *);
void code2x2m4 (BLOCK *);
void code4x4m6 (BLOCK *);
void code8x8m6 (BLOCK *);
void code16x16m6 (BLOCK *);
void code32x32m6 (BLOCK *);

#endif  /* H_RENDER_H*/
