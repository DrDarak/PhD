//////////////////////////////////////////////////////////////////
//                      David Bethel                            //
//					  Darak@compuserve.com 					    //
//                    Input output buffer                       //
//////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "iobuff.h"

#define limit(x) ((x) > 255 ? 255 : (x < 0 ? 0 :x))

// class constant
const unsigned char IOBuff::c_mask[8]=
{	
	0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,
};
//////////////////////////////////////////////////////////////////

IOBuff::IOBuff()
{
	m_pt=NULL;
	m_size=0;
	m_pos=0;
	m_memCreatedHere=false;
}

//////////////////////////////////////////////////////////////////

IOBuff::IOBuff(unsigned char *pt, int size)
{
	m_pt=pt;
	m_size=size;
	m_pos=0;
	m_memCreatedHere=false;
}

//////////////////////////////////////////////////////////////////

IOBuff::~IOBuff()
{
	if (m_memCreatedHere && m_pt)
	   delete [] m_pt;
}

//////////////////////////////////////////////////////////////////

bool IOBuff::CreateMemoryBuffer(int size,bool force)
{
	bool ok=true;

	// delete old buffer if applicable
	if (m_pt && m_memCreatedHere){
	   if (!force && m_size>=size)
	      return ok;
	   else
	       delete [] m_pt;
	}

	// create new memory
	m_pt=new unsigned char [size];

	// check for problem
	if (!m_pt)
	   ok=false;

	// react to problem
	if (ok){
	   m_memCreatedHere=true;
	   m_size=size;
	}
	else {
		m_size=0;
	    m_memCreatedHere=false;
	}

	return ok;
}

//////////////////////////////////////////////////////////////////

bool IOBuff::AddExternalMemoryBuffer(unsigned char *pt, int size)
{
	bool ok=true;

	if (!pt)
	   ok=false;

	// get rid of any old mem
	if (ok&&m_memCreatedHere){
	   delete [] m_pt;
	   m_memCreatedHere=false;
	}

	if (ok){
	   m_pt=pt;
	   m_size=size;
	   m_pos=0;
	}

	return ok;
}

//////////////////////////////////////////////////////////////////

void IOBuff::Rewind()
{
	m_pos=0;
}

//////////////////////////////////////////////////////////////////

unsigned char IOBuff::ReadFromBuffer()
{
	bool ok=true;
	
	if (!m_pt)
	   ok=false;

	if (m_pos>=m_size)
	   return 0;

	m_pos++;
	return m_pt[m_pos-1];
}

//////////////////////////////////////////////////////////////////

unsigned char * IOBuff::ReadFromBuffer(int length)
{
	bool ok=true;

	if (!m_pt)
	   ok=false;

	if (m_pos>=m_size)
	   return NULL;

	if (length<1)
	   return NULL;

	m_pos+=length;

	return m_pt+m_pos-length;
	
}

//////////////////////////////////////////////////////////////////

bool IOBuff::WriteToBuffer(unsigned char ip)
{
	bool ok=true;
	
	if (!m_pt)
	   ok=false;


	if (m_pos>=m_size)
	   ok=false;

    if (ok){
	   m_pt[m_pos]=ip;
	   m_pos++;
	}

    return ok;
}

//////////////////////////////////////////////////////////////////

bool IOBuff::WriteToBuffer(unsigned char *ip,int length)
{
    bool ok=true;
	int i;

	if (!m_pt)
	   ok=false;

	if (m_pos+length>=m_size)
	   ok=false;

	if (ok)
	   for (i=0;i<length;i++){
		   m_pt[m_pos]=ip[i];
		   m_pos++;
	   } 
	

	return ok;
}

//////////////////////////////////////////////////////////////////

bool IOBuff::WriteToBuffer(int *ip,int length)
{
	bool ok=true;
	int i;

	if (!m_pt)
	   ok=false;

	if (m_pos+length>=m_size)
	   ok=false;

	if (ok)
	   for (i=0;i<length;i++){
	   	   m_pt[m_pos]=limit(ip[i]);
		   m_pos++;
	   }

	return ok;
}

//////////////////////////////////////////////////////////////////

bool IOBuff::ReadFromBuffer(unsigned char *ip,int length)
{
	bool ok=true;
	int i;

	if (!m_pt)
	   ok=false;

	if (m_pos+length>=m_size)
	   ok=false;

	if (ok)
	   for (i=0;i<length;i++){
	   	   ip[i]=m_pt[m_pos];
		   m_pos++;
	   }

	return ok;
}

//////////////////////////////////////////////////////////////////

bool IOBuff::WriteBufferToFile(FILE *fp)
{
	bool ok=true;
	int noBytesWritten=0;
	
	if (!m_pt)
	   ok=false;

	if (ok)
	   noBytesWritten=fwrite(m_pt,sizeof(unsigned char),m_pos,fp);
	
	if (noBytesWritten<m_pos)
	   ok=false;
	
	return ok;
}

//////////////////////////////////////////////////////////////////
//read entire file 
bool IOBuff::ReadBufferFromFile(FILE *fp)
{
	bool ok=true;
	int noBytesRead=0;

	if (!m_pt)
	   ok=false;

	if (ok)
	   noBytesRead=fread(m_pt,sizeof(unsigned char),m_size,fp);
	
	if (noBytesRead==m_size)
	   ok=false; // this may not be an error but you will be very 
				 // luck if its not ie file exactly equal to m_pt size
	// move file to end of file

	m_pos=noBytesRead;

	return ok;
}

//////////////////////////////////////////////////////////////////

bool IOBuff::WriteStringToBuffer(char *s)
{
	bool ok=true;
	int i=0;

	if (!m_pt)
	   ok=false;
	
	if (ok){
		while (s[i]!='\0'){
			if (m_pos>=m_size){
			   ok=false;	
			   break;
			}
		    m_pt[m_pos++]=(unsigned char)s[i++];
		}
		// add zero termination but do not increament 
		// storage position
		if (ok && m_pos<m_size)
		   m_pt[m_pos]='\0';

	}

	return ok;
}

//////////////////////////////////////////////////////////////////

bool IOBuff::ReadToEOLInBuffer(char *s,int length)
{
	bool ok=true;
	int i=0;

	if (!m_pt)
	   ok=false;
	
	if (ok){
		while (m_pt[m_pos]!='\n' && i<length-1){
			if (m_pos>=m_size){
			   ok=false;	
			   break;
			}
		    s[i++]=(char)m_pt[m_pos++];
		}
		// move pass \n
		if (ok)
		   m_pos++;
		// add zero termination to string
		s[i]='\0';
	}

	return ok;
}

//////////////////////////////////////////////////////////////////

bool IOBuff::ZeroTerminateBuffer()
{
	bool ok=true;

	if (!m_pt)
	   ok=false;

	if (m_pos>=m_size)
	   ok=false;

	if (ok){
	   m_pt[m_pos++]='\0';
	}

	return ok;
}

//////////////////////////////////////////////////////////////////

void IOBuff::WriteNBits(int data,int n)
{

	if (n>0)
		Write8Bits(data,(n>8 ? 8:n));
	if (n>8)
		Write8Bits((data>>8),(n>16 ? 8:n-8));
	if (n>16)	
		Write8Bits((data>>16),(n>24 ? 8:n-16));
	if (n>24)	
		Write8Bits((data>>24),(n>32 ? 8:n-24));

	return;
}

//////////////////////////////////////////////////////////////////

void IOBuff::Write8Bits(int data,int n)
{
	
	data&=0xff;	// ensure data is unsigned char size
				// it is assume that the data is clean
				// ie zero padded at top
	m_activeBytes|=(data<<m_bitLoc); // add new data
	m_bitLoc+=n; // increament position
	if (m_bitLoc > 7)
	{
		// add full byte to buffer
		m_pt[m_pos++]=(m_activeBytes & 0xff); 
		if (m_pos >= m_size)
		   EngageBufferPanic(); // panics more effectivly than
								// this function
		// freshens active bytes
		m_activeBytes=(m_activeBytes&0xff00) >> 8;
		m_bitLoc-=8; // restores bit loc
	}

	return;

}

//////////////////////////////////////////////////////////////////

void IOBuff::BitMode()
{
	m_activeBytes=0;
	m_bitLoc=0;
	return;
}

//////////////////////////////////////////////////////////////////

void IOBuff::TerminateBitWrite()
{
	m_pt[m_pos++]=(m_activeBytes&0xff);
	return;
}

//////////////////////////////////////////////////////////////////

unsigned char IOBuff::Read1Bit()
{
	if (m_bitLoc==8)
	{
		m_bitLoc=0;
		m_pos++;
	}

	return (c_mask[m_bitLoc++] & m_pt[m_pos]);
}

//////////////////////////////////////////////////////////////////

void IOBuff::EngageBufferPanic()
{
	unsigned char *tmp=NULL;
	int tmpSize,i;

	if (m_memCreatedHere)
	{
		// we are in luck and can correct panic
		tmpSize=(m_size*6)/5; // increase max buffer
		tmp=new unsigned char [tmpSize];
		if (!tmp)
		   exit(-1); // failed again, bottom.

		// copy old memory to new
		for (i=0;i<m_size;i++)
		    tmp[i]=m_pt[i];

		// throw away old memory
		delete [] m_pt;

		// save pointer and size
		m_pt=tmp;
		m_size=tmpSize;

	}
	else
	{
		// external mem, better fall over - in a really
		// bad way
		exit(-1);
	}
}

//////////////////////////////////////////////////////////////////
