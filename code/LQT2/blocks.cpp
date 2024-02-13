//////////////////////////////////////////////////////////////////
//						David Bethel							//
//					 Darak@compuserve.com						//
//						LQTDCT Block coder						//
//////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "blocks.h"
#include "image.h"
#include "arithm.h"

#define AbsLimit(x) ( x < 0 ? (-x >255 ? 255:-x) : (x >255 ? 255 : x))

// class constant
const float Block::c_q[MAX_SIZE+1]=
{	
	0,0,
	Q*2,0,
	Q*4,0,0,0,
	Q*8,0,0,0,0,0,0,0,
	Q*16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	Q*32 
};

//////////////////////////////////////////////////////////////////

Block::Block()
:	m_currentFrame(NULL),
	m_lastFrame(NULL),
	m_x(0),
	m_y(0),
	m_jump(0),
	m_size(0),
	m_error(0),
	m_next(NULL),
	m_sortNext(NULL),
	m_isLastFrame(false),
	m_useCurrentFrame(true)
{


}

//////////////////////////////////////////////////////////////////

Block::~Block()
{


}

//////////////////////////////////////////////////////////////////

bool Block::SetupBlock
(int x,int y,int size,
 ImageData *current,
 ImageData *last)
{
	bool ok=true;

	if (!current)
	   ok=false;

	// setup default state
	m_isLastFrame=false;
	m_useCurrentFrame=true;

	if (ok)
	{
		m_x=x;
		m_y=y;
		m_size=size;
		m_jump=current->m_lPitch-size;
		m_currentFrame=current->m_pt+(current->m_lPitch*y)+x;
	
		if (last)
		{
			m_lastFrame=last->m_pt+(last->m_lPitch*y)+x;
			m_isLastFrame=true;
		}
	}
	
	m_useCurrentFrame=true;

	return ok;

}

//////////////////////////////////////////////////////////////////

bool Block::SetupBlock(int x,int y,int size,Block *parent)
{
	bool ok=true;
	int lPitch;

	if (!parent->m_currentFrame)
	   ok=false;

		// setup default state
	m_isLastFrame=false;
	m_useCurrentFrame=true;

	if (ok)
	{
		m_x=x;
		m_y=y;
		m_size=size;

		// figure internals out from parent
		lPitch=parent->m_jump+parent->m_size;
		m_jump=lPitch-size;
		m_currentFrame=parent->m_currentFrame+(x-parent->m_x)
					   +(y-parent->m_y)*lPitch;

		// work stuff out fo previous frame
		if (parent->m_isLastFrame)
		{
		   m_isLastFrame=true;
		   m_lastFrame=parent->m_lastFrame+(x-parent->m_x)
					   +(y-parent->m_y)*lPitch;
		}
	}
	
	return ok;

}

//////////////////////////////////////////////////////////////////

int Block::CalcLastBlockError()
{	
	
	int i,j,*ptrCurrent,*ptrLast;	
	int error=0,diff;

	ptrCurrent=m_currentFrame;
	ptrLast=m_lastFrame;

	for (j=0;j<m_size;j++,ptrLast+=m_jump,ptrCurrent+=m_jump)
		for (i=0;i<m_size;i++)
		{
			diff=AbsLimit((*ptrLast-*ptrCurrent));
			error+=mult256sq[diff];
		//	error+=diff*diff;
			ptrLast++;
			ptrCurrent++;
	    }

	return error;
}
 
//////////////////////////////////////////////////////////////////

int Block::CalcCurrentBlockError()
{
 
	int i,j,*ptr;	
	float cq;
	float quant;
	int error;
         
	// first calculate entire error 
	error=0;

	ptr=m_currentFrame;
	for (j=0;j<m_size;j++,ptr+=m_jump)
		for (i=0;i<m_size;i++)
		{
			error+=mult256sq[*ptr++];
	    }

	quant=c_q[m_size];
	for (i=0;i<MAX_COEFF;i++)
	{
		cq=quant*rint(m_c[i]/quant);
		error-=(int)rint(2*cq*m_c[i]-cq*cq);
	}     

	return myabs(error);
}

//////////////////////////////////////////////////////////////////
 
bool Block::RenderBlock()
{
	bool ok=true;

	switch (m_size)
	{
		case(2): Render2x2m4();
			     break;
		case(4): Render4x4m6();
			     break;
		case(8): Render8x8m6();
			     break;
		case(16): Render16x16m6();
			      break;
		case(32): Render32x32m6();
			      break;
		default: ok=false;
	}

	return ok;
}        

bool Block::CopyLastFrame()
{
	int *ptrLast,*ptrCurrent;
	int i,j;
	
	if (!m_lastFrame)
	   return false;

	ptrLast=m_lastFrame;
	ptrCurrent=m_currentFrame;
	
	for (j=0;j<m_size;j++,ptrLast+=m_jump,ptrCurrent+=m_jump)
		for (i=0;i<m_size;i++)
			(*ptrCurrent++)=(*ptrLast++);
	
	return true;

}
//////////////////////////////////////////////////////////////////

bool Block::CodeBlock()
{
	bool ok=true;

	switch (m_size)
	{
		case(2): Code2x2m4();
		         break;
		case(4): Code4x4m6();
		         break;
		case(8): Code8x8m6();
				 break;
		case(16): Code16x16m6();
				  break;
	    case(32): Code32x32m6();
				  break;      
		default: ok=false;
	}

	return ok;
}     
    
//////////////////////////////////////////////////////////////////
//			Quantisation  Functions						     	//
//////////////////////////////////////////////////////////////////

void Block::QuantiseBlock()
{
	int i,n;

	if (m_size==2)
	   n=4;
	else
		n=MAX_COEFF;

	for (i=0;i<n;i++)
		m_c[i]=(float)rint(m_c[i]/c_q[m_size]);
	
	return;
	
}        

//////////////////////////////////////////////////////////////////

void Block::UnquantiseBlock()
{
    int i,n;

//	if (m_size==2)
//	   n=4;
//	else
		n=MAX_COEFF;

	for (i=0;i<n;i++)
		m_c[i]*=c_q[m_size];
	
	return;
}        
//////////////////////////////////////////////////////////////////
//			Huffman block functions								//
//////////////////////////////////////////////////////////////////

int Block::HuffmanEncodeBlock(Huffman **coeff)
{
	int i,bits=0,n;

	if (m_size==2)
	   n=4;
	else
		n=MAX_COEFF;
	
	for (i=0;i<n;i++)
	{
		bits+=coeff[i]->CompressSymbol((int)(m_c[i]));
    }      
	return bits;
        
}
 
//////////////////////////////////////////////////////////////////

int Block::HuffmanEstimateBlock(Huffman **coeff)
{
	int i,bits=0,n;

 	if (m_size==2)
	   n=4;
	else
		n=MAX_COEFF;

	for (i=0;i<n;i++)
	{
		bits+=coeff[i]->EstimateCompressSymbol((int)m_c[i]);
    }      

	return bits;
 
}
 //////////////////////////////////////////////////////////////////

void Block::HuffmanDecodeBlock(Huffman **coeff)
{
	int i,n;

	if (m_size==2)
	   n=4;
	else
		n=MAX_COEFF;

 	for (i=0;i<n;i++)
	{
		m_c[i]=(float)(coeff[i]->UncompressSymbol());
	}  
 
}

//////////////////////////////////////////////////////////////////
//			Render Functions									//
//////////////////////////////////////////////////////////////////

void Block::Code2x2m4()
{
	int i,j;
	int cols[2],rows[2];
	int *ptp; /* posative moving pointer */
	int cos11;
 
    // clear memory
	for (i=0;i<2;i++)
	{
        cols[i]=0;
        rows[i]=0;
    }
 
    // setup pointer for inital sums 
    ptp=m_currentFrame;
    // sum pixels 
    for (j=0;j<2;j++,ptp+=m_jump)
        for (i=0;i<2;i++)
		{
            cols[i]+=*ptp;
            rows[j]+=(*ptp++);
		}

	// c00 coeff 
    m_c[0]=(float)0.5*(float)(cols[0]+cols[1]);
    // c01 , c10 
    m_c[1]=(float)0.5*(float)(cols[0]-cols[1]);
    m_c[2]=(float)0.5*(float)(rows[0]-rows[1]);

	ptp=m_currentFrame;
	cos11=(*ptp++);
	cos11-=(*ptp++);

	ptp+=m_jump;
	cos11-=(*ptp++);
	cos11+=*ptp;

    m_c[3]=(float)(float)0.5*(float)cos11;

}

//////////////////////////////////////////////////////////////////

void Block::Code4x4m6()
{
	int i,j,j2,jn2;
	int cols[4],rows[4];
	int *ptp,*ptn; // posative moving pointer , negative going pointer 
	int *ptp_hb,*ptn_hb; // pin,ptp + half a block offset 
	int cos00,cos01[2],cos10[2],cos11[4];

	// clear memory 
	for (i=0;i<4;i++)
	{
        cols[i]=0;
        rows[i]=0;
	}

    // setup pointer for inital sums */
    ptp=m_currentFrame;
    // sum pixels 
    for (j=0;j<4;j++,ptp+=m_jump)
        for (i=0;i<4;i++)
		{
            cols[i]+=*ptp;
            rows[j]+=(*ptp++);
		}

	// c00 coeff 
	cos00=0;
    for (i=0;i<4;i++)
        cos00+=cols[i];
    
	m_c[0]=(float)0.25*(float)cos00;

    // c01 , c10 
    for (i=0;i<2;i++)
	{
        cos10[i]=cols[i]-cols[3-i];
        cos01[i]=rows[i]-rows[3-i];
    }
    m_c[1]=(float)0.326641*(float)cos10[0]
          +(float)0.135299*(float)cos10[1];
	m_c[3]=(float)0.326641*(float)cos01[0]
          +(float)0.135299*(float)cos01[1];

    // c02 , c20 
    m_c[2]=(float)0.25*(float)(cols[0]-cols[1]-cols[2]+cols[3]);
    m_c[5]=(float)0.25*(float)(rows[0]-rows[1]-rows[2]+rows[3]);
 
    //c11 
    // posative and negative block pointers
    ptp=m_currentFrame;
    ptn=ptp+3;

    // posative and negative half block pointers
    ptp_hb=ptp+m_jump*2+8;
    ptn_hb=ptp_hb+3;
 
    for (j2=0;j2<4;j2+=2)
	{
        for (i=0;i<2;i++)
            cos11[j2+i]=(*ptp++)-(*ptn--);
        // start new lines 
        ptp+=m_jump+2;
        ptn=ptp+3;
    }
 
	for (jn2=2;jn2>=0;jn2-=2)
	{
        for (i=0;i<2;i++)
            cos11[jn2+i]+=(*ptn_hb--)-(*ptp_hb++);
        // start new lines 
        ptp_hb+=m_jump+2;
        ptn_hb=ptp_hb+3;
	}
 
    m_c[4]=(float)0.4267766*(float)cos11[0]
          +(float)0.1767766*(float)(cos11[1]+cos11[2])
          +(float)0.0673220*(float)cos11[3];
 
}

//////////////////////////////////////////////////////////////////

void Block::Code8x8m6()
{
	int i,j,j2,jn2;
	int cols[8],rows[8];
	int *ptp,*ptn; // posative moving pointer , negative going pointer 
	int *ptp_hb,*ptn_hb; // pin,ptp + half a block offset 
	int cos00,cos01[4],cos10[4],cos02[2],cos20[2],cos11[16];
 
	//clear memory 
    for (i=0;i<8;i++)
	{
        cols[i]=0;
        rows[i]=0;
    }
 
    // setup pointer for inital sums 
    ptp=m_currentFrame;
    // sum pixels
    for (j=0;j<8;j++,ptp+=m_jump)
        for (i=0;i<8;i++)
		{
            cols[i]+=*ptp;
            rows[j]+=(*ptp++);
        }

    // c00 coeff 
    cos00=0;
    for (i=0;i<8;i++)
        cos00+=cols[i];
    m_c[0]=(float)0.125*(float)cos00;

    // c01 , c10 
    for (i=0;i<4;i++)
	{
        cos10[i]=cols[i]-cols[7-i];
        cos01[i]=rows[i]-rows[7-i];
    }

    m_c[1]=(float)0.17338*(float)cos10[0]
          +(float)0.14698*(float)cos10[1]
          +(float)0.098312*(float)cos10[2]
          +(float)0.034487*(float)cos10[3];
    m_c[3]=(float)0.17338*(float)cos01[0]
          +(float)0.14698*(float)cos01[1]
          +(float)0.098312*(float)cos01[2]
          +(float)0.034487*(float)cos01[3];

    // c02 , c20 
    for (i=0;i<2;i++)
	{
        cos20[i]=cols[i]-cols[3-i]-cols[4+i]+cols[7-i];
        cos02[i]=rows[i]-rows[3-i]-rows[4+i]+rows[7-i];
    }
    m_c[2]=(float)0.16332*(float)cos20[0]
          +(float)0.06765*(float)cos20[1];

    m_c[5]=(float)0.16332*(float)cos02[0]
          +(float)0.06765*(float)cos02[1];

    // c11 
    // posative and negative block pointers
    ptp=m_currentFrame;
    ptn=ptp+7;

    // posative and negative half block pointers
    ptp_hb=ptp+m_jump*4+32;
    ptn_hb=ptp_hb+7;

    for (j2=0;j2<16;j2+=4)
	{
        for (i=0;i<4;i++)
            cos11[j2+i]=(*ptp++)-(*ptn--);
        /* start new lines */
        ptp+=m_jump+4;
        ptn=ptp+7;
    }

    for (jn2=12;jn2>=0;jn2-=4)
	{
        for (i=0;i<4;i++)
            cos11[jn2+i]+=(*ptn_hb--)-(*ptp_hb++);
        /* start new lines */
        ptp_hb+=m_jump+4;
        ptn_hb=ptp_hb+7;
	}
 
    m_c[4]=(float)0.240485*(float)cos11[0]
          +(float)0.203873*(float)(cos11[1]+cos11[4])
          +(float)0.136224*(float)(cos11[2]+cos11[8])
          +(float)0.047835*(float)(cos11[3]+cos11[12])
          +(float)0.172835*(float)cos11[5]
          +(float)0.115485*(float)(cos11[6]+cos11[9])
          +(float)0.040553*(float)(cos11[7]+cos11[13])
          +(float)0.077165*(float)cos11[10]
          +(float)0.027097*(float)(cos11[11]+cos11[14])
          +(float)0.00951506*(float)cos11[15];
}

//////////////////////////////////////////////////////////////////

void Block::Code16x16m6()
{
 
	int i,j,j2,jn2;
	int cols[16],rows[16];
	int *ptp,*ptn; // posative moving pointer , negative going pointer 
    int *ptp_hb,*ptn_hb; // pin,ptp + half a block offset 
	int cos00,cos01[8],cos10[8],cos02[4],cos20[4],cos11[64];

    // clear memory 
    for (i=0;i<16;i++)
	{
        cols[i]=0;
        rows[i]=0;
    }

    // setup pointer for inital sums 
    ptp=m_currentFrame;
    // sum pixels 
    for (j=0;j<16;j++,ptp+=m_jump)
        for (i=0;i<16;i++)
		{
            cols[i]+=*ptp;
            rows[j]+=(*ptp++);
		}

    // c00 coeff
    cos00=0;
    for (i=0;i<16;i++)
        cos00+=cols[i];
    m_c[0]=(float)0.0625*(float)cos00;

    /* c01 , c10 */
    for (i=0;i<8;i++){
        cos10[i]=cols[i]-cols[15-i];
        cos01[i]=rows[i]-rows[15-i];
    }

    m_c[1]=(float)0.087962734*(float)cos10[0]
          +(float)0.084582375*(float)cos10[1]
          +(float)0.077951563*(float)cos10[2]
          +(float)0.068325117*(float)cos10[3]
          +(float)0.056072974*(float)cos10[4]
          +(float)0.041665979*(float)cos10[5]
          +(float)0.025657783*(float)cos10[6]
          +(float)0.008663573*(float)cos10[7];

    m_c[3]=(float)0.087962734*(float)cos01[0]
          +(float)0.084582375*(float)cos01[1]
          +(float)0.077951563*(float)cos01[2]
          +(float)0.068325117*(float)cos01[3]
          +(float)0.056072974*(float)cos01[4]
          +(float)0.041665979*(float)cos01[5]
          +(float)0.025657783*(float)cos01[6]
          +(float)0.008663573*(float)cos01[7];

    // c02 , c20 
    for (i=0;i<4;i++)
	{
        cos20[i]=cols[i]-cols[7-i]-cols[8+i]+cols[15-i];
        cos02[i]=rows[i]-rows[7-i]-rows[8+i]+rows[15-i];
    }

    m_c[2]=(float)0.08669*(float)cos20[0]
          +(float)0.073492225*(float)cos20[1]
          +(float)0.049105935*(float)cos20[2]
          +(float)0.017243711*(float)cos20[3];

    m_c[5]=(float)0.08669*(float)cos02[0]
          +(float)0.073492225*(float)cos02[1]
          +(float)0.049105935*(float)cos02[2]
          +(float)0.017243711*(float)cos02[3];

    // c11 
    // posative and negative block pointers
    ptp=m_currentFrame;
    ptn=ptp+15;

    // posative and negative half block pointers
    ptp_hb=ptp+m_jump*8+128;
    ptn_hb=ptp_hb+15;

    for (j2=0;j2<64;j2+=8)
	{
        for (i=0;i<8;i++)
            cos11[j2+i]=(*ptp++)-(*ptn--);
        // start new lines 
        ptp+=m_jump+8;
        ptn=ptp+15;
    }

    for (jn2=56;jn2>=0;jn2-=8)
	{
        for (i=0;i<8;i++)
            cos11[jn2+i]+=(*ptn_hb--)-(*ptp_hb++);
        // start new lines 
        ptp_hb+=m_jump+8;
        ptn_hb=ptp_hb+15;
	}

    m_c[4]=(float)0.12379908 *(float)cos11[0]
          +(float)0.119041551*(float)(cos11[1]+cos11[8])
          +(float)0.109709322*(float)(cos11[2]+cos11[16])
          +(float)0.096161025*(float)(cos11[3]+cos11[24])
          +(float)0.078917313*(float)(cos11[4]+cos11[32])
          +(float)0.058640854*(float)(cos11[5]+cos11[40])
          +(float)0.03611086 *(float)(cos11[6]+cos11[48])
          +(float)0.012193145*(float)(cos11[7]+cos11[56])

          +(float)0.114466851*(float)cos11[9]
          +(float)0.105493254*(float)(cos11[10]+cos11[17])
          +(float)0.00924656* (float)(cos11[11]+cos11[25])
          +(float)0.075884565*(float)(cos11[12]+cos11[33])
          +(float)0.056387319*(float)(cos11[13]+cos11[41])
          +(float)0.03472314* (float)(cos11[14]+cos11[49])
          +(float)0.011724569*(float)(cos11[15]+cos11[57])

          +(float)0.09722314*(float)cos11[18]
          +(float)0.085216795*(float)(cos11[19]+cos11[26])
          +(float)0.069935616*(float)(cos11[20]+cos11[34])
          +(float)0.051966851*(float)(cos11[21]+cos11[42])
          +(float)0.032001029*(float)(cos11[22]+cos11[50])
          +(float)0.010805425*(float)(cos11[23]+cos11[58])

          +(float)0.074693145*(float)cos11[27]
          +(float)0.06129908*(float)(cos11[28]+cos11[35])
          +(float)0.045549326*(float)(cos11[29]+cos11[43])
          +(float)0.028049136*(float)(cos11[30]+cos11[51])
          +(float)0.009471034*(float)(cos11[31]+cos11[59])

          +(float)0.050306855*(float)cos11[36]
          +(float)0.037381365*(float)(cos11[37]+cos11[44])
          +(float)0.023019331*(float)(cos11[38]+cos11[52])
          +(float)0.007772677*(float)(cos11[39]+cos11[60])

          +(float)0.02777686*(float)cos11[45]
          +(float)0.017104906*(float)(cos11[46]+cos11[53])
          +(float)0.00577562*(float)(cos11[47]+cos11[61])

          +(float)0.010533149*(float)cos11[54]
          +(float)0.003556609*(float)(cos11[55]+cos11[62])
 
          +(float)0.00120092*(float)cos11[63];
		 
}

//////////////////////////////////////////////////////////////////

void Block::Code32x32m6()
{
	int i,j,j2,jn2;
    int cols[32],rows[32];
    int *ptp,*ptn; // posative moving pointer , negative going pointer
    int *ptp_hb,*ptn_hb; // pin,ptp + half a block offset 
    int cos00,cos01[16],cos10[16],cos02[8],cos20[8],cos11[256];

    // clear memory 
    for (i=0;i<32;i++)
	{
        cols[i]=0;
        rows[i]=0;
	}

    // setup pointer for inital sums 
    ptp=m_currentFrame;
    // sum pixels 
    for (j=0;j<32;j++,ptp+=m_jump)
        for (i=0;i<32;i++){
            cols[i]+=*ptp;
            rows[j]+=(*ptp++);
        }

    // c00 coeff 
    cos00=0;
    for (i=0;i<32;i++)
        cos00+=cols[i];

    m_c[0]=(float)0.03125*(float)cos00;

    // c01 , c10 
    for (i=0;i<16;i++)
	{
        cos10[i]=cols[i]-cols[31-i];
        cos01[i]=rows[i]-rows[31-i];
	}

    m_c[1]=(float)0.04414094*(float)cos10[0]
          +(float)0.04371584*(float)cos10[1]
          +(float)0.04286973*(float)cos10[2]
          +(float)0.04161076*(float)cos10[3]
          +(float)0.03995106*(float)cos10[4]
          +(float)0.03790661*(float)cos10[5]
          +(float)0.03549709*(float)cos10[6]
          +(float)0.03274572*(float)cos10[7]
          +(float)0.02967899*(float)cos10[8]
          +(float)0.02632644*(float)cos10[9]
          +(float)0.02272035*(float)cos10[10]
          +(float)0.01889544*(float)cos10[11]
          +(float)0.01488857*(float)cos10[12]
          +(float)0.01073831*(float)cos10[13]
          +(float)0.00648463*(float)cos10[14]
          +(float)0.00216851*(float)cos10[15];

	m_c[3]=(float)0.04414094*(float)cos01[0]
          +(float)0.04371584*(float)cos01[1]
          +(float)0.04286973*(float)cos01[2]
          +(float)0.04161076*(float)cos01[3]
          +(float)0.03995106*(float)cos01[4]
          +(float)0.03790661*(float)cos01[5]
          +(float)0.03549709*(float)cos01[6]
          +(float)0.03274572*(float)cos01[7]
          +(float)0.02967899*(float)cos01[8]
          +(float)0.02632644*(float)cos01[9]
          +(float)0.02272035*(float)cos01[10]
          +(float)0.01889544*(float)cos01[11]
          +(float)0.01488857*(float)cos01[12]
          +(float)0.01073831*(float)cos01[13]
          +(float)0.00648463*(float)cos01[14]
          +(float)0.00216851*(float)cos01[15];

	// c02 , c20 
    for (i=0;i<8;i++)
	{
        cos20[i]=cols[i]-cols[15-i]-cols[16+i]+cols[31-i];
        cos02[i]=rows[i]-rows[15-i]-rows[16+i]+rows[31-i];
	}

    m_c[2]=(float)0.04398137*(float)cos20[0]
           +(float)0.04229119*(float)cos20[1]
           +(float)0.03897578*(float)cos20[2]
           +(float)0.03416256*(float)cos20[3]
           +(float)0.02803649*(float)cos20[4]
           +(float)0.02083299*(float)cos20[5]
           +(float)0.01282889*(float)cos20[6]
           +(float)0.00433179*(float)cos20[7];

	m_c[5]=(float)0.04398137*(float)cos02[0]
          +(float)0.04229119*(float)cos02[1]
          +(float)0.03897578*(float)cos02[2]
          +(float)0.03416256*(float)cos02[3]
          +(float)0.02803649*(float)cos02[4]
          +(float)0.02083299*(float)cos02[5]
          +(float)0.01282889*(float)cos02[6]
          +(float)0.00433179*(float)cos02[7];

	// c11 
	// posative and negative block pointers*/
	ptp=m_currentFrame;
	ptn=ptp+31;
 
	// posative and negative half block pointers*/
    ptp_hb=ptp+m_jump*16+512;
    ptn_hb=ptp_hb+31;

    for (j2=0;j2<256;j2+=16)
	{
        for (i=0;i<16;i++)
            cos11[j2+i]=(*ptp++)-(*ptn--);
        // start new lines 
        ptp+=m_jump+16;
        ptn=ptp+31;
    }

    for (jn2=240;jn2>=0;jn2-=16)
	{
        for (i=0;i<16;i++)
            cos11[jn2+i]+=(*ptn_hb--)-(*ptp_hb++);
        // start new lines 
        ptp_hb+=m_jump+16;
        ptn_hb=ptp_hb+31;
    }

    m_c[4]=(float)0.06234952*(float)cos11[0]
          +(float)0.06174906*(float)(cos11[1]+cos11[16])
          +(float)0.06055393*(float)(cos11[2]+cos11[32])
          +(float)0.05877562*(float)(cos11[3]+cos11[48])
          +(float)0.05643127*(float)(cos11[4]+cos11[64])
          +(float)0.05354346*(float)(cos11[5]+cos11[80])
          +(float)0.05014000*(float)(cos11[6]+cos11[96])
          +(float)0.04625366*(float)(cos11[7]+cos11[112])
          +(float)0.04192188*(float)(cos11[8]+cos11[128])
          +(float)0.03718636*(float)(cos11[9]+cos11[144])
          +(float)0.03209272*(float)(cos11[10]+cos11[160])
          +(float)0.02669001*(float)(cos11[11]+cos11[176])
          +(float)0.02103025*(float)(cos11[12]+cos11[192])
          +(float)0.01516797*(float)(cos11[13]+cos11[208])
          +(float)0.00915961*(float)(cos11[14]+cos11[224])
          +(float)0.00306304*(float)(cos11[15]+cos11[240])

          +(float)0.06115439*(float)cos11[17]
          +(float)0.05997076*(float)(cos11[18]+cos11[33])
          +(float)0.05820958*(float)(cos11[19]+cos11[49])
          +(float)0.05588781*(float)(cos11[20]+cos11[65])
          +(float)0.05302781*(float)(cos11[21]+cos11[81])
          +(float)0.04965713*(float)(cos11[22]+cos11[97])
          +(float)0.04580822*(float)(cos11[23]+cos11[113])
          +(float)0.04151815*(float)(cos11[24]+cos11[129])
          +(float)0.03682823*(float)(cos11[25]+cos11[145])
          +(float)0.03178365*(float)(cos11[26]+cos11[161])
          +(float)0.02643297*(float)(cos11[27]+cos11[177])
          +(float)0.02082772*(float)(cos11[28]+cos11[193])
          +(float)0.01502189*(float)(cos11[29]+cos11[209])
          +(float)0.00907140*(float)(cos11[30]+cos11[225])
          +(float)0.00303354*(float)(cos11[31]+cos11[241])
 
          +(float)0.05881004*(float)cos11[34]
          +(float)0.05708295*(float)(cos11[35]+cos11[50])
          +(float)0.05480612*(float)(cos11[36]+cos11[66])
          +(float)0.05200147*(float)(cos11[37]+cos11[82])
          +(float)0.04869603*(float)(cos11[38]+cos11[98])
          +(float)0.04492161*(float)(cos11[39]+cos11[114])
          +(float)0.04071457*(float)(cos11[40]+cos11[130])
          +(float)0.03611543*(float)(cos11[41]+cos11[146])
          +(float)0.03116848*(float)(cos11[42]+cos11[162])
          +(float)0.02592136*(float)(cos11[43]+cos11[178])
          +(float)0.02042461*(float)(cos11[44]+cos11[194])
          +(float)0.01473115*(float)(cos11[45]+cos11[210])
          +(float)0.00889582*(float)(cos11[46]+cos11[226])
          +(float)0.00297482*(float)(cos11[47]+cos11[242])
 
          +(float)0.05540658*(float)cos11[51]
          +(float)0.05319661*(float)(cos11[52]+cos11[67])
          +(float)0.05047433*(float)(cos11[53]+cos11[83])
          +(float)0.04726596*(float)(cos11[54]+cos11[99])
          +(float)0.04360238*(float)(cos11[55]+cos11[115])
          +(float)0.03951890*(float)(cos11[56]+cos11[131])
          +(float)0.03505482*(float)(cos11[57]+cos11[147])
          +(float)0.03025315*(float)(cos11[58]+cos11[163])
          +(float)0.02516012*(float)(cos11[59]+cos11[179])
          +(float)0.01982479*(float)(cos11[60]+cos11[195])
          +(float)0.01429853*(float)(cos11[61]+cos11[211])
          +(float)0.00863458*(float)(cos11[62]+cos11[227])
          +(float)0.00288746*(float)(cos11[63]+cos11[243])
 
          +(float)0.05107479*(float)cos11[68]
          +(float)0.04846109*(float)(cos11[69]+cos11[84])
          +(float)0.04538069*(float)(cos11[70]+cos11[100])
          +(float)0.04186324*(float)(cos11[71]+cos11[116])
          +(float)0.03794263*(float)(cos11[72]+cos11[132])
          +(float)0.03365661*(float)(cos11[73]+cos11[148])
          +(float)0.02904646*(float)(cos11[74]+cos11[164])
          +(float)0.02415658*(float)(cos11[75]+cos11[180])
          +(float)0.01903405*(float)(cos11[76]+cos11[196])
          +(float)0.01372822*(float)(cos11[77]+cos11[212])
          +(float)0.00829017*(float)(cos11[78]+cos11[228])
          +(float)0.00277229*(float)(cos11[79]+cos11[244])
 
          +(float)0.04598115*(float)cos11[85]
          +(float)0.04305838*(float)(cos11[86]+cos11[101])
          +(float)0.03972094*(float)(cos11[87]+cos11[117])
          +(float)0.03600096*(float)(cos11[88]+cos11[133])
          +(float)0.03193427*(float)(cos11[89]+cos11[149])
          +(float)0.02756004*(float)(cos11[90]+cos11[165])
          +(float)0.02292039*(float)(cos11[91]+cos11[181])
          +(float)0.01806000*(float)(cos11[92]+cos11[197])
          +(float)0.01302569*(float)(cos11[93]+cos11[213])
          +(float)0.00786593*(float)(cos11[94]+cos11[229])
          +(float)0.00263042*(float)(cos11[95]+cos11[245])

          +(float)0.04032140*(float)cos11[102]
          +(float)0.03719610*(float)(cos11[103]+cos11[118])
          +(float)0.03371258*(float)(cos11[104]+cos11[134])
          +(float)0.02990439*(float)(cos11[105]+cos11[150])
          +(float)0.02580820*(float)(cos11[106]+cos11[166])
          +(float)0.02146347*(float)(cos11[107]+cos11[182])
          +(float)0.01691203*(float)(cos11[108]+cos11[198])
          +(float)0.01219772*(float)(cos11[109]+cos11[214])
          +(float)0.00736594*(float)(cos11[110]+cos11[230])
          +(float)0.00246322*(float)(cos11[111]+cos11[246])

          +(float)0.03431304*(float)cos11[119]
          +(float)0.03109952*(float)(cos11[120]+cos11[135])
          +(float)0.02758650*(float)(cos11[121]+cos11[151])
          +(float)0.02380781*(float)(cos11[122]+cos11[167])
          +(float)0.01979984*(float)(cos11[123]+cos11[183])
          +(float)0.01560118*(float)(cos11[124]+cos11[199])
          +(float)0.01125228*(float)(cos11[125]+cos11[215])
          +(float)0.00679501*(float)(cos11[126]+cos11[231])
          +(float)0.00227230*(float)(cos11[127]+cos11[247])

          +(float)0.02818696*(float)cos11[136]
          +(float)0.02500295*(float)(cos11[137]+cos11[152])
          +(float)0.02157814*(float)(cos11[138]+cos11[168])
          +(float)0.01794553*(float)(cos11[139]+cos11[184])
          +(float)0.01414009*(float)(cos11[140]+cos11[200])
          +(float)0.01019847*(float)(cos11[141]+cos11[216])
          +(float)0.00615864*(float)(cos11[142]+cos11[232])
          +(float)0.00205949*(float)(cos11[143]+cos11[248])

          +(float)0.02217860*(float)cos11[153]
          +(float)0.01914067*(float)(cos11[154]+cos11[169])
          +(float)0.01591839*(float)(cos11[155]+cos11[185])
          +(float)0.01254282*(float)(cos11[156]+cos11[201])
          +(float)0.00904645*(float)(cos11[157]+cos11[217])
          +(float)0.00546295*(float)(cos11[158]+cos11[233])
          +(float)0.00182685*(float)(cos11[159]+cos11[249])

          +(float)0.01651885*(float)cos11[170]
          +(float)0.01373795*(float)(cos11[171]+cos11[186])
          +(float)0.01082475*(float)(cos11[172]+cos11[202])
          +(float)0.00780730*(float)(cos11[173]+cos11[218])
          +(float)0.00471466*(float)(cos11[174]+cos11[234])
          +(float)0.00157661*(float)(cos11[175]+cos11[250])
 
          +(float)0.01142521*(float)cos11[187]
          +(float)0.00900244*(float)(cos11[188]+cos11[203])
          +(float)0.00649296*(float)(cos11[189]+cos11[219])
          +(float)0.00392096*(float)(cos11[190]+cos11[235])
          +(float)0.00131120*(float)(cos11[191]+cos11[251])
 
          +(float)0.00709342*(float)cos11[204]
          +(float)0.00511610*(float)(cos11[205]+cos11[220])
          +(float)0.00308950*(float)(cos11[206]+cos11[236])
          +(float)0.00103315*(float)(cos11[207]+cos11[252])
 
          +(float)0.00368996*(float)cos11[221]
          +(float)0.00222829*(float)(cos11[222]+cos11[237])
          +(float)0.00074515*(float)(cos11[223]+cos11[253])
 
          +(float)0.00134561*(float)cos11[238]
          +(float)0.00044998*(float)(cos11[239]+cos11[254])
 
          +(float)0.00015048*(float)cos11[255];
 
}

//////////////////////////////////////////////////////////////////

void Block::Render2x2m4()
{

	float g,a10,a01,a2;
	int *pix;
 
    /* return if block does not need to be rendered ie no coeff */
    if (!m_useCurrentFrame)
       return;

    /* calculate block multiplies */
    g=(float)0.5*m_c[0];
    a10=(float)0.5*m_c[1];
    a01=(float)0.5*m_c[2];
    a2=(float)0.5*m_c[3];

    /* make pointer to block easier to handle */
    pix=m_currentFrame;
    (*pix++)+=(int) (g+a01+a10+a2);
    (*pix++)+=(int) (g+a01-a10-a2);
    pix+=m_jump;
    (*pix++)+=(int) (g-a01+a10-a2);
    *pix+=    (int) (g-a01-a10+a2);

}

//////////////////////////////////////////////////////////////////

void Block::Render4x4m6()
{
    int i,j,sym,cnt,*pix;
    float cos00,cos10[4],cos20[4],cos01[4],cos02[4],cos11[16];

    // return if block does not need to be rendered ie no coeff 
    if (!m_useCurrentFrame)
       return;

    // setup coefficents 
    cos00=(float)0.25*m_c[0];

    cos10[0]=(float)0.326641*m_c[1];
    cos10[1]=(float)0.135299*m_c[1];

    cos01[0]=(float)0.326641*m_c[3];
    cos01[1]=(float)0.135299*m_c[3];

    cos20[0]=(float)0.25*m_c[2];
    cos02[0]=(float)0.25*m_c[5];
    
    cos11[0]=(float)0.4267766*m_c[4];
    cos11[1]=(float)0.1767766*m_c[4];
    cos11[4]=cos11[1];
    cos11[5]=(float)0.0673220*m_c[4];

    // symetric extention of coefficients 
    for (i=1;i<2;i++)
	{
        sym=1-i;
        cos20[i]=-cos20[sym];
        cos02[i]=-cos02[sym];
    }

    for (i=2;i<4;i++)
	{
        sym=3-i;
        cos10[i]=-cos10[sym];
        cos01[i]=-cos01[sym];

        cos20[i]=cos20[sym];
        cos02[i]=cos02[sym];
    }

    for (j=2;j<4;j++)
        for (i=0;i<2;i++)
            cos11[j*4+i]=-cos11[(3-j)*4+i];
    for (j=0;j<4;j++)
        for (i=2;i<4;i++)
            cos11[j*4+i]=-cos11[4*j+(3-i)];

    // remove common factors in additions 
    for (i=0;i<4;i++)
	{
        cos10[i]+=cos00+cos20[i];
        cos01[i]+=cos02[i];
    }

	// render information 
    pix=m_currentFrame;
    cnt=0;
    for (j=0;j<4;j++,pix+=m_jump)
        for (i=0;i<4;i++)
            (*pix++)+=(int)(cos10[i]+cos01[j]+cos11[cnt++]);

}

//////////////////////////////////////////////////////////////////

void Block::Render8x8m6()
{
    int i,j,sym,cnt,*pix;
    float cos00,cos10[8],cos20[8],cos01[8],cos02[8],cos11[64];

    // return if block does not need to be rendered ie no coeff 
    if (!m_useCurrentFrame)
       return;

    // setup coefficents 
    cos00=(float)0.125*m_c[0];

    cos10[0]=(float)0.17338*m_c[1];
    cos10[1]=(float)0.14698*m_c[1];
    cos10[2]=(float)0.098312*m_c[1];
    cos10[3]=(float)0.034487*m_c[1];

    cos01[0]=(float)0.17338*m_c[3];
    cos01[1]=(float)0.14698*m_c[3];
    cos01[2]=(float)0.098312*m_c[3];
    cos01[3]=(float)0.034487*m_c[3];

    cos20[0]=(float)0.16332*m_c[2];
    cos20[1]=(float)0.06765*m_c[2];
    cos02[0]=(float)0.16332*m_c[5];
    cos02[1]=(float)0.06765*m_c[5];
    
    cos11[0]=(float)0.240485*m_c[4];
    cos11[1]=(float)0.203873*m_c[4];
    cos11[2]=(float)0.136224*m_c[4];
    cos11[3]=(float)0.047835*m_c[4];
    cos11[9]=(float)0.172835*m_c[4];
    cos11[10]=(float)0.115485*m_c[4];
    cos11[11]=(float)0.040553*m_c[4];
    cos11[18]=(float)0.077165*m_c[4];
    cos11[19]=(float)0.027097*m_c[4];
    cos11[27]=(float)0.00951506*m_c[4];

	// reflect in xy direction 
	for (j=0;j<8;j++)
		for (i=0;i<j;i++)
			cos11[j*8+i]=cos11[i*8+j];

    // symetric extention of coefficients 
	// c20 c02 
    for (i=2;i<4;i++){
        sym=3-i;
        cos20[i]=-cos20[sym];
        cos02[i]=-cos02[sym];
    }

	// c10 c01 c20 c02 
    for (i=4;i<8;i++){
        sym=7-i;
        cos10[i]=-cos10[sym];
        cos01[i]=-cos01[sym];

        cos20[i]=cos20[sym];
        cos02[i]=cos02[sym];
    }

	// c11 
    for (j=4;j<8;j++)
        for (i=0;i<4;i++)
            cos11[j*8+i]=-cos11[(7-j)*8+i];
    for (j=0;j<8;j++)
        for (i=4;i<8;i++)
            cos11[j*8+i]=-cos11[8*j+7-i];

    // remove common factors in additions 
    for (i=0;i<8;i++){
        cos10[i]+=cos00+cos20[i];
        cos01[i]+=cos02[i];
    }

	// render information 
    pix=m_currentFrame;
    cnt=0;
    for (j=0;j<8;j++,pix+=m_jump)
        for (i=0;i<8;i++)
        (*pix++)+=(int)(cos10[i]+cos01[j]+cos11[cnt++]);
 
}

//////////////////////////////////////////////////////////////////

void Block::Render16x16m6()
{
    int i,j,sym,cnt,*pix;
    float cos00,cos10[16],cos20[16],cos01[16],cos02[16],cos11[256];

    // return if block does not need to be rendered ie no coeff 
    if (!m_useCurrentFrame)
       return;

    // setup coefficents 
    cos00=(float)0.0625*m_c[0];

    cos10[0]=(float)0.087962734*m_c[1];
    cos10[1]=(float)0.084582375*m_c[1];
    cos10[2]=(float)0.077951563*m_c[1];
    cos10[3]=(float)0.068325117*m_c[1];
    cos10[4]=(float)0.056072974*m_c[1];
    cos10[5]=(float)0.041665979*m_c[1];
    cos10[6]=(float)0.025657783*m_c[1];
    cos10[7]=(float)0.008663573*m_c[1];

    cos01[0]=(float)0.087962734*m_c[3];
    cos01[1]=(float)0.084582375*m_c[3];
    cos01[2]=(float)0.077951563*m_c[3];
    cos01[3]=(float)0.068325117*m_c[3];
    cos01[4]=(float)0.056072974*m_c[3];
    cos01[5]=(float)0.041665979*m_c[3];
    cos01[6]=(float)0.025657783*m_c[3];
    cos01[7]=(float)0.008663573*m_c[3];

    cos20[0]=(float)0.08669*m_c[2];
    cos20[1]=(float)0.073492225*m_c[2];
    cos20[2]=(float)0.049105935*m_c[2];
    cos20[3]=(float)0.017243711*m_c[2];

    cos02[0]=(float)0.08669*m_c[5];
    cos02[1]=(float)0.073492225*m_c[5];
    cos02[2]=(float)0.049105935*m_c[5];
    cos02[3]=(float)0.017243711*m_c[5];

    cos11[0]=(float)0.12379908*m_c[4];
    cos11[1]=(float)0.119041551*m_c[4];
    cos11[2]=(float)0.109709322*m_c[4];
    cos11[3]=(float)0.096161025*m_c[4];
    cos11[4]=(float)0.078917313*m_c[4];
    cos11[5]=(float)0.058640854*m_c[4];
    cos11[6]=(float)0.03611086*m_c[4];
    cos11[7]=(float)0.012193145*m_c[4];
    cos11[17]=(float)0.114466851*m_c[4];
    cos11[18]=(float)0.105493254*m_c[4];
    cos11[19]=(float)0.09246561*m_c[4];
    cos11[20]=(float)0.075884565*m_c[4];
    cos11[21]=(float)0.056387319*m_c[4];
    cos11[22]=(float)0.03472314*m_c[4];
    cos11[23]=(float)0.011724569*m_c[4];
    cos11[34]=(float)0.09722314*m_c[4];
    cos11[35]=(float)0.085216795*m_c[4];
    cos11[36]=(float)0.069935616*m_c[4];
    cos11[37]=(float)0.051966851*m_c[4];
    cos11[38]=(float)0.032001029*m_c[4];
    cos11[39]=(float)0.010805425*m_c[4];
    cos11[51]=(float)0.074693145*m_c[4];
    cos11[52]=(float)0.06129908*m_c[4];
    cos11[53]=(float)0.045549326*m_c[4];
    cos11[54]=(float)0.028049136*m_c[4];
    cos11[55]=(float)0.009471034*m_c[4];
    cos11[68]=(float)0.050306855*m_c[4];
    cos11[69]=(float)0.037381365*m_c[4];
    cos11[70]=(float)0.023019331*m_c[4];
    cos11[71]=(float)0.007772677*m_c[4];
    cos11[85]=(float)0.02777686*m_c[4];
    cos11[86]=(float)0.017104906*m_c[4];
    cos11[87]=(float)0.00577562*m_c[4];
    cos11[102]=(float)0.010533149*m_c[4];
    cos11[103]=(float)0.003556609*m_c[4];
    cos11[119]=(float)0.00120092*m_c[4];

    // reflect c11 in xy direction 
    for (j=0;j<16;j++)
        for (i=0;i<j;i++)
            cos11[(j<<4)+i]=cos11[(i<<4)+j];

    // symetric extention of coefficients 
    // c20 c02 
    for (i=4;i<8;i++)
	{
        sym=7-i;
        cos20[i]=-cos20[sym];
        cos02[i]=-cos02[sym];
    }

    // c10 c01 c20 c02 
    for (i=8;i<16;i++)
	{
        sym=15-i;
        cos10[i]=-cos10[sym];
        cos01[i]=-cos01[sym];

        cos20[i]=cos20[sym];  
        cos02[i]=cos02[sym];
    }

    // c11 
    for (j=8;j<16;j++)
        for (i=0;i<8;i++)
            cos11[(j<<4)+i]=-cos11[((15-j)<<4)+i];

    for (j=0;j<16;j++)
        for (i=8;i<16;i++)
            cos11[(j<<4)+i]=-cos11[(j<<4)+(15-i)];

    // remove common factors in additions 
    for (i=0;i<16;i++)
	{
        cos10[i]+=cos00+cos20[i];
        cos01[i]+=cos02[i];
    }

	// render information 
    pix=m_currentFrame;
    cnt=0;         
    for (j=0;j<16;j++,pix+=m_jump)
        for (i=0;i<16;i++)
        (*pix++)+=(int)(cos10[i]+cos01[j]+cos11[cnt++]);
 
}          

//////////////////////////////////////////////////////////////////                                             

void Block::Render32x32m6()
{
    int i,j,sym,cnt,*pix;
    float cos00,cos10[32],cos20[32],cos01[32],cos02[32],cos11[1024];

	if (!m_useCurrentFrame)
       return;

    cos00=(float)0.03125*m_c[0];
	
	cos10[0]=m_c[1]*(float)0.04414094;
	cos10[1]=m_c[1]*(float)0.04371584;
	cos10[2]=m_c[1]*(float)0.04286973;
	cos10[3]=m_c[1]*(float)0.04161076;
	cos10[4]=m_c[1]*(float)0.03995106;
	cos10[5]=m_c[1]*(float)0.03790661;
	cos10[6]=m_c[1]*(float)0.03549709;
	cos10[7]=m_c[1]*(float)0.03274572;
	cos10[8]=m_c[1]*(float)0.02967899;
	cos10[9]=m_c[1]*(float)0.02632644;
	cos10[10]=m_c[1]*(float)0.02272035;
	cos10[11]=m_c[1]*(float)0.01889544;
	cos10[12]=m_c[1]*(float)0.01488857;
	cos10[13]=m_c[1]*(float)0.01073831;
	cos10[14]=m_c[1]*(float)0.00648463;
	cos10[15]=m_c[1]*(float)0.00216851;

	cos01[0]=m_c[3]*(float)0.04414094;
	cos01[1]=m_c[3]*(float)0.04371584;
	cos01[2]=m_c[3]*(float)0.04286973;
	cos01[3]=m_c[3]*(float)0.04161076;
	cos01[4]=m_c[3]*(float)0.03995106;
	cos01[5]=m_c[3]*(float)0.03790661;
	cos01[6]=m_c[3]*(float)0.03549709;
	cos01[7]=m_c[3]*(float)0.03274572;
	cos01[8]=m_c[3]*(float)0.02967899;
	cos01[9]=m_c[3]*(float)0.02632644;
	cos01[10]=m_c[3]*(float)0.02272035;
	cos01[11]=m_c[3]*(float)0.01889544;
	cos01[12]=m_c[3]*(float)0.01488857;
	cos01[13]=m_c[3]*(float)0.01073831;
	cos01[14]=m_c[3]*(float)0.00648463;
	cos01[15]=m_c[3]*(float)0.00216851;

	cos20[0]=m_c[2]*(float)0.04398137;
	cos20[1]=m_c[2]*(float)0.04229119;
	cos20[2]=m_c[2]*(float)0.03897578;
	cos20[3]=m_c[2]*(float)0.03416256;
	cos20[4]=m_c[2]*(float)0.02803649;
	cos20[5]=m_c[2]*(float)0.02083299;
	cos20[6]=m_c[2]*(float)0.01282889;
	cos20[7]=m_c[2]*(float)0.00433179;

	cos02[0]=m_c[5]*(float)0.04398137;
	cos02[1]=m_c[5]*(float)0.04229119;
	cos02[2]=m_c[5]*(float)0.03897578;
	cos02[3]=m_c[5]*(float)0.03416256;
	cos02[4]=m_c[5]*(float)0.02803649;
	cos02[5]=m_c[5]*(float)0.02083299;
	cos02[6]=m_c[5]*(float)0.01282889;
	cos02[7]=m_c[5]*(float)0.00433179;

	cos11[0]=m_c[4]*(float)0.06234952;
	cos11[1]=m_c[4]*(float)0.06174906;
	cos11[2]=m_c[4]*(float)0.06055393;
	cos11[3]=m_c[4]*(float)0.05877562;
	cos11[4]=m_c[4]*(float)0.05643127;
	cos11[5]=m_c[4]*(float)0.05354346;
	cos11[6]=m_c[4]*(float)0.05014000;
	cos11[7]=m_c[4]*(float)0.04625366;
	cos11[8]=m_c[4]*(float)0.04192188;
	cos11[9]=m_c[4]*(float)0.03718636;
	cos11[10]=m_c[4]*(float)0.03209272;
	cos11[11]=m_c[4]*(float)0.02669001;
	cos11[12]=m_c[4]*(float)0.02103025;
	cos11[13]=m_c[4]*(float)0.01516797;
	cos11[14]=m_c[4]*(float)0.00915961;
	cos11[15]=m_c[4]*(float)0.00306304;
	cos11[33]=m_c[4]*(float)0.06115439;
	cos11[34]=m_c[4]*(float)0.05997076;
	cos11[35]=m_c[4]*(float)0.05820958;
	cos11[36]=m_c[4]*(float)0.05588781;
	cos11[37]=m_c[4]*(float)0.05302781;
	cos11[38]=m_c[4]*(float)0.04965713;
	cos11[39]=m_c[4]*(float)0.04580822;
	cos11[40]=m_c[4]*(float)0.04151815;
	cos11[41]=m_c[4]*(float)0.03682823;
	cos11[42]=m_c[4]*(float)0.03178365;
	cos11[43]=m_c[4]*(float)0.02643297;
	cos11[44]=m_c[4]*(float)0.02082772;
	cos11[45]=m_c[4]*(float)0.01502189;
	cos11[46]=m_c[4]*(float)0.00907140;
	cos11[47]=m_c[4]*(float)0.00303354;
	cos11[66]=m_c[4]*(float)0.05881004;
	cos11[67]=m_c[4]*(float)0.05708295;
	cos11[68]=m_c[4]*(float)0.05480612;
	cos11[69]=m_c[4]*(float)0.05200147;
	cos11[70]=m_c[4]*(float)0.04869603;
	cos11[71]=m_c[4]*(float)0.04492161;
	cos11[72]=m_c[4]*(float)0.04071457;
	cos11[73]=m_c[4]*(float)0.03611543;
	cos11[74]=m_c[4]*(float)0.03116848;
	cos11[75]=m_c[4]*(float)0.02592136;
	cos11[76]=m_c[4]*(float)0.02042461;
	cos11[77]=m_c[4]*(float)0.01473115;
	cos11[78]=m_c[4]*(float)0.00889582;
	cos11[79]=m_c[4]*(float)0.00297482;
	cos11[99]=m_c[4]*(float)0.05540658;
	cos11[100]=m_c[4]*(float)0.05319661;
	cos11[101]=m_c[4]*(float)0.05047433;
	cos11[102]=m_c[4]*(float)0.04726596;
	cos11[103]=m_c[4]*(float)0.04360238;
	cos11[104]=m_c[4]*(float)0.03951890;
	cos11[105]=m_c[4]*(float)0.03505482;
	cos11[106]=m_c[4]*(float)0.03025315;
	cos11[107]=m_c[4]*(float)0.02516012;
	cos11[108]=m_c[4]*(float)0.01982479;
	cos11[109]=m_c[4]*(float)0.01429853;
	cos11[110]=m_c[4]*(float)0.00863458;
	cos11[111]=m_c[4]*(float)0.00288746;
	cos11[132]=m_c[4]*(float)0.05107479;
	cos11[133]=m_c[4]*(float)0.04846109;
	cos11[134]=m_c[4]*(float)0.04538069;
	cos11[135]=m_c[4]*(float)0.04186324;
	cos11[136]=m_c[4]*(float)0.03794263;
	cos11[137]=m_c[4]*(float)0.03365661;
	cos11[138]=m_c[4]*(float)0.02904646;
	cos11[139]=m_c[4]*(float)0.02415658;
	cos11[140]=m_c[4]*(float)0.01903405;
	cos11[141]=m_c[4]*(float)0.01372822;
	cos11[142]=m_c[4]*(float)0.00829017;
	cos11[143]=m_c[4]*(float)0.00277229;
	cos11[165]=m_c[4]*(float)0.04598115;
	cos11[166]=m_c[4]*(float)0.04305838;
	cos11[167]=m_c[4]*(float)0.03972094;
	cos11[168]=m_c[4]*(float)0.03600096;
	cos11[169]=m_c[4]*(float)0.03193427;
	cos11[170]=m_c[4]*(float)0.02756004;
	cos11[171]=m_c[4]*(float)0.02292039;
	cos11[172]=m_c[4]*(float)0.01806000;
	cos11[173]=m_c[4]*(float)0.01302569;
	cos11[174]=m_c[4]*(float)0.00786593;
	cos11[175]=m_c[4]*(float)0.00263042;
	cos11[198]=m_c[4]*(float)0.04032140;
	cos11[199]=m_c[4]*(float)0.03719610;
	cos11[200]=m_c[4]*(float)0.03371258;
	cos11[201]=m_c[4]*(float)0.02990439;
	cos11[202]=m_c[4]*(float)0.02580820;
	cos11[203]=m_c[4]*(float)0.02146347;
	cos11[204]=m_c[4]*(float)0.01691203;
	cos11[205]=m_c[4]*(float)0.01219772;
	cos11[206]=m_c[4]*(float)0.00736594;
	cos11[207]=m_c[4]*(float)0.00246322;
	cos11[231]=m_c[4]*(float)0.03431304;
	cos11[232]=m_c[4]*(float)0.03109952;
	cos11[233]=m_c[4]*(float)0.02758650;
	cos11[234]=m_c[4]*(float)0.02380781;
	cos11[235]=m_c[4]*(float)0.01979984;
	cos11[236]=m_c[4]*(float)0.01560118;
	cos11[237]=m_c[4]*(float)0.01125228;
	cos11[238]=m_c[4]*(float)0.00679501;
	cos11[239]=m_c[4]*(float)0.00227230;
	cos11[264]=m_c[4]*(float)0.02818696;
	cos11[265]=m_c[4]*(float)0.02500295;
	cos11[266]=m_c[4]*(float)0.02157814;
	cos11[267]=m_c[4]*(float)0.01794553;
	cos11[268]=m_c[4]*(float)0.01414009;
	cos11[269]=m_c[4]*(float)0.01019847;
	cos11[270]=m_c[4]*(float)0.00615864;
	cos11[271]=m_c[4]*(float)0.00205949;
	cos11[297]=m_c[4]*(float)0.02217860;
	cos11[298]=m_c[4]*(float)0.01914067;
	cos11[299]=m_c[4]*(float)0.01591839;
	cos11[300]=m_c[4]*(float)0.01254282;
	cos11[301]=m_c[4]*(float)0.00904645;
	cos11[302]=m_c[4]*(float)0.00546295;
	cos11[303]=m_c[4]*(float)0.00182685;
	cos11[330]=m_c[4]*(float)0.01651885;
	cos11[331]=m_c[4]*(float)0.01373795;
	cos11[332]=m_c[4]*(float)0.01082475;
	cos11[333]=m_c[4]*(float)0.00780730;
	cos11[334]=m_c[4]*(float)0.00471466;
	cos11[335]=m_c[4]*(float)0.00157661;
	cos11[363]=m_c[4]*(float)0.01142521;
	cos11[364]=m_c[4]*(float)0.00900244;
	cos11[365]=m_c[4]*(float)0.00649296;
	cos11[366]=m_c[4]*(float)0.00392096;
	cos11[367]=m_c[4]*(float)0.00131120;
	cos11[396]=m_c[4]*(float)0.00709342;
	cos11[397]=m_c[4]*(float)0.00511610;
	cos11[398]=m_c[4]*(float)0.00308950;
	cos11[399]=m_c[4]*(float)0.00103315;
	cos11[429]=m_c[4]*(float)0.00368996;
	cos11[430]=m_c[4]*(float)0.00222829;
	cos11[431]=m_c[4]*(float)0.00074515;
	cos11[462]=m_c[4]*(float)0.00134561;
	cos11[463]=m_c[4]*(float)0.00044998;
	cos11[495]=m_c[4]*(float)0.00015048;

	// reflect c11 in xy direction 
    for (j=0;j<32;j++)
        for (i=0;i<j;i++)
            cos11[32*j+i]=cos11[32*i+j];

    for (i=8;i<16;i++)
	{
        sym=15-i;
        cos20[i]=-cos20[sym];
        cos02[i]=-cos02[sym];
    }

    for (i=16;i<32;i++)
	{
        sym=31-i;
        cos10[i]=-cos10[sym];
        cos01[i]=-cos01[sym];

        cos20[i]=cos20[sym];
        cos02[i]=cos02[sym];
    }

    for (j=16;j<32;j++)
        for (i=0;i<16;i++)
            cos11[j*32+i]=-cos11[(31-j)*32+i];
    for (j=0;j<32;j++)
        for (i=16;i<32;i++)
            cos11[j*32+i]=-cos11[32*j+(31-i)];

    for (i=0;i<32;i++)
	{
        cos10[i]+=cos00+cos20[i];
        cos01[i]+=cos02[i];
    }

    pix=m_currentFrame;
    cnt=0;
    for (j=0;j<32;j++,pix+=m_jump)
        for (i=0;i<32;i++)
            (*pix++)+=(int)(cos10[i]+cos01[j]+cos11[cnt++]);
 
}