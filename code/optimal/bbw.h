/* bbw.h */

/* image structure */
typedef struct 
{
  int *pt;     /* image data   */
  int width;   /* image width  */
  int height;  /* image height */
} IMAGE;

/* structure holding subband info */
typedef struct
{
  int x_len;  /* no. rows in block = height */
  int y_len;  /* no. cols in block = width  */
  int x_pos;  /* upper left corner coord    */
  int y_pos;  /* upper left corner coord    */
  int *pt;    /* block data                 */
} BLOCK_ARRAY;


/* The wavelet tree  - NB need one for each colour field */
typedef struct
{
  /* Status stuff */
  int update;           /* How many frames since last sent */
  int significant;      /* Is tree significant */

  /* Tree data */
  int N;                /* Total number of coeffs in tree */
  int i, j;             /* Root of tree */
  int *pt;              /* The actual WT data */
} NEWTREE;

