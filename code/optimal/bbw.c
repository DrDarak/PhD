/* bbw.c */

#include <stdio.h>
#include "bbw.h"

#define SCALES 5
#define TREESIZE 512

void get_pgm_file(IMAGE *image, char *fname);
void write_pgm_file(IMAGE *image, char *fname);

void main(int argc, char *argv[])
{
  IMAGE im, testim;
  NEWTREE *tree;
  char fname[50];
  int i, j, k, l, lowest_h, lowest_w;
  int index=0, treecount, numtrees;
  
  /* Read in image */
  /*  printf("\nInput image: ");
  scanf("%s", fname);*/
  printf("Reading image..."); fflush(stdout);
  get_pgm_file(&im, argv[1]);
  printf("done.\n");

  /* Find dimensions of lowest SB */
  lowest_h = im.height >> SCALES;
  lowest_w = im.width >> SCALES;
  numtrees - lowest_w*lowest_h;
  lowest_h *= 2;  /* Imagining missing SB's are present */ 

  /* Allocate memory for tree and reconstructed image */

  tree = (NEWTREE *)calloc(numtrees, sizeof(NEWTREE));
  if(tree==NULL)
  {
    printf("Error allocating memory - exiting.\n");
    exit(0);
  }
  for(k=0; k<numtrees; k++)
  {
    tree[k].N  = TREESIZE;
    tree[k].pt = (int *)calloc(TREESIZE, sizeof(int));
    if(tree[k].pt==NULL)
    {
      printf("Error allocating memory - exiting.\n");
      exit(0);
    }
  }

  testim.height = lowest_w*lowest_h;
  testim.width  = TREESIZE;
  testim.pt = (int *)calloc(testim.width*testim.height, sizeof(int));

  /* Extract trees */
  printf("Extracting..."); fflush(stdout);
  treecount=0;
  for(i=0; i<lowest_h; i++)
    for(j=0; j<lowest_w; j++)
      {
     printf("%d\n", treecount);
      extract_tree(&im, i, j, &tree[treecount++]); 
      }
  printf("done.\n");    

  /* Copy trees to an image structure */
  printf("Copying flattened trees to IMAGE structure..."); fflush(stdout);
  for(k=0; k<numtrees; k++)
    for(l=0; l<TREESIZE; l++)
      testim.pt[index++] = tree[k].pt[l];
  printf("done.\n");

  /* Replace trees */
  printf("Replacing..."); fflush(stdout);
  treecount=0;
  for(i=0; i<lowest_h; i++)
    for(j=0; j<lowest_w; j++)
      replace_tree(&im, i, j, &tree[treecount++]); 
  printf("done.\n");   

  /* Write out resulting image files */
  printf("Writing tree image and recovered image to file..."); 
  fflush(stdout);
  write_pgm_file(&im, "rec.y");
  write_pgm_file(&testim, "trees.y");
  printf("done.\n");

  /* Housekeeping */
  free(testim.pt);
  free(im.pt);
  for(k=0; k<numtrees; k++)
    free(tree[k].pt);

}

/************************************************************************/

/* Extracts tree rooted in lowest SB from IMAGE structure */
/* wtim - structure containing WT'd image
   i, j - coordinates of tree root in lowest SB
   tree - the extracted tree
*/
int extract_tree(IMAGE *wtimage, int i, int j, NEWTREE *tree)
{
  int treeindex = 0;
  int k, l, w, h, W, H, I, J, b, temp;
  int *wtimpt, *treept; 
  int dummy; 

  w = wtimage->width;
  h = wtimage->height;
  wtimpt = wtimage->pt;
  treept = tree->pt;
  W = w >> (SCALES);
  H = h >> (SCALES); 
  H *=2;   /*Imagine the missing subbands are there */

  /********************* Add root to tree ***********************/
  *(treept + treeindex++) = *(wtimpt + i*w + j);     /* SB 13 */
   
  /******************** Add next finest coeffs ******************/
  temp = (i+H)*w;
  *(treept + treeindex++) = *(wtimpt + temp + j  );    /* SB 12 */
  *(treept + treeindex++) = *(wtimpt + temp + j+W);    /* SB 11 */
  *(treept + treeindex++) = *(wtimpt + i*w  + j+W);    /* SB 10 */

  /************************* And next... **************************/
  /* SB 9 */
  b=2;
  I = (i+H) << 1;
  J = j << 1;
  temp = I*w;

  for(k=0; k<b; k++)
  {
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J  );   
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+1);  
  }

  /* SB 8 */
  I = (i+H) << 1;
  J = (j+W) << 1;
  temp = I*w;
  for(k=0; k<b; k++)
  {
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J  ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+1); 
  }
  
  /* SB 7 */
  I = i << 1;
  J = (j+H) << 1;
  temp = I*w;
  for(k=0; k<b; k++)
  {
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J  );  
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+1);  
  }

  /************************* And next... (yawn) ********************/
  /* SB 6 */
  b = 4;
  I = (i+H) << 2;                                     
  J = j << 2;
  temp = I*w;
  for(k=0; k<b; k++)
  {
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J  ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+1); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+2); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+3); 
  }

  /* SB 5 */
  I = (i+H) << 2;                                     
  J = (j+W) << 2;
  temp = I*w;
  for(k=0; k<b; k++)
  {
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J  ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+1); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+2); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+3); 
  }


  I = i << 2;                                         /* SB 4 */
  J = (j+W) << 2;
  temp = I*w;
  for(k=0; k<b; k++)
  {
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J  ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+1); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+2); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+3); 
  }

  /******************* And next........(zzzzzzzzzzzz) *****************/
  /* SB 4 */
  b = 8;
  I = (i+H) << 3;                                         
  J = j << 3;
  temp = I*w;
  for(k=0; k<b; k++)
  {
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J  ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+1); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+2); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+3); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+4); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+5); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+6); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+7); 
  }

  /* SB 3 */
  I = (i+H) << 3;                                         
  J = (j+W) << 3;
  temp = I*w;
  for(k=0; k<b; k++)
  {
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J  ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+1); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+2); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+3); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+4); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+5); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+6); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+7); 
  }

  /* SB 2 */
  I = i << 3;                                            
  J = (j+W) << 3;
  temp = I*w;
  for(k=0; k<b; k++)
  {
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J  ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+1); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+2); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+3); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+4); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+5); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+6); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+7); 
  }

  /*************** And finally the last funny one **********************/
  /* Unroll with it... SB 0 */
  b = 16;
  I= i << 4;
  J= (W+j) << 4;
  temp = I*w;
  for(k=0; k<b; k++)
  {
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J   ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+1 ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+2 ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+3 ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+4 ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+5 ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+6 ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+7 ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+8 ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+9 ); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+10); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+11); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+12); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+13); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+14); 
    *(treept + treeindex++) = *(wtimpt + (temp+k*w) + J+15); 
  }
}

/**********************************************************************/

/* Replaces tree rooted in lowest SB in an IMAGE structure */
/* wtim - structure containing WT'd image
   i, j - coordinates of treee root in lowest SB
   tree - the extracted tree
*/
int replace_tree(IMAGE *wtimage, int i, int j, NEWTREE *tree)
{  
  int treeindex = 0;
  int k, l, w, h, W, H, I, J, b, temp;
  int *wtimpt, *treept;

  w = wtimage->width;
  h = wtimage->height;
  wtimpt = wtimage->pt;
  treept = tree->pt;
  W = w >> SCALES;
  H = h >> SCALES;
  H *= 2; /* Imagine missing subbands are there */

  /********************* Add root to tree ***********************/
  *(wtimpt + i*w + j) = *(treept + treeindex++);  /* SB 13 */
   
  /******************** Add next finest coeffs ******************/
  temp=(i+H)*w;
  *(wtimpt + temp + j) = *(treept + treeindex++);      /* SB 12 */
  *(wtimpt + temp + j + W) = *(treept + treeindex++);  /* SB 11 */
  *(wtimpt + i*w + j + W) = *(treept + treeindex++);   /* SB 10 */

  /************************* And next... **************************/
  /* SB 9 */
  b=2;
  I = (i+H) << 1;
  J = j << 1;
  temp=I*w;
  for(k=0; k<b; k++)
  {
    *(wtimpt + (temp+k*w) + J  ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+1) = *(treept + treeindex++);  
  }
 
  /* SB 8 */
  I = (i+H) << 1;
  J = (j+W) << 1;
  temp=I*w;
  for(k=0; k<b; k++)
  {
    *(wtimpt + (temp+k*w) + J  ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+1) = *(treept + treeindex++);  
  }

  /* SB 7 */
  I = i << 1;
  J = (j+H) << 1;
  temp=I*w;
  for(k=0; k<b; k++)
  {
    *(wtimpt + (temp+k*w) + J  ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+1) = *(treept + treeindex++);  
  }

  /************************* And next... (yawn) ********************/
  /* SB 6 */
  b=4;
  I = (i+H) << 2;                                    
  J = j << 2;
  temp=I*w;
  for(k=0; k<b; k++)
  {
    *(wtimpt + (temp+k*w) + J  ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+1) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+2) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+3) = *(treept + treeindex++);  
  }

  /* SB 5 */
  I = (i+H) << 2;                                    
  J = (j+W) << 2;
  temp=I*w;
  for(k=0; k<b; k++)
  {
    *(wtimpt + (temp+k*w) + J  ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+1) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+2) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+3) = *(treept + treeindex++);  
  }

  /* SB 4 */
  I = i << 2;                                         
  J = (j+W) << 2;
  temp=I*w;
  for(k=0; k<b; k++)
  {
    *(wtimpt + (temp+k*w) + J  ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+1) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+2) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+3) = *(treept + treeindex++);  
  }

  /******************* And next........(zzzzzzzzzzzz) *****************/
  /* SB 3 */
  b=8;
  I = (i+H) << 3;                                        
  J = j << 3;
  temp=I*w;
  for(k=0; k<b; k++)
  {
    *(wtimpt + (temp+k*w) + J  ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+1) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+2) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+3) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+4) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+5) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+6) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+7) = *(treept + treeindex++);  
  }

  /* SB 2 */
  I = (i+H) << 3;                                         
  J = (j+W) << 3;
  temp=I*w;
  for(k=0; k<b; k++)
  {
    *(wtimpt + (temp+k*w) + J  ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+1) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+2) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+3) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+4) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+5) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+6) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+7) = *(treept + treeindex++);  
  }

  /* SB 1 */
  I = i << 3;                                             
  J = (j+W) << 3;
  temp=I*w;
  for(k=0; k<b; k++)
  {
    *(wtimpt + (temp+k*w) + J  ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+1) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+2) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+3) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+4) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+5) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+6) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+7) = *(treept + treeindex++);  
  }

  /*************** And finally the last funny one **********************/
  /* Unroll with it... SB 0 */
  b=16;
  I= i << 4;
  J= (W+j) << 4;  temp=I*w;
  for(k=0; k<b; k++)
  {
    *(wtimpt + (temp+k*w) + J   ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+1 ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+2 ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+3 ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+4 ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+5 ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+6 ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+7 ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+8 ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+9 ) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+10) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+11) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+12) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+13) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+14) = *(treept + treeindex++);  
    *(wtimpt + (temp+k*w) + J+15) = *(treept + treeindex++);  
  }
}

/**********************************************************************/

/* Compares 2 trees wrt a given threshold.  If new tree is 
significant - old one is overwritten */
/* new - new tree 
   old - old tree
   threshold - error level to decide if sig/insig 
*/
int compare_trees(NEWTREE *new, NEWTREE *old, int threshold)
{
  int i, j, k;
  long error;
  
  /* Find error (mse here) between new and old trees */
  for(k=0; k<new->N; k++)
    error+= (*(new->pt + k) - *(old->pt + k)) * 
                  (*(new->pt + k) - *(old->pt + k));

  error/=new->N;

  /* If error is greater than threshold */
  if(error>threshold)
  {
    /* Copy old into new */
    for(k=0; k<new->N; k++)
      *(old->pt + k) = *(new->pt + k);  
    
    /* Update status data */
    old->update=0;          /* Frames since update now zero */  
    new->significant=1;     /* Tree was significant */
    
    return 1;
  }
  else
  {
    /* Update status data */
    new->significant=0;     /* Tree insignificant */
    (old->update)++;        /* Increment frames since last updated */

    return 0;
  }
}

/*****************************************************************/
/***                    FILE I/O STUFF                         ***/
/*****************************************************************/

void get_pgm_file(IMAGE *image, char *fname)
{
        /* Reads image in P5 format*/
        FILE *fp;
        int i, j, tmp;
        char dummy[256];

        /* Open image file */
        fp = fopen(fname, "r");
        if(fp==0)
        {
           printf("\n File not found - Terminating program\n");
           exit(1);
        }

        /* Skip past P5 line */
        fgets(dummy, 255, fp);

        /* Skip past comment lines */
        do
        {
           fgets(dummy, 255, fp);
        }while (strncmp(dummy, "#", 1)==0);

        /* Read in image stats */
        sscanf(dummy, "%d %d", &(image->width), &(image->height));
        fscanf(fp, "%d\n", &tmp);
	/*printf("\nwidth = %d, height = %d\n", image->width, image->height);
printf("greyscales = %d\n", tmp);
*/
        /* Allocate memory for image */
        /*if(image->pt!=NULL)
           free(image->pt);*/
        image->pt = (int *)calloc(image->width*image->height, sizeof(int));
        if(image->pt==NULL)
	{
          printf("get_pgm_file(): error allocating memory - exiting.\n");
          exit(0);
        }

        /* Read in image data */
        for(i=0; i<image->height*image->width; i++)
	{
	  /*printf("%d\n",  i);  */ 
           image->pt[i] = getc(fp);
        }

        fclose(fp);
}

void write_pgm_file(IMAGE *image, char *fname)
{
        FILE *fp;
        int i,j;
        unsigned char tmp;
 
        fp=fopen(fname,"wb");
 
        if(fp==0)
                {
                printf("\n Can't create file - Terminating program\n");
                exit(1);
                }
 
        /* write image stats + comments */
        fputs("P5",fp);         
        fprintf(fp,"\n# Created by Wavelet Image Compression (daveb 96)#");
        fprintf(fp,"\n%d %d",image->width,image->height);
        fprintf(fp,"\n255\n");                          /* 255 graylevels */
 
        /* write image data */
        for (i=0;i<image->width*image->height;i++){
            if (image->pt[i]>255)
               image->pt[i]=255;
            if (image->pt[i]<0)
               image->pt[i]=0;
           
            putc(image->pt[i],fp);
        }

        fclose(fp);
} 
