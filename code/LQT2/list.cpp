//////////////////////////////////////////////////////////////////
//                      David Bethel                            //
//                  Darak@compuserve.com				        //
//					    List sorting					        //
//////////////////////////////////////////////////////////////////

#include <iostream.h>
#include <stdlib.h>

#include "list.h"

//////////////////////////////////////////////////////////////////

// List functions
List::List()
:	m_nItems(0),
	m_start(NULL)
{

}

//////////////////////////////////////////////////////////////////

List::~List()
{

}

//////////////////////////////////////////////////////////////////

bool List::Put(Block *in)
{
	bool ok=true;
	Block *block,*last;
	int error;

	// check for null pointer
	if (!in)
	   ok=false;	

	if (ok)
	   m_nItems++;

	// start new list if empty
	if (!m_start && ok)
	{
	   m_start=in;
	   in->m_sortNext=NULL;// makesure list is terminated
	   return ok;
	}

	if (ok)
	{
		error=in->m_error;
		// greater than top
		if (error>m_start->m_error)
		{
			in->m_sortNext=m_start;
			m_start=in;
			return ok;
		}

		// item is in lists main body
		last=m_start;
		for (block=m_start->m_sortNext;block!=NULL;block=block->m_sortNext)
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

void List::ShowList()
{
	int i;
	Block *block;

	for (i=0,block=m_start;block!=NULL;block=block->m_sortNext,i++)
		cout << i << "\t" << block->m_error << endl;

}

//////////////////////////////////////////////////////////////////

// returns max key value stored in list
int List::Max()
{
	if (m_start)
	   return m_start->m_error;
	else
		return 0;
}

//////////////////////////////////////////////////////////////////

Block *List::Get()
{
	Block *result=NULL;

	if (!m_start)
	   return result;

	m_nItems--; // decrease count of items in list

	result=m_start; // grab tiop of list
	m_start=m_start->m_sortNext; // moves star on one (ok if NULL)
	
	return result;
} 

//////////////////////////////////////////////////////////////////

bool List::MergeLists(List &in)
{
	bool ok=true;
	Block *block,*last,*mergeBlock,*nextMerge;
	int error;
	
	// lists are always sorted an therefore merging 
	// must also be sorted. Operation also destroys 
	// the linking in the incoming list and hence 
	// it must be wiped

	// check for null pointer
	if (!in.m_start || in.m_nItems==0)
	   ok=false;	

	// increase number of items in primary
	if (ok)
	   m_nItems+=in.m_nItems;

	if (ok)
	{	
		last=NULL;
		block=m_start; // initalise start of primary list
		mergeBlock=in.m_start; // initalise start of merge list

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
				mergeBlock->m_sortNext=m_start;
				m_start=mergeBlock;
			}
			last=mergeBlock; // load in last position
			mergeBlock=nextMerge; // move merge block to new location
		}
	}

	// clean up spent list 
	in.m_start=NULL;
	in.m_nItems=0;

	return ok;

}

//////////////////////////////////////////////////////////////////

bool List::ClearListLinking()
{
	Block *next,*block;

	if (!m_start)
	   return false;

	block=m_start;
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
	m_start=NULL;
	m_nItems=0;

	return true;

}

//////////////////////////////////////////////////////////////////
//			Buffered List class									//
//////////////////////////////////////////////////////////////////

BuffList::BuffList(int N)
:	m_maxBufferSize(N),
	m_buffer(),
	m_list()
{

}

//////////////////////////////////////////////////////////////////

BuffList::~BuffList()
{

}

//////////////////////////////////////////////////////////////////

bool BuffList::Put(Block *in)
{
	bool ok=true;
	// sort buffer into list if there are enough items
	if (m_buffer.m_nItems>=m_maxBufferSize)
	   ok=m_list.MergeLists(m_buffer);

	if (ok)
	   ok=m_buffer.Put(in);

	return ok;
}

//////////////////////////////////////////////////////////////////
	
Block * BuffList::Get()
{
	Block *result;

	if (m_buffer.Max() > m_list.Max())
		result=m_buffer.Get();
	else
		result=m_list.Get();

	return result;
}

//////////////////////////////////////////////////////////////////
bool BuffList::ClearListLinking()
{
	bool ok=true;

	ok=(m_buffer.ClearListLinking() && m_list.ClearListLinking());

	return ok;	
}

//////////////////////////////////////////////////////////////////
