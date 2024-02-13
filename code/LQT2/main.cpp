#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "Qtree.h"


void main
(int argc,
 char *argv[])
{
	bool ok=true;
	char dummy[256];
	int timer;
	unsigned char buff[400000];
	int i,last;
	QtreeCodec codec; 
	float bpp;
	HUFF_TYPE type;
	char input_fname[256],output_fname[256],pdf_fname[256];

		/* set defaults */
        sprintf(output_fname,"new.lqt");
        sprintf(pdf_fname,"pdf.all");
        last=0;
	type=normal;

        /* check arguments */
        for (i=1;i<argc;i++){

            /* outpur file option */
            if (!strcmp(argv[i],"-o")){
               i++;
               if (argv[i]==NULL){
                  printf("NULL output argument\n");
                  exit(-1);
               } 
               sprintf(output_fname,"%s",argv[i]);
               /*printf("-o: %s\n",output_fname);*/
               last=i;
               continue;
            }

	    /*  append option */
            if (!strcmp(argv[i],"-a")){
	       type=append;
               last=i;
               continue;
            }

	    /* wipe pdf file option */
            if (!strcmp(argv[i],"-w")){
	       type=wipe;
               last=i;
               continue;
            }
 
            /* pdf  file option */
            if (!strcmp(argv[i],"-p")){
               i++;
	       if (argv[i]==NULL){
                  printf("NULL output argument\n");
                  exit(-1);
               } 
               sprintf(pdf_fname,"%s",argv[i]);
               /*printf("-w: %s\n",wavelet_fname);*/
               last=i;
               continue;
            }
 
            /* unknown option */
            if (!strncmp(argv[i],"-",1)){
               printf("Unknown option\n");
               exit(-1);
            }
        }
 
        /* warn if wrong input length */
        if (last+2>=argc){
           printf("%s:"
        "[-a ] appends huffman pdf"
        "[-w ] wipe huffman pdf"
        "[-o <output>]   default: new.lqt"
        "[-p <pdf file>] default: pdf.all"
        "<bpp>   - compression"
        "<input> - pgm/ppm file\n",argv[0]);
           exit(-1);
        }
 
        /* load last 2 items */
        bpp=atof(argv[last+1]);
        sprintf(input_fname,"%s",argv[last+2]);
	
	// set defaults 
	codec.SetFileNames(input_fname,output_fname,pdf_fname);
	codec.SetCompression(bpp);

	codec.m_type=type;
	if (ok)
	   ok=codec.SetupPDF();

	if (type==wipe){
		codec.WipePDF();
		printf("wipeing huffman pdf file\n");
       exit(-1);
    }
if (ok)
{
	codec.m_compressed->AddExternalMemoryBuffer(buff,400000);


/*
	codec.SetFileNames("lab.ppm","new.lqt",NULL);
	codec.CompressImageFromFile();
//	return;
;
	timer=clock();
	for (i=0;i<0;i++)
	{
		codec.CompressImage();
	}
	timer=clock()-timer;
	printf("time=%d\n",timer/20);

//	return;
    
	if (type==append){
	   codec.AppendPDF();
       printf("appending huffman pdf to disk\n"); 
    } 
	else{
		codec.m_writeResultToFile=true;
		codec.SetFileNames("new.lqt","new.ppm",NULL);
	    codec.DecompressImageFromFile();
		timer=clock();
		for (i=0;i<0;i++)
		{
			codec.DecompressImage();
		}
		timer=clock()-timer;
		printf("time=%d\n",timer/10);


	}
	
*/
    codec.m_writeResultToFile=false;
	codec.m_image->m_imageIsYUV422=true;
	codec.m_noUpdate=true;
	codec.SetFileNames("lab.ppm","new.lqt",NULL);
	codec.CompressImageFromFile();

//	codec.SetFileNames("new.lqt","new.ppm",NULL);
//	codec.DecompressImage();//FromFile();
//	printf("%d\n",i);
	timer=clock();
	for (i=0;i<50;i++)
	{
		codec.m_image->m_imageIsYUV422=true;
		codec.m_noUpdate=false;
		sprintf(dummy,"output\\new%d.lqt",i);
		codec.SetFileNames(NULL,dummy,NULL);
		codec.CompressImage();


		codec.SetFileNames("new.lqt",dummy,NULL);
		codec.DecompressImage();//FromFile();
//		printf("%d\n",i);
	}
	timer=clock()-timer;
	printf("time=%d\n",timer/50);
	
	codec.m_writeResultToFile=true;
	codec.SetFileNames("new.lqt","new.ppm",NULL);
	codec.DecompressImage();//FromFile();
}
	return ;
}