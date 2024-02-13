#ifndef H_LIST_H
#define H_LIST_H

#include "blocks.h"

class List {
public:

	List ();
	~List();
	
	bool Put(Block *in); // puts node to list.. needs improving
	int Max(); // gets max key value stored
	
	Block *Get(); //gets top node from list
	void ShowList();
	bool MergeLists(List &in);
	bool ClearListLinking();

	// list information
    Block *m_start; // start of linked list
	int m_nItems; // number of items

};


class BuffList {
public:		
	BuffList(int N); // constructor N long buffer
	~BuffList();

	// sorted buffer for incoming information
	List m_buffer;
	int m_maxBufferSize;
	// List of sorted information
	List m_list;

	// functions
	bool ClearListLinking();
	bool Put(Block *in);
	Block *Get();

};


#endif /* H_List_H */