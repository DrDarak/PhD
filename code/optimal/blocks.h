/* memory handleing header */
#include "struct.h"

/* Functions used externally */
void setup_blocks (IMAGE *, int );
void reform_image (IMAGE *, int);
void free_blocked_image (IMAGE *);

/* Internal functions */
void tree_block (IMAGE *, BLOCK *,int); 
void untree_block  (IMAGE *, BLOCK *, int);

/* External functions */
extern int *malloc_int(int ,char *);
extern void free_int (int *pt);
extern BLOCK *malloc_BLOCK(int ,char *);
extern void free_BLOCK (BLOCK *pt);

/* defines */

/* macros */
