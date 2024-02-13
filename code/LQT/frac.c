/* dummy file to load correct version of fractal */
#include "struct.h"

#ifdef BFT
	#include "bft.c"
#else
	#include "implicit.c"
#endif
