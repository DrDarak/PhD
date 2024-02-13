//////////////////////////////////////////////////////////////////
//                      David Bethel                            //
//                     Bath University                          //
//                    Darak@compuserve.com                      //
//                    Huffman encoder                           //
//                      reworked 29/8/96                        //
//						C++ ished 97							//
//////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "huff.h"
#include "IOBuff.h"

//////////////////////////////////////////////////////////////////
//			Leaf functions								    	//
//////////////////////////////////////////////////////////////////
Leaf::Leaf()
{
	SetupLeaf(0,0);
}

//////////////////////////////////////////////////////////////////

Leaf::Leaf(int nitems,int index)
{
	SetupLeaf(nitems,index);
}
//////////////////////////////////////////////////////////////////

Leaf::~Leaf()
{

}

//////////////////////////////////////////////////////////////////
//		sets leaf to inital conditions 							//
//////////////////////////////////////////////////////////////////

void Leaf::SetupLeaf(int nitems,int index)
{
	m_next=NULL;
	m_orig=NULL;
	m_zeroPt=NULL;
	m_onePt=NULL;
	m_nitems=nitems;
	m_index=index;
	m_symbol=0;
	m_length=0;
	m_run=0;

}

//////////////////////////////////////////////////////////////////
//			Huffman functions									//
//////////////////////////////////////////////////////////////////

Huffman::Huffman()
:	m_dump(NULL),
	m_huff(NULL),
    m_huffTop(NULL), 
    m_max(0), 
	m_min(0), 
	m_run(0),
	m_context(0)
{

}

//////////////////////////////////////////////////////////////////

Huffman::~Huffman()
{
	if (m_huffTop)
	   FreeTree(m_huffTop);
	if (m_huff)
	   delete [] m_huff;

}
//////////////////////////////////////////////////////////////////
//Ued to Free the huffman tree form for encoding and decoding 	//
//////////////////////////////////////////////////////////////////

void Huffman::FreeTree
(Leaf *huff)
{
	if (huff==NULL)
	   return;
	
	if (huff->m_zeroPt==NULL)
	   return ; // MUST not free here (free linear array as a whole) 

	FreeTree(huff->m_zeroPt);
    FreeTree(huff->m_onePt);

	delete huff;

	return ;
}

//////////////////////////////////////////////////////////////////
// sets up huffman structures and tree to allow efficien huffman//
// coding 														//
//////////////////////////////////////////////////////////////////

/* FILE STRUCTURE 	

PDF[_CREATE]
# N comments
min\tmax\trun\n
1
1
(max-min+run+1) long
:
:
END_PDF

*/

bool Huffman::SetupHuffmanTree(IOBuff *dump,FILE *fp)
{
	bool ok=true;
	int i,run,max,min,n;
	PDF_TYPE type;
	char dummy[256];

	// default seting 
	type=normal;

	// check files is ok to use 
	if (!fp)
	   ok=false;

	// attach stream pointer 
	if (dump&&ok)
	{
		m_dump=dump;
	}
	else
		ok=false;
	
	// take a line from file
	if (ok)
	{
		fgets(dummy,255,fp);
		if (strncmp(dummy,"PDF",3))	
		   ok=false; 	//PDF tables read error
	}

	if (ok)
	{
		// sets flag to create blank pdf tabels 
		if (!strncmp(dummy,"PDF_CREATE",10))
			type=blank;
		// remove comment lines 
		do 
		{
			fgets(dummy,255,fp);
		} while (strncmp(dummy,"#",1)==0);

		// load max/min from file 
		sscanf(dummy,"%d\t%d\t%d\n",&min,&max,&run);

		// store max / min in packed arry 
		m_min=min;
		m_max=max;
		m_run=run;
	}

	// set up huff symbol stream 
	m_huff=new Leaf[max-min+1+run];
	if (!m_huff)
	   ok=false;

	if (ok)
	{
		// read huffman tree link list from file 
		for (i=min;i<max;i++)
		{	
			// read pdf in or set to 1 
			if (type==normal)
			   fscanf(fp,"%d\n",&n);
			else
				n=1;
			// setup the leaf link list 
            m_huff[i-min].SetupLeaf(n,i);
            m_huff[i-min].m_orig=&(m_huff[i-min+1]);
            m_huff[i-min].m_next=m_huff[i-min].m_orig;
		}
	}

	if (ok)
	{
		if (run>0)
		{
		   // read in run lengths 1->run and max symbol		
			for (i=0;i<run;i++)
			{
				//read pdf in or set to 1 
				if (type==normal)
					  fscanf(fp,"%d\n",&n);
				else
					n=1;

				// setup the leaf link list 
				if (i==0)
					m_huff[i+max-min].SetupLeaf(n,max);
				else 
				{
					m_huff[i+max-min].SetupLeaf(n,i);
					m_huff[i+max-min].m_run=1;
				}

				m_huff[i+max-min].m_orig=&(m_huff[i+max-min+1]);
				m_huff[i+max-min].m_next=m_huff[i+max-min].m_orig;
			}

			if (type==normal)
			   fscanf(fp,"%d\n",&n);
			else
				n=1;

			m_huff[run+max-min].SetupLeaf(n,run);
			m_huff[run+max-min].m_run=1;
		}
		else 
		{
			 if (type==normal)
					fscanf(fp,"%d\n",&n);
			 else
				 n=1;
				 m_huff[i-min].SetupLeaf(n,i);
		}
	}

	// must terminate pdf data with END_PDF .. provents error overflow
	if (ok)
	{
		fgets(dummy,255,fp);
		if (strncmp(dummy,"END_PDF",7)) //PDF tables read error
			ok=false;
	}

	if (ok)
	{
		// initialise huffman tables                 
		m_huffTop=m_huff;
		ConstructHuffTree();
	}

	return ok;
}

//////////////////////////////////////////////////////////////////
// Writes a flat pdf to the standard pdf file on disk. Use for	//
// reformatting the pdf tables									//
//////////////////////////////////////////////////////////////////
 
bool Huffman::WipeHuffmanPDF(FILE *fp)
{
	bool ok=true;
	int i;
 
	if (!fp)
	   ok=false;

	if (ok)
	{
		fprintf(fp,"PDF\n");
		fprintf(fp,"# pdf wipe by program\n");
		fprintf(fp,"%d\t%d\t%d\n",m_min,m_max,m_run);

		for (i=m_min;i<=m_max+m_run;i++)
			fprintf(fp,"%d\n",1);
		
		fprintf(fp,"END_PDF\n");
	}

	return ok;
}

//////////////////////////////////////////////////////////////////
// Writes an existing pdf to standard pdf file on disk. Use for //
// training the pdf tables										//  
//////////////////////////////////////////////////////////////////

bool Huffman::SaveHuffmanPDF(FILE *fp)
{
	bool ok=true;
	int i,max,halfInt,scale;
	
	if (!fp)
		ok=false;

	// find max entery in the list 
	max=0;
	for (i=m_min;i<=m_max+m_run&&ok;i++)
	{
	    if (m_huff[i-m_min].m_nitems>max)
	       max=m_huff[i-m_min].m_nitems;
	    if (m_huff[i-m_min].m_nitems<1)
			ok=false; //Integer overflow, can not append huffman tables
	}

	// may varry with platform 
	halfInt=(1<<16);
	scale=max/halfInt;
	if (scale<2)
	   scale=2;
	
	// reduce all enteries in table to maintain integers 
	
	if (max>halfInt &&ok){
	   //Modifying huffman tables
	   for (i=m_min;i<=m_max+m_run;i++) 
	       if (m_huff[i-m_min].m_nitems/scale>1)
	           m_huff[i-m_min].m_nitems/=scale;
	}

	// write out pdftable to file 
	if (ok)
	{
		fprintf(fp,"PDF\n");
		fprintf(fp,"# pdf appended by program\n");
        fprintf(fp,"%d\t%d\t%d\n",m_min,m_max,m_run);
        for (i=m_min;i<=m_max+m_run;i++)
            fprintf(fp,"%d\n",m_huff[i-m_min].m_nitems);
		fprintf(fp,"END_PDF\n");
	}

	return ok;
}                   


//////////////////////////////////////////////////////////////////
// Estimates from PDF average bits per coefficent for a			//
// particular stream dump										//
//////////////////////////////////////////////////////////////////

float Huffman::HuffmanBPC()
{
	int i,nitems,bits;
	float entropy;

	nitems=0;
	bits=0;
	for (i=m_min;i<=m_max;i++){
            nitems+=m_huff[i-m_min].m_nitems;
	    bits+=m_huff[i-m_min].m_nitems*m_huff[i-m_min].m_length;
	}
	
	entropy=(float)bits/(float)nitems;

	return entropy;

}

//////////////////////////////////////////////////////////////////
// This routine takes a list of symbols with their associated	//
// number of occurences (pdf) and forms the huffman tree. The	//
// huffman symbols and lengths are placed in the original list	//
// and the top of the tree (for decode) is returned				//
//////////////////////////////////////////////////////////////////

bool Huffman::ConstructHuffTree()
{
	bool ok=true;
	Leaf *huffPt,*firstPt,*secondPt;

	// scan through data to creat huff tree 
	while (m_huffTop->m_next!=NULL && ok)
	{
		// search list for min values and delete enteries from list
		if (ok) 
		{
			firstPt=SearchList();
			if (!firstPt)
			   ok=false;
		}
		if (ok)
			ok=DeleteElement(firstPt);

		if (ok) 
		{
			secondPt=SearchList();
			if (!secondPt)
			   ok=false;
		}
		if (ok)
			ok=DeleteElement(secondPt);
		
		// create space for new data combined from previous 2 items
		if (ok)
		   huffPt=new Leaf(firstPt->m_nitems+secondPt->m_nitems,-1);

		if (!huffPt)
		   ok=false;
		
		if (ok)
		{
			// setup tree pointers 
			huffPt->m_zeroPt=firstPt;
			huffPt->m_onePt=secondPt;
			// append data to top of list 
			huffPt->m_next=m_huffTop;
			m_huffTop=huffPt;
		}
	}

	// asigns symbols and lengths to input data 
	if (ok)
	   ok=TraverseTree(m_huffTop,0,0);
	
	return ok;

}

//////////////////////////////////////////////////////////////////
// This routine takes the tree and assignes the appropriate		//
// huffman  symbols to the original root list					//				    
//////////////////////////////////////////////////////////////////

bool Huffman::TraverseTree(Leaf *huffPt,int symbol,int length)
{
	bool ok=true;
	
	if (huffPt->m_zeroPt==NULL)
	{
	   huffPt->m_length=length;
	   huffPt->m_symbol=symbol;
	   if (length>=sizeof(int)*8)
	      ok=false; //interger overflow in huffman tables lengths	   
	   return ok;
	}

	if (ok)
       ok=TraverseTree(huffPt->m_zeroPt,symbol,length+1);
        
	if (ok)
	   ok=TraverseTree(huffPt->m_onePt,symbol | (1<<length),length+1);

	return ok;
}

//////////////////////////////////////////////////////////////////
// This removes items from the list that is scan to form the	//
// huffman tree BUT does not free them as they are used as		//
// nodes in the final tree										//
//////////////////////////////////////////////////////////////////

bool Huffman::DeleteElement(Leaf *delPt)
{
	Leaf *huffPt,*lastPt;
	bool ok=true;

	// return top pointer if it has changed 
	huffPt=m_huffTop;
	if (delPt==m_huffTop)
	{
	   // mends the hole in the list 
	   m_huffTop=huffPt->m_next;
	   huffPt->m_next=NULL;
	   return ok;
	}

	// increament to next position 
	while (huffPt!=NULL)
	{
		lastPt=huffPt;
		huffPt=lastPt->m_next;
		if (huffPt==delPt){
			// mends the hole in the list 
			lastPt->m_next=huffPt->m_next;
			huffPt->m_next=NULL;
			return ok;
		}
	}

	//error in delete element
	ok=false;
	return ok;

}

//////////////////////////////////////////////////////////////////
// This routine searchs the huff list to the largest nitems in	//
// one symbol and returns the pointer to that symbol 			//
//////////////////////////////////////////////////////////////////

Leaf * Huffman::SearchList()
{
	Leaf *minPt,*huffPt;
	int min;

	minPt=NULL;
	min=1024*1024; // large image likely to be encountered 
	
	// scan active list
	for (huffPt=m_huffTop;huffPt!=NULL;huffPt=huffPt->m_next)
	{
		if (huffPt->m_nitems<min)
		{
			min=huffPt->m_nitems;
			minPt=huffPt;
		}
	}

	return minPt;
}


//////////////////////////////////////////////////////////////////
//This routine uses huffman tree to mpress a symbol			    //
//////////////////////////////////////////////////////////////////

int Huffman::CompressSymbol(int symbol)
{
	Leaf *huff;

	if (symbol>m_max)
	{
	   printf("WARNING: Symbol to be compressed is > max[%d]\n",symbol);
	   symbol=m_max;
	}
	if (symbol<m_min)
	{
	   printf("WARNING: Symbol to be compressed is < min[%d]\n",symbol);
	   symbol=m_min;
	}

	//find appropate symbol to compress
	huff=m_huff+symbol-m_min;
	
	//output symbol to bitstream 
	m_dump->WriteNBits(huff->m_symbol,huff->m_length);

	//save context symbol 
	m_context=huff->m_index;

	//increament number of occurences for symbol 
	huff->m_nitems++;

	return (huff->m_length);

}

//////////////////////////////////////////////////////////////////
// This routine uses huffman tree to estimate size of compress	//
// symbol														//
//////////////////////////////////////////////////////////////////

int Huffman::EstimateCompressSymbol(int symbol)
{

	Leaf *huff;

	if (symbol>m_max)
	{
	   printf("WARNING: Symbol to be compressed is > max[%d]\n",symbol);
	   symbol=m_max;
	}

	if (symbol<m_min)
	{
	   printf("WARNING: Symbol to be compressed is < min[%d]\n",symbol);
	   symbol=m_min;
	}

	// find appropate symbol to compress
	huff=m_huff+symbol-m_min;

	return (huff->m_length);

}

//////////////////////////////////////////////////////////////////
// This routine uses huffman tree to compress a runlength symbol//									//
//////////////////////////////////////////////////////////////////
 
int Huffman::CompressRun(int symbol)
{
 
	Leaf *huff;
 
	if (symbol>m_run)
	{
		printf("WARNING: Run lenght Symbol to be compressed > max\n");
		symbol=m_run;
	}

    if (symbol<1)
	{
		printf("WARNING: Run lenght Symbol to be compressed < 1\n");
		symbol=1;
	}

    // find appropate symbol to compress
	huff=m_huff+symbol+m_max-m_min;
         
	// output symbol to bitstream 
	m_dump->WriteNBits(huff->m_symbol,huff->m_length);

	// increament number of occurences for symbol 
    huff->m_nitems++;

    return (huff->m_length);
 
}

//////////////////////////////////////////////////////////////////
// This routine uses huffman tree to estimate size of a			//
// compress a runlength symbol									//
//////////////////////////////////////////////////////////////////
 
int Huffman::EstimateCompressRun(int symbol)
{

    Leaf *huff;

    if (symbol>m_run)
	{
       printf("WARNING: Run lenght Symbol to be compressed > max\n");
       symbol=m_run;
    }

    if (symbol<1)
	{
       printf("WARNING: Run lenght Symbol to be compressed < 1\n");
       symbol=1;
    }

    // find appropate symbol to compress
    huff=m_huff+symbol+m_max-m_min;

    return (huff->m_length);

}                              


//////////////////////////////////////////////////////////////////
//		This routine decodeds the huffman stream				//
//////////////////////////////////////////////////////////////////

int Huffman::UncompressSymbol()
{
 	Leaf *huff;

	huff=m_huffTop;
 
	for (;;)
	{ 
	    // continue until symbol decoded 
	    if (huff->m_zeroPt==NULL)
		{
           huff->m_nitems++;
           return huff->m_index;
	    }

        if (m_dump->Read1Bit())
	       huff=huff->m_onePt;
	    else
	        huff=huff->m_zeroPt;
	}

	return 0;
            
}

//////////////////////////////////////////////////////////////////
// This routine decodeds possible runs from the huffman stream  //                     */
//////////////////////////////////////////////////////////////////

int Huffman::UncompressRunSymbol(int &run)
{
    Leaf *huff;

    huff=m_huffTop;
    run=0; // no run 

    for (;;){               
        // continue until symbol decoded 
        if (huff->m_zeroPt==NULL){
           huff->m_nitems++;
           run=huff->m_run;
           return huff->m_index;
        }
        if (m_dump->Read1Bit())
           huff=huff->m_onePt;
        else
            huff=huff->m_zeroPt;
    }

}     



