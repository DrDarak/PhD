#ifndef IOBUFF_H
#define IOBUFF_H

#include <stdio.h>
 
class IOBuff {

public:
	IOBuff();
	IOBuff(unsigned char *pt,int size);
	~IOBuff();

	unsigned char *m_pt;
	int m_size;
	int m_pos;
	int m_activeBytes;
	int m_bitLoc;
	static const unsigned char c_mask[8];

	//setup
	void Rewind();	
	void BitMode();
	void TerminateBitWrite();

	// creates memory
	bool CreateMemoryBuffer(int size,bool force=0); 
	bool AddExternalMemoryBuffer(unsigned char *pt, int size);

	/* ouput from buffer */
	unsigned char ReadFromBuffer();
	unsigned char *ReadFromBuffer(int length);
	bool ReadFromBuffer(unsigned char *ip,int length);

	/* input from buffer */
	bool WriteToBuffer(unsigned char);
	bool WriteToBuffer(unsigned char *,int length);
	bool WriteToBuffer(int *ip,int length);

	// string manipulation 
	bool WriteStringToBuffer(char *s);
	bool ReadToEOLInBuffer(char *s,int length);

	bool ZeroTerminateBuffer();

	bool WriteBufferToFile(FILE *fp);
	bool ReadBufferFromFile(FILE *fp);

	// bit operations - only in bitmode
	void WriteNBits(int data,int n);
	unsigned char Read1Bit();

private:
	void EngageBufferPanic();
	void Write8Bits(int data,int n);
	bool m_memCreatedHere; // Marks if memory has been formed here

};

#endif