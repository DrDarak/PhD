//////////////////////////////////////////////////////////////////
//                      David Bethel                            //
//                  Darak@compuserve.com				        //
//						misc									//
//////////////////////////////////////////////////////////////////

#include "codec.h"
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////
//							video data  sorting					                         //
///////////////////////////////////////////////////////////////////////////////////////////

VideoData *VideoData_Create
(int pitch, 
 int width, 
 int height,
 bool cache,
 bool onlyY)
{
	VideoData *inst;
	bool ok=true;
	int sizeY,sizeUV,i;

	sizeY = (pitch * (height+4));
    sizeUV = ((pitch >> 1) * (height+4));
	// caste memory as approprate
	inst=(VideoData *)malloc(sizeof(VideoData));
	inst->m_Y=(unsigned char *)malloc(sizeY);
	if (onlyY)
	{
		inst->m_U=0;
		inst->m_V=0;
	}
	else
	{
		inst->m_U=(unsigned char *)malloc(sizeUV);
		inst->m_V=(unsigned char *)malloc(sizeUV);
	}
	
	inst->m_width=width;
	inst->m_height=height;
	inst->m_pitch=pitch;
	inst->m_camera=0;
	SetupSystemTime(&inst->m_time);
	inst->m_basePtr=0;
	inst->m_size=0;

	inst->m_hasTextTime=false;
	inst->m_timeX=0;
	inst->m_timeY=0;
	inst->m_hasTextDate=false;
	inst->m_dateX=0;
	inst->m_dateY=0;

	inst->m_hasText=0;
	for (i=0;i<MAX_VIDEO_TEXT;i++)
	{
		inst->m_x[i]=0;
		inst->m_y[i]=0;
		memset(inst->m_text[i],0,64);
	}

	memset ( (char *)inst->m_Y, 0x00, pitch*height);
	if (!onlyY)
	{
		memset ( (char *)inst->m_U, 0x80, pitch*height/2);
		memset ( (char *)inst->m_V, 0x80, pitch*height/2);
	}

	return inst;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DeReferenceSystemTime(SystemTime *inst,SystemTime *ref)
{
	int total;
	int next;
	int rem;
	int maxDays;
	int day,month,year;
	
	if (!inst || !ref)
		return;

	// frames
	total=(inst->m_relativeTime-ref->m_relativeTime)+ref->m_frames;
	next=total/50;
	rem=total-50*next;
	inst->m_frames=rem;

	// seconds
	total=next+ref->m_seconds;
	next=total/60;
	rem=total-60*next;
	inst->m_seconds=rem;

	// minutes
	total=next+ref->m_mins;
	next=total/60;
	rem=total-60*next;
	inst->m_mins=rem;

	// hours
	total=next+ref->m_hours;
	next=total/24;
	rem=total-24*next;
	inst->m_hours=rem;

	// days - only works for 1 month past ref date

	day=next+ref->m_days;
	month=ref->m_months;
	year=ref->m_years;

	do 
	{
		maxDays=31;
		if (month==3 ||	month==5 ||
			month==8 ||	month==10)
			maxDays=30;

		if (month==1)
		{
			maxDays=28;
			if ((year & 0x3)==0)	// leap year
				maxDays++;
		}
		
		if (day>=maxDays)
		{
			month++;
			day-=maxDays;
		}

		if (month>=12)
		{
			month-=12;
			year++;
		}
		
	}	while (day>=maxDays);
	
	// set days and months
	inst->m_days=day;
	inst->m_months=month;
	inst->m_years=year;

	sprintf(inst->m_time,"%-2.2d:%-2.2d:%-2.2d:%-2.2d",inst->m_hours,inst->m_mins,
													inst->m_seconds,inst->m_frames);
	sprintf(inst->m_date,"%-2.2d/%-2.2d/%-4.4d",inst->m_days+1,inst->m_months+1,
													inst->m_years);
	return ;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool SetupSystemTime(SystemTime *inst)
{
	inst->m_relativeTime=0;
	inst->m_frames=0;
	inst->m_seconds=0;
	inst->m_mins=0;
	inst->m_hours=0;
	inst->m_days=0;
	inst->m_months=0;
	inst->m_years=0;
	inst->m_time[0]=0;
	inst->m_date[0]=0;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
/*
VideoData *VideoData_Create
(int pitch, 
 int width, 
 int height,
 bool cache,
 bool onlyY)
{
	VideoData *inst;
	unsigned char *basePtr;
	unsigned int size,dataSize;
	bool ok=true;
	int sizeY,sizeUV,i;

	sizeY = (pitch * (height+4));
    sizeUV = ((pitch >> 1) * (height+4));

	dataSize=sizeof (VideoData);
	dataSize += 63;
	dataSize &=~63;	//must fall on byte boundary

	// create 1 big lump of memory
	if (onlyY)
		size=dataSize+sizeY;
	else
		size=dataSize+sizeY+2*sizeUV;
		
	basePtr=(unsigned char *)malloc(size);

	if (!basePtr)
		return NULL;

	// caste memory as approprate
	inst=(VideoData *)basePtr;

	inst->m_Y=(unsigned char *)(basePtr+dataSize);
	if (onlyY)
	{
		inst->m_U=0;
		inst->m_V=0;
	}
	else
	{
		inst->m_U=(unsigned char *)(basePtr+dataSize+sizeY);
		inst->m_V=(unsigned char *)(basePtr+dataSize+sizeY+sizeUV);
	}
	
	inst->m_width=width;
	inst->m_height=height;
	inst->m_pitch=pitch;
	inst->m_camera=0;
	memset(&inst->m_time,0,sizeof(SystemTime));
	inst->m_basePtr=basePtr;
	inst->m_size=size;

	inst->m_hasTextTime=false;
	inst->m_timeX=0;
	inst->m_timeY=0;
	inst->m_hasTextDate=false;
	inst->m_dateX=0;
	inst->m_dateY=0;

	inst->m_hasText=0;
	for (i=0;i<MAX_VIDEO_TEXT;i++)
	{
		inst->m_x[i]=0;
		inst->m_y[i]=0;
		memset(inst->m_text[i],0,64);
	}

	memset ( (char *)inst->m_Y, 0x00, pitch*height);
	if (!onlyY)
	{
		memset ( (char *)inst->m_U, 0x80, pitch*height/2);
		memset ( (char *)inst->m_V, 0x80, pitch*height/2);
	}

	return inst;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////

void VideoData_Copy(VideoData **destPtr,VideoData *src)
{
	int i,j;
	int width,height,hPitch,pitch,hInputPitch,inputPitch,hWidth;
	unsigned char *Y,*U,*V,*inY,*inU,*inV;
	VideoData *dest=*destPtr;

	width=src->m_width;
	height=src->m_height;
	hWidth=width>>1;
	if (dest->m_width!=src->m_width ||
		dest->m_height!=src->m_height)
	{
		pitch=(width+0x1f)&~0x1f;	// convert to units of 32
		dest=VideoData_Create(pitch,width,height,false,false);

		// copy stuff from clear over
		dest->m_camera=(*destPtr)->m_camera;
		dest->m_time=(*destPtr)->m_time;
		dest->m_hasTextTime=(*destPtr)->m_hasTextTime;
		dest->m_timeX=(*destPtr)->m_timeX;
		dest->m_timeY=(*destPtr)->m_timeY;
		dest->m_hasTextDate=(*destPtr)->m_hasTextDate;
		dest->m_dateX=(*destPtr)->m_dateX;
		dest->m_dateY=(*destPtr)->m_dateY;
		dest->m_hasText=(*destPtr)->m_hasText;
		for (i=0;i<MAX_VIDEO_TEXT;i++)
		{
			dest->m_x[i]=(*destPtr)->m_x[i];
			dest->m_y[i]=(*destPtr)->m_y[i];
			memcpy(dest->m_text[i],(*destPtr)->m_text[i],64);
		}
		VideoData_Destroy(destPtr);
		*destPtr=dest;
	}

	Y=dest->m_Y;
	U=dest->m_U;
	V=dest->m_V;
	pitch=dest->m_pitch;
	hPitch=pitch>>1;
	inY=src->m_Y;
	inU=src->m_U;
	inV=src->m_V;
	inputPitch=src->m_pitch;
	hInputPitch=inputPitch>>1;

	for (j=0;j<height;j++)
	{
		memcpy(Y,inY,width);
		memcpy(U,inU,hWidth);
		memcpy(V,inV,hWidth);
		Y+=pitch;
		U+=hPitch;
		V+=hPitch;
		inY+=inputPitch;
		inU+=hInputPitch;
		inV+=hInputPitch;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////

void VideoData_Destroy(VideoData **inst)
{
	if (*inst)
	{
		if ((*inst)->m_basePtr)
			free((*inst)->m_basePtr);
		else
		{
			if ((*inst)->m_Y)
				free((*inst)->m_Y);
			if ((*inst)->m_U)
				free((*inst)->m_U);
			if ((*inst)->m_V)
				free((*inst)->m_V);
			free(*inst);
		}
		*inst=NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////

void VideoData_Set(VideoData *inst,unsigned char value)
{
	if (!inst)
		return ;
	memset(inst->m_Y,value, inst->m_pitch*inst->m_height);
	if (inst->m_U)		
		memset(inst->m_U, value, inst->m_pitch*inst->m_height/2);
	if (inst->m_V)			
		memset(inst->m_V, value, inst->m_pitch*inst->m_height/2);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool SetupSystemTime(SystemTime *inst);
void DeReferenceSystemTime(SystemTime *inst,SystemTime *ref);

///////////////////////////////////////////////////////////////////////////////////////////
//					    List sorting												     //
///////////////////////////////////////////////////////////////////////////////////////////

BlockList *BlockList_Create()
{
	BlockList *inst;
	inst=(BlockList *)malloc(sizeof(BlockList));
	inst->m_nItems=0;
	inst->m_start=NULL;
	return inst;
}

//////////////////////////////////////////////////////////////////

void BlockList_Destroy(BlockList **inst)
{
	if (*inst)
		free(*inst);
	*inst=NULL;
}

//////////////////////////////////////////////////////////////////

bool BlockList_Put(BlockList *inst,Block *in)
{
	bool ok=true;
	Block *block,*last;
	int error;

	// check for null pointer
	if (!in)
	   ok=false;	

	if (ok)
	   inst->m_nItems++;

	// start new list if empty
	if (!inst->m_start && ok)
	{
	   inst->m_start=in;
	   in->m_sortNext=NULL;// makesure list is terminated
	   return ok;
	}

	if (ok)
	{
		error=in->m_error;
		// greater than top
		if (error>inst->m_start->m_error)
		{
			in->m_sortNext=inst->m_start;
			inst->m_start=in;
			return ok;
		}

		// item is in lists main body
		last=inst->m_start;
		for (block=inst->m_start->m_sortNext;block!=NULL;block=block->m_sortNext)
		{
			if (error>=block->m_error)
			{
			   last->m_sortNext=in;
			   in->m_sortNext=block;
			   return ok;
			}
			last=block; // load in last position
		}

		// item needs to be appened to end of list
		last->m_sortNext=in;
		in->m_sortNext=NULL; // terminate list correctly	
	}

	return ok;

}

//////////////////////////////////////////////////////////////////

// returns max key value stored in list

int BlockList_Max(BlockList *inst)
{
	if (inst->m_start)
	   return inst->m_start->m_error;
	else
		return 0;
}

//////////////////////////////////////////////////////////////////

Block *BlockList_Get(BlockList *inst)
{
	Block *result=NULL;

	if (!inst->m_start)
	   return result;

	inst->m_nItems--; // decrease count of items in list
	result=inst->m_start; // grab tiop of list
	inst->m_start=inst->m_start->m_sortNext; // moves star on one (ok if NULL)
	return result;
} 

//////////////////////////////////////////////////////////////////

bool BlockList_MergeLists(BlockList *inst,BlockList *in) //&
{
	bool ok=true;
	Block *block,*last,*mergeBlock,*nextMerge;
	int error;
	
	// lists are always sorted an therefore merging 
	// must also be sorted. Operation also destroys 
	// the linking in the incoming list and hence 
	// it must be wiped

	// check for null pointer
	if (!in->m_start || in->m_nItems==0)
	   ok=false;	

	// increase number of items in primary
	if (ok)
	   inst->m_nItems+=in->m_nItems;

	if (ok)
	{	
		last=NULL;
		block=inst->m_start; // initalise start of primary list
		mergeBlock=in->m_start; // initalise start of merge list

		while (mergeBlock!=NULL)
		{
			// find location to place blocks
		    error=mergeBlock->m_error;
			// save location of Merge block
			nextMerge=mergeBlock->m_sortNext; 

			while (block!=NULL)
			{
				if (error>=block->m_error)
				   break;
				last=block;
				block=block->m_sortNext;
			}
			// decide how to place blocks
			if (last)
			{
				last->m_sortNext=mergeBlock;
				if (block)
				   mergeBlock->m_sortNext=block;
				else
				   mergeBlock->m_sortNext=NULL;
			}
			else
			{
				// only ocurrs at top of list once
				mergeBlock->m_sortNext=inst->m_start;
				inst->m_start=mergeBlock;
			}
			last=mergeBlock; // load in last position
			mergeBlock=nextMerge; // move merge block to new location
		}
	}

	// clean up spent list 
	in->m_start=NULL;
	in->m_nItems=0;

	return ok;

}

//////////////////////////////////////////////////////////////////

bool BlockList_ClearListLinking(BlockList *inst)
{
	Block *next,*block;

	if (!inst->m_start)
	   return false;

	block=inst->m_start;
	while (block)
	{
		// sort next
		next=block->m_sortNext;
		// remove link only !
		block->m_sortNext=NULL;
		// move on
		block=next;
	}

	//remove top marker
	inst->m_start=NULL;
	inst->m_nItems=0;

	return true;

}


//////////////////////////////////////////////////////////////////
//			Buffered List class									//
//////////////////////////////////////////////////////////////////

BuffList *BuffList_Create(int N)
{
	BuffList *inst;
	inst=(BuffList *)malloc(sizeof(BuffList));
	inst->m_maxBufferSize=N;
	inst->m_buffer=BlockList_Create();
	inst->m_list=BlockList_Create();
	inst->m_getNew=false;
	return inst;
}

//////////////////////////////////////////////////////////////////

void BuffList_Destroy(BuffList **inst)
{
	if (*inst)
	{
		BlockList_Destroy(&(*inst)->m_buffer);
		BlockList_Destroy(&(*inst)->m_list);
		free(*inst);
		*inst=NULL;
	}
}

//////////////////////////////////////////////////////////////////

bool BuffList_Put(BuffList *inst,Block *in)
{
	bool ok=true;
	BlockList *m_buffer=inst->m_buffer;
	// sort buffer into list if there are enough items
	if (m_buffer->m_nItems>=inst->m_maxBufferSize)
	   ok=BlockList_MergeLists(inst->m_list,m_buffer);

	if (ok)
	   ok=BlockList_Put(m_buffer,in);
	return ok;
}

//////////////////////////////////////////////////////////////////
	
Block * BuffList_Get(BuffList *inst)
{
	Block *result;
	do
	{
		if (inst->m_getNew)
		{
			result=BlockList_Get(inst->m_list);
			if (!result)
				result=BlockList_Get(inst->m_buffer);			
		}
		else
		{
			if (BlockList_Max(inst->m_buffer) > BlockList_Max(inst->m_list))
				result=BlockList_Get(inst->m_buffer);
			else
				result=BlockList_Get(inst->m_list);
		}
	}
	while (result && inst->m_getNew && result->m_blockType==currentFrameBlock);
	return result;
}

//////////////////////////////////////////////////////////////////
bool BuffList_ClearListLinking(BuffList *inst)
{
	bool ok=true;

	ok=(BlockList_ClearListLinking(inst->m_buffer) && 
		BlockList_ClearListLinking(inst->m_list));

	return ok;	
}

//////////////////////////////////////////////////////////////////
