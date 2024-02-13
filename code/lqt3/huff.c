//////////////////////////////////////////////////////////////////////////////////
//                        David Bethel											//
//                    Darak@compuserve.com										//
//////////////////////////////////////////////////////////////////////////////////

#include "codec.h"

//////////////////////////////////////////////////////////////////////////////////
//						Dump Functions											//
//////////////////////////////////////////////////////////////////////////////////

Dump *Dump_Create()
{
	Dump *inst;
	inst=(Dump *)malloc(sizeof(Dump));
	inst->m_packPtr=inst->m_pt;
	inst->m_activeBytes=0;
	inst->m_bits=0;
	inst->m_size=0;
	return inst;
}

//////////////////////////////////////////////////////////////////////////////////

void Dump_Destroy(Dump **inst)
{
	if (*inst)
	{
		free(*inst);
		*inst=NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////

void Dump_Rewind(Dump *inst)
{
	inst->m_packPtr=inst->m_pt;
	inst->m_activeBytes=0;
	inst->m_bits=0;
}

//////////////////////////////////////////////////////////////////////////////////

void Dump_BitMode(Dump *inst)
{
	inst->m_activeBytes=0;
	inst->m_bits=0;
}

//////////////////////////////////////////////////////////////////////////////////

void Dump_TerminateBitWrite(Dump *inst)
{
	while (inst->m_bits>0)
	{
		*inst->m_packPtr++=(inst->m_activeBytes>>24);
		inst->m_activeBytes<<=8;
		inst->m_bits-=8;	
	}
	inst->m_bits=0;
	inst->m_activeBytes=0;
	inst->m_size=inst->m_packPtr-inst->m_pt;
}

//////////////////////////////////////////////////////////////////////////////////

void Dump_TerminateBitRead(Dump *inst)
{
	if (inst->m_bits>0)
	{
		inst->m_packPtr++;
		inst->m_bits=0;
	}
	inst->m_size=inst->m_packPtr-inst->m_pt;
}

//////////////////////////////////////////////////////////////////////////////////
//						Huffman functions										//
//////////////////////////////////////////////////////////////////////////////////

Huffman *Huffman_Create()
{
	Huffman *inst;
	inst=(Huffman *)malloc(sizeof(Huffman));
	// establist quick code array
	inst->m_code=inst->m_codeSymbols+MAX_HUFFMAN_COEFF/2; 
	return inst;
}

//////////////////////////////////////////////////////////////////////////////////

void Huffman_Destroy(Huffman **inst)
{
	if (*inst)
	{
		free(*inst);
		*inst=NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////

bool Huffman_Setup(Huffman *inst,Dump *dump,unsigned int *codes,int min,int max)
{
	// code format 
	// <code 31:16><size 15:0> codes are full left shifted
	int i,j,size,nSize,fill,index;
	unsigned int code;
	unsigned short *decode;

	if (!dump)
		return false;
	if (max-min+1>=MAX_HUFFMAN_COEFF)
		return false;

	// attach uchar dump 
	inst->m_dump=dump;
	
	// setup encode
	// fill in min/maxz symbols
	for (i=0;i<MAX_HUFFMAN_COEFF/2;i++)
		inst->m_codeSymbols[i]=codes[0];
	for (;i<MAX_HUFFMAN_COEFF;i++)
		inst->m_codeSymbols[i]=codes[min+max];

	for (i=0;i<max-min+1;i++)
		inst->m_code[i+min]=codes[i];

	// setup decode
	decode=inst->m_decode;
	memset(decode,0,sizeof(short)*MAX_HUFFMAN_SIZE);
	for (i=0;i<max-min+1;i++)
	{
		code=codes[i];
		size=code & 0xffff;
		code>>=16;
		nSize=16-size;
		fill=1<<nSize;
		for (j=0;j<fill;j++)
		{
			index=(code|j) & 0xffff;
			decode[index]=((i+min)<<4) | (size-1);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////

void Huffman_CompressSymbol(Huffman *huff,int symbol)
{
	int size,shift;
	Dump *dump;
	unsigned char *packPtr;
	unsigned int activeBytes,code;
	int bits;

	// grab bits from dump
	dump=huff->m_dump;
	packPtr=dump->m_packPtr;
	activeBytes=dump->m_activeBytes;
	bits=dump->m_bits;

	code=huff->m_code[symbol];
	size=code & 0xffff;  
	code=code & 0xffff0000;
	activeBytes|=code>>bits;
	bits+=size;
	// remove finsihed data
	shift=bits&0x38; 	
	if (shift>=8)	packPtr[0]=(activeBytes>>24);
	if (shift>=16)	packPtr[1]=(activeBytes>>16) & 0xff;
	activeBytes<<=shift;
	packPtr+=bits>>3; 
	bits&=0x7;
   
	// replace
	dump->m_packPtr=packPtr;
	dump->m_activeBytes=activeBytes;
	dump->m_bits=bits;
}

//////////////////////////////////////////////////////////////////////////////////

int Huffman_UncompressSymbol(Huffman *inst)
{
	int bits,shift,index,size;
	short decode;
	Dump *dump;
	unsigned char *packPtr;
	unsigned int activeBytes;

	// grab bits from dump
	dump=inst->m_dump;
	packPtr=dump->m_packPtr;
	bits=dump->m_bits;

	// assemble data
	shift=8-bits;
	activeBytes=packPtr[2];
	activeBytes|= (packPtr[1] <<8) | (packPtr[0]<<16);
	activeBytes>>=shift;

	// find correct symbol
	index=activeBytes & 0xffff;
	decode=inst->m_decode[index];
	size=(decode & 0xf) +1;
	decode>>=4;	

	// move on
	bits+=size;
	packPtr+=bits>>3; 
	bits&=0x7;

	// replace
	dump->m_packPtr=packPtr;
	dump->m_bits=bits;

	return (int)decode;
}

//////////////////////////////////////////////////////////////////////////////////
