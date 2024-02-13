/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                  Wavelet Encoder Program                     */
/*                  + decoder + mirroring                       */
/*                         23/1/96                              */
/*                  Reworked 2/7/96                             */
/*##############################################################*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "struct.h"

extern void get_ppm_file(IMAGE *,IMAGE *,IMAGE *, char *);
extern void write_ppm_file(IMAGE *,IMAGE *,IMAGE *, char *);
extern IMAGE *malloc_IMAGE(int ,char *);
extern WAVELET *malloc_WAVELET(int ,char *);
extern void setup_wavelet (char *, WAVELET *);
extern void wavelet_inv_filter_image (IMAGE *, WAVELET *,int);
extern void wavelet_filter_image (IMAGE *, WAVELET *,int);



void main()
{

	IMAGE *image_y,*image_u,*image_v;
	WAVELET *wavelet;

	wavelet=malloc_WAVELET(1,"");

	image_y=malloc_IMAGE(1,"");
	image_u=malloc_IMAGE(1,"");
	image_v=malloc_IMAGE(1,"");
	setup_wavelet ("bi10_0.4.wav",wavelet);
	printf("git\n");

	get_ppm_file(image_y,image_u,image_v,"pic.raw");
	wavelet_filter_image (image_y,wavelet,4);
	wavelet_inv_filter_image (image_y,wavelet,4);
	write_ppm_file(image_y,image_u,image_v,"new.raw");

}


