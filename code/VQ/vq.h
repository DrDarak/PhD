#ifndef H_VQ_H
#define H_VQ_H

typedef struct vq_data {

	int *pt;
	int *acc;
	int size;
	int index;
	int sum2;
	int nitems;
	float entropy;
	float rate;

	struct vq_data *next;


} VQ;

#define myabs(x)    ( x < 0 ? -x : x)

VQ *loadVQ(char *);
VQ *saveVQ(char *,VQ *);
VQ *generateBlankVQ(int ,int );
void normaliseVQ(VQ *);
void printVQ(VQ *);
VQ *newVQ(int ,int );
void deleteVQ(VQ *);
void treeDeleteVQ(VQ *);
VQ *addErrorBlockToVQ(VQ *,BLOCK *);
float findBestVQ(VQ *,VQ *,int *);
float differenceVQ(VQ *, VQ *);
VQ *partitionVQ(VQ *,VQ *,float *);
VQ *removeVQFromList(VQ *,VQ *);
VQ *partitionImage(IMAGE *,VQ *);
void codeBlockVQ(VQ *,BLOCK *);
void renderVQ(IMAGE *,VQ *);

#endif
