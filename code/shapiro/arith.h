#ifndef ARITH_H
#define ARITH_H

/* arithmetic coder header */
#include "struct.h"

/* Functions used externally */
void initialize_arithmetic_encoder(SYMBOL *);
void OutputBit(PACKED *,unsigned short );
void Compress_symbol(PACKED *,int );
int Expand_symbol(PACKED *);
void initialize_arithmetic_decoder(PACKED *);
int InputBit(PACKED *);

#endif
