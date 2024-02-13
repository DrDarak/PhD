/*##############################################################*/
/*                      David Bethel                            */
/*                     Bath University                          */
/*                    daveb@ee.bath.ac.uk                       */
/*                    Huffman encoder                           */
/*                      reworked 29/8/96                        */
/*##############################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "mem_hand.h"
#include "render.h"

#define N 4
void codeNxNm6
(BLOCK *block)
{
	int i,j,j2,jn2;
	int cols[N],rows[N];
	int *ptp,*ptn; /* posative moving pointer , negative going pointer */
	int *ptp_hb,*ptn_hb; /* pin,ptp + half a block offset */
	int cos00,cos01[N/2],cos10[N/2],cos02[N/4],cos20[N/4],cos11[(N*N)/4];

	/* clear memory */
	for (i=0;i<N;i++){
	    cols[i]=0;
	    rows[i]=0; 
	}

	/* setup pointer for inital sums */
	ptp=block->pt;
	/* sum pixels */
	for (j=0;j<N;j++,ptp+=block->jump)
	    for (i=0;i<N;i++){
		cols[i]+=*ptp;
	        rows[j]+=(*ptp++);
	    }

	/* c00 coeff */
	cos00=0;
	for (i=0;i<N;i++)
	    cos00+=cols[i];
	block->c[0]=0.03125*(float)cos00;

	/* c01 , c10 */
	for (i=0;i<N/2;i++){
	    cos10[i]=cols[i]-cols[N-i-1];
	    cos01[i]=rows[i]-rows[N-i-1];
	}

        block->c[1]=0;

	block->c[3]=0;

	/* c02 , c20 */
	for (i=0;i<N/4;i++){
	    cos20[i]=cols[i]-cols[N/2-1-i]-cols[N/2+i]+cols[N-1-i];
	    cos02[i]=rows[i]-rows[N/2-1-i]-rows[N/2+i]+rows[N-1-i];
	}

	block->c[2]=0;

	block->c[5]=0;

	/* c11 */
	/* posative and negative block pointers*/
	ptp=block->pt;
	ptn=ptp+N-1;

	/* posative and negative half block pointers*/
	ptp_hb=ptp+(block->jump+block->size)*N/2;
	ptn_hb=ptp_hb+N-1;

	for (j2=0;j2<N*N/4;j2+=N/2){
	    for (i=0;i<N/2;i++)
		cos11[j2+i]=(*ptp++)-(*ptn--);
	    /* start new lines */
	    ptp+=block->jump+N/2;
	    ptn=ptp+N-1;
	}

	for (jn2=(N*N/4)-N/2;jn2>=0;jn2-=N/2){
	    for (i=0;i<N/2;i++)
		cos11[jn2+i]+=(*ptn_hb--)-(*ptp_hb++);
	    /* start new lines */
	    ptp_hb+=block->jump+N/2;
	    ptn_hb=ptp_hb+N-1;
	}

	block->c[4]=0;

}

void code2x2m4
(BLOCK *block)
{
        int i,j;
        int cols[2],rows[2];
        int *ptp; /* posative moving pointer */
	int cos11;
 
        /* clear memory */
        for (i=0;i<2;i++){
            cols[i]=0;
            rows[i]=0;
        }
 
        /* setup pointer for inital sums */
        ptp=block->pt;
        /* sum pixels */
        for (j=0;j<2;j++,ptp+=block->jump)
            for (i=0;i<2;i++){
                cols[i]+=*ptp;
                rows[j]+=(*ptp++);
            }

        /* c00 coeff */
        block->c[0]=0.5*(float)(cols[0]+cols[1]);
        /* c01 , c10 */
        block->c[1]=0.5*(float)(cols[0]-cols[1]);
        block->c[2]=0.5*(float)(rows[0]-rows[1]);

        ptp=block->pt;
	cos11=(*ptp++);
	cos11-=(*ptp++);

	ptp+=block->jump;
	cos11-=(*ptp++);
	cos11+=*ptp;

        block->c[3]=0.5*(float)cos11;

}


void code32x32m6
(BLOCK *block)
{
        int i,j,j2,jn2;
        int cols[32],rows[32];
        int *ptp,*ptn; /* posative moving pointer , negative going pointer */
        int *ptp_hb,*ptn_hb; /* pin,ptp + half a block offset */
        int cos00,cos01[16],cos10[16],cos02[8],cos20[8],cos11[256];

        /* clear memory */
        for (i=0;i<32;i++){
            cols[i]=0;
            rows[i]=0;
        }

        /* setup pointer for inital sums */
        ptp=block->pt;
        /* sum pixels */
        for (j=0;j<32;j++,ptp+=block->jump)
            for (i=0;i<32;i++){
                cols[i]+=*ptp;
                rows[j]+=(*ptp++);
            }

        /* c00 coeff */
        cos00=0;
        for (i=0;i<32;i++)
            cos00+=cols[i];
        block->c[0]=0.03125*(float)cos00;

        /* c01 , c10 */
        for (i=0;i<16;i++){
            cos10[i]=cols[i]-cols[31-i];
            cos01[i]=rows[i]-rows[31-i];
        }

        block->c[1]=0.04414094*(float)cos10[0]
                   +0.04371584*(float)cos10[1]
                   +0.04286973*(float)cos10[2]
                   +0.04161076*(float)cos10[3]
                   +0.03995106*(float)cos10[4]
                   +0.03790661*(float)cos10[5]
                   +0.03549709*(float)cos10[6]
                   +0.03274572*(float)cos10[7]
                   +0.02967899*(float)cos10[8]
                   +0.02632644*(float)cos10[9]
                   +0.02272035*(float)cos10[10]
                   +0.01889544*(float)cos10[11]
                   +0.01488857*(float)cos10[12]
                   +0.01073831*(float)cos10[13]
                   +0.00648463*(float)cos10[14]
                   +0.00216851*(float)cos10[15];

        block->c[3]=0.04414094*(float)cos01[0]
                   +0.04371584*(float)cos01[1]
                   +0.04286973*(float)cos01[2]
                   +0.04161076*(float)cos01[3]
                   +0.03995106*(float)cos01[4]
                   +0.03790661*(float)cos01[5]
                   +0.03549709*(float)cos01[6]
                   +0.03274572*(float)cos01[7]
                   +0.02967899*(float)cos01[8]
                   +0.02632644*(float)cos01[9]
                   +0.02272035*(float)cos01[10]
                   +0.01889544*(float)cos01[11]
                   +0.01488857*(float)cos01[12]
                   +0.01073831*(float)cos01[13]
                   +0.00648463*(float)cos01[14]
                   +0.00216851*(float)cos01[15];

        /* c02 , c20 */
        for (i=0;i<8;i++){
            cos20[i]=cols[i]-cols[15-i]-cols[16+i]+cols[31-i];
            cos02[i]=rows[i]-rows[15-i]-rows[16+i]+rows[31-i];
        }

        block->c[2]=0.04398137*(float)cos20[0]
                   +0.04229119*(float)cos20[1]
                   +0.03897578*(float)cos20[2]
                   +0.03416256*(float)cos20[3]
                   +0.02803649*(float)cos20[4]
                   +0.02083299*(float)cos20[5]
                   +0.01282889*(float)cos20[6]
                   +0.00433179*(float)cos20[7];

        block->c[5]=0.04398137*(float)cos02[0]
                   +0.04229119*(float)cos02[1]
                   +0.03897578*(float)cos02[2]
                   +0.03416256*(float)cos02[3]
                   +0.02803649*(float)cos02[4]
                   +0.02083299*(float)cos02[5]
                   +0.01282889*(float)cos02[6]
                   +0.00433179*(float)cos02[7];

	 /* c11 */
        /* posative and negative block pointers*/
        ptp=block->pt;
        ptn=ptp+31;
 
        /* posative and negative half block pointers*/
        ptp_hb=ptp+block->jump*16+512;
        ptn_hb=ptp_hb+31;

        for (j2=0;j2<256;j2+=16){
            for (i=0;i<16;i++)
                cos11[j2+i]=(*ptp++)-(*ptn--);
            /* start new lines */
            ptp+=block->jump+16;
            ptn=ptp+31;
        }

        for (jn2=240;jn2>=0;jn2-=16){
            for (i=0;i<16;i++)
                cos11[jn2+i]+=(*ptn_hb--)-(*ptp_hb++);
            /* start new lines */
            ptp_hb+=block->jump+16;
            ptn_hb=ptp_hb+31;
        }

        block->c[4]=0.06234952*(float)cos11[0]
                   +0.06174906*(float)(cos11[1]+cos11[16])
                   +0.06055393*(float)(cos11[2]+cos11[32])
                   +0.05877562*(float)(cos11[3]+cos11[48])
                   +0.05643127*(float)(cos11[4]+cos11[64])
                   +0.05354346*(float)(cos11[5]+cos11[80])
                   +0.05014000*(float)(cos11[6]+cos11[96])
                   +0.04625366*(float)(cos11[7]+cos11[112])
                   +0.04192188*(float)(cos11[8]+cos11[128])
                   +0.03718636*(float)(cos11[9]+cos11[144])
                   +0.03209272*(float)(cos11[10]+cos11[160])
                   +0.02669001*(float)(cos11[11]+cos11[176])
                   +0.02103025*(float)(cos11[12]+cos11[192])
                   +0.01516797*(float)(cos11[13]+cos11[208])
                   +0.00915961*(float)(cos11[14]+cos11[224])
                   +0.00306304*(float)(cos11[15]+cos11[240])

                   +0.06115439*(float)cos11[17]
                   +0.05997076*(float)(cos11[18]+cos11[33])
                   +0.05820958*(float)(cos11[19]+cos11[49])
                   +0.05588781*(float)(cos11[20]+cos11[65])
                   +0.05302781*(float)(cos11[21]+cos11[81])
                   +0.04965713*(float)(cos11[22]+cos11[97])
                   +0.04580822*(float)(cos11[23]+cos11[113])
                   +0.04151815*(float)(cos11[24]+cos11[129])
                   +0.03682823*(float)(cos11[25]+cos11[145])
                   +0.03178365*(float)(cos11[26]+cos11[161])
                   +0.02643297*(float)(cos11[27]+cos11[177])
                   +0.02082772*(float)(cos11[28]+cos11[193])
                   +0.01502189*(float)(cos11[29]+cos11[209])
                   +0.00907140*(float)(cos11[30]+cos11[225])
                   +0.00303354*(float)(cos11[31]+cos11[241])
 
                   +0.05881004*(float)cos11[34]
                   +0.05708295*(float)(cos11[35]+cos11[50])
                   +0.05480612*(float)(cos11[36]+cos11[66])
                   +0.05200147*(float)(cos11[37]+cos11[82])
                   +0.04869603*(float)(cos11[38]+cos11[98])
                   +0.04492161*(float)(cos11[39]+cos11[114])
                   +0.04071457*(float)(cos11[40]+cos11[130])
                   +0.03611543*(float)(cos11[41]+cos11[146])
                   +0.03116848*(float)(cos11[42]+cos11[162])
                   +0.02592136*(float)(cos11[43]+cos11[178])
                   +0.02042461*(float)(cos11[44]+cos11[194])
                   +0.01473115*(float)(cos11[45]+cos11[210])
                   +0.00889582*(float)(cos11[46]+cos11[226])
                   +0.00297482*(float)(cos11[47]+cos11[242])
 
                   +0.05540658*(float)cos11[51]
                   +0.05319661*(float)(cos11[52]+cos11[67])
                   +0.05047433*(float)(cos11[53]+cos11[83])
                   +0.04726596*(float)(cos11[54]+cos11[99])
                   +0.04360238*(float)(cos11[55]+cos11[115])
                   +0.03951890*(float)(cos11[56]+cos11[131])
                   +0.03505482*(float)(cos11[57]+cos11[147])
                   +0.03025315*(float)(cos11[58]+cos11[163])
                   +0.02516012*(float)(cos11[59]+cos11[179])
                   +0.01982479*(float)(cos11[60]+cos11[195])
                   +0.01429853*(float)(cos11[61]+cos11[211])
                   +0.00863458*(float)(cos11[62]+cos11[227])
                   +0.00288746*(float)(cos11[63]+cos11[243])
 
                   +0.05107479*(float)cos11[68]
                   +0.04846109*(float)(cos11[69]+cos11[84])
                   +0.04538069*(float)(cos11[70]+cos11[100])
                   +0.04186324*(float)(cos11[71]+cos11[116])
                   +0.03794263*(float)(cos11[72]+cos11[132])
                   +0.03365661*(float)(cos11[73]+cos11[148])
                   +0.02904646*(float)(cos11[74]+cos11[164])
                   +0.02415658*(float)(cos11[75]+cos11[180])
                   +0.01903405*(float)(cos11[76]+cos11[196])
                   +0.01372822*(float)(cos11[77]+cos11[212])
                   +0.00829017*(float)(cos11[78]+cos11[228])
                   +0.00277229*(float)(cos11[79]+cos11[244])
 
                   +0.04598115*(float)cos11[85]
                   +0.04305838*(float)(cos11[86]+cos11[101])
                   +0.03972094*(float)(cos11[87]+cos11[117])
                   +0.03600096*(float)(cos11[88]+cos11[133])
                   +0.03193427*(float)(cos11[89]+cos11[149])
                   +0.02756004*(float)(cos11[90]+cos11[165])
                   +0.02292039*(float)(cos11[91]+cos11[181])
                   +0.01806000*(float)(cos11[92]+cos11[197])
                   +0.01302569*(float)(cos11[93]+cos11[213])
                   +0.00786593*(float)(cos11[94]+cos11[229])
                   +0.00263042*(float)(cos11[95]+cos11[245])

                   +0.04032140*(float)cos11[102]
                   +0.03719610*(float)(cos11[103]+cos11[118])
                   +0.03371258*(float)(cos11[104]+cos11[134])
                   +0.02990439*(float)(cos11[105]+cos11[150])
                   +0.02580820*(float)(cos11[106]+cos11[166])
                   +0.02146347*(float)(cos11[107]+cos11[182])
                   +0.01691203*(float)(cos11[108]+cos11[198])
                   +0.01219772*(float)(cos11[109]+cos11[214])
                   +0.00736594*(float)(cos11[110]+cos11[230])
                   +0.00246322*(float)(cos11[111]+cos11[246])

                   +0.03431304*(float)cos11[119]
                   +0.03109952*(float)(cos11[120]+cos11[135])
                   +0.02758650*(float)(cos11[121]+cos11[151])
                   +0.02380781*(float)(cos11[122]+cos11[167])
                   +0.01979984*(float)(cos11[123]+cos11[183])
                   +0.01560118*(float)(cos11[124]+cos11[199])
                   +0.01125228*(float)(cos11[125]+cos11[215])
                   +0.00679501*(float)(cos11[126]+cos11[231])
                   +0.00227230*(float)(cos11[127]+cos11[247])

                   +0.02818696*(float)cos11[136]
                   +0.02500295*(float)(cos11[137]+cos11[152])
                   +0.02157814*(float)(cos11[138]+cos11[168])
                   +0.01794553*(float)(cos11[139]+cos11[184])
                   +0.01414009*(float)(cos11[140]+cos11[200])
                   +0.01019847*(float)(cos11[141]+cos11[216])
                   +0.00615864*(float)(cos11[142]+cos11[232])
                   +0.00205949*(float)(cos11[143]+cos11[248])

                   +0.02217860*(float)cos11[153]
                   +0.01914067*(float)(cos11[154]+cos11[169])
                   +0.01591839*(float)(cos11[155]+cos11[185])
                   +0.01254282*(float)(cos11[156]+cos11[201])
                   +0.00904645*(float)(cos11[157]+cos11[217])
                   +0.00546295*(float)(cos11[158]+cos11[233])
                   +0.00182685*(float)(cos11[159]+cos11[249])

                   +0.01651885*(float)cos11[170]
                   +0.01373795*(float)(cos11[171]+cos11[186])
                   +0.01082475*(float)(cos11[172]+cos11[202])
                   +0.00780730*(float)(cos11[173]+cos11[218])
                   +0.00471466*(float)(cos11[174]+cos11[234])
                   +0.00157661*(float)(cos11[175]+cos11[250])
 
                   +0.01142521*(float)cos11[187]
                   +0.00900244*(float)(cos11[188]+cos11[203])
                   +0.00649296*(float)(cos11[189]+cos11[219])
                   +0.00392096*(float)(cos11[190]+cos11[235])
                   +0.00131120*(float)(cos11[191]+cos11[251])
 
                   +0.00709342*(float)cos11[204]
                   +0.00511610*(float)(cos11[205]+cos11[220])
                   +0.00308950*(float)(cos11[206]+cos11[236])
                   +0.00103315*(float)(cos11[207]+cos11[252])
 
                   +0.00368996*(float)cos11[221]
                   +0.00222829*(float)(cos11[222]+cos11[237])
                   +0.00074515*(float)(cos11[223]+cos11[253])
 
                   +0.00134561*(float)cos11[238]
                   +0.00044998*(float)(cos11[239]+cos11[254])
 
                   +0.00015048*(float)cos11[255];
 
}

void code16x16m6
(BLOCK *block)
{
        int i,j,j2,jn2;
        int cols[16],rows[16];
        int *ptp,*ptn; /* posative moving pointer , negative going pointer */
        int *ptp_hb,*ptn_hb; /* pin,ptp + half a block offset */
        int cos00,cos01[8],cos10[8],cos02[4],cos20[4],cos11[64];

        /* clear memory */
        for (i=0;i<16;i++){
            cols[i]=0;
            rows[i]=0;
        }

        /* setup pointer for inital sums */
        ptp=block->pt;
        /* sum pixels */
        for (j=0;j<16;j++,ptp+=block->jump)
            for (i=0;i<16;i++){
                cols[i]+=*ptp;
                rows[j]+=(*ptp++);
            }

        /* c00 coeff */
        cos00=0;
        for (i=0;i<16;i++)
            cos00+=cols[i];
        block->c[0]=0.0625*(float)cos00;

        /* c01 , c10 */
        for (i=0;i<8;i++){
            cos10[i]=cols[i]-cols[15-i];
            cos01[i]=rows[i]-rows[15-i];
        }

        block->c[1]=0.087962734*(float)cos10[0]
                   +0.084582375*(float)cos10[1]
                   +0.077951563*(float)cos10[2]
                   +0.068325117*(float)cos10[3]
                   +0.056072974*(float)cos10[4]
                   +0.041665979*(float)cos10[5]
                   +0.025657783*(float)cos10[6]
                   +0.008663573*(float)cos10[7];

        block->c[3]=0.087962734*(float)cos01[0]
                   +0.084582375*(float)cos01[1]
                   +0.077951563*(float)cos01[2]
                   +0.068325117*(float)cos01[3]
                   +0.056072974*(float)cos01[4]
                   +0.041665979*(float)cos01[5]
                   +0.025657783*(float)cos01[6]
                   +0.008663573*(float)cos01[7];


        /* c02 , c20 */
        for (i=0;i<4;i++){
            cos20[i]=cols[i]-cols[7-i]-cols[8+i]+cols[15-i];
            cos02[i]=rows[i]-rows[7-i]-rows[8+i]+rows[15-i];
        }

        block->c[2]=0.08669*(float)cos20[0]
                   +0.073492225*(float)cos20[1]
                   +0.049105935*(float)cos20[2]
                   +0.017243711*(float)cos20[3];

        block->c[5]=0.08669*(float)cos02[0]
                   +0.073492225*(float)cos02[1]
                   +0.049105935*(float)cos02[2]
                   +0.017243711*(float)cos02[3];

        /* c11 */
        /* posative and negative block pointers*/
        ptp=block->pt;
        ptn=ptp+15;

        /* posative and negative half block pointers*/
        ptp_hb=ptp+block->jump*8+128;
        ptn_hb=ptp_hb+15;

        for (j2=0;j2<64;j2+=8){
            for (i=0;i<8;i++)
                cos11[j2+i]=(*ptp++)-(*ptn--);
            /* start new lines */
            ptp+=block->jump+8;
            ptn=ptp+15;
        }

        for (jn2=56;jn2>=0;jn2-=8){
            for (i=0;i<8;i++)
                cos11[jn2+i]+=(*ptn_hb--)-(*ptp_hb++);
            /* start new lines */
            ptp_hb+=block->jump+8;
            ptn_hb=ptp_hb+15;
        }

        block->c[4]=0.12379908 *(float)cos11[0]
                   +0.119041551*(float)(cos11[1]+cos11[8])
                   +0.109709322*(float)(cos11[2]+cos11[16])
                   +0.096161025*(float)(cos11[3]+cos11[24])
                   +0.078917313*(float)(cos11[4]+cos11[32])
                   +0.058640854*(float)(cos11[5]+cos11[40])
                   +0.03611086 *(float)(cos11[6]+cos11[48])
                   +0.012193145*(float)(cos11[7]+cos11[56])

                   +0.114466851*(float)cos11[9]
                   +0.105493254*(float)(cos11[10]+cos11[17])
                   +0.00924656* (float)(cos11[11]+cos11[25])
                   +0.075884565*(float)(cos11[12]+cos11[33])
                   +0.056387319*(float)(cos11[13]+cos11[41])
                   +0.03472314* (float)(cos11[14]+cos11[49])
                   +0.011724569*(float)(cos11[15]+cos11[57])

                   +0.09722314*(float)cos11[18]
                   +0.085216795*(float)(cos11[19]+cos11[26])
                   +0.069935616*(float)(cos11[20]+cos11[34])
                   +0.051966851*(float)(cos11[21]+cos11[42])
                   +0.032001029*(float)(cos11[22]+cos11[50])
                   +0.010805425*(float)(cos11[23]+cos11[58])

                   +0.074693145*(float)cos11[27]
                   +0.06129908*(float)(cos11[28]+cos11[35])
                   +0.045549326*(float)(cos11[29]+cos11[43])
                   +0.028049136*(float)(cos11[30]+cos11[51])
                   +0.009471034*(float)(cos11[31]+cos11[59])

                   +0.050306855*(float)cos11[36]
                   +0.037381365*(float)(cos11[37]+cos11[44])
                   +0.023019331*(float)(cos11[38]+cos11[52])
                   +0.007772677*(float)(cos11[39]+cos11[60])

                   +0.02777686*(float)cos11[45]
                   +0.017104906*(float)(cos11[46]+cos11[53])
                   +0.00577562*(float)(cos11[47]+cos11[61])

                   +0.010533149*(float)cos11[54]
                   +0.003556609*(float)(cos11[55]+cos11[62])
 
                   +0.00120092*(float)cos11[63];
}

void code8x8m6
(BLOCK *block)
{
        int i,j,j2,jn2;
        int cols[8],rows[8];
        int *ptp,*ptn; /* posative moving pointer , negative going pointer */
        int *ptp_hb,*ptn_hb; /* pin,ptp + half a block offset */
        int cos00,cos01[4],cos10[4],cos02[2],cos20[2],cos11[16];
 
        /* clear memory */
        for (i=0;i<8;i++){
            cols[i]=0;
            rows[i]=0;
        }
 
        /* setup pointer for inital sums */
        ptp=block->pt;
        /* sum pixels */
        for (j=0;j<8;j++,ptp+=block->jump)
            for (i=0;i<8;i++){
                cols[i]+=*ptp;
                rows[j]+=(*ptp++);
            }

        /* c00 coeff */
        cos00=0;
        for (i=0;i<8;i++)
            cos00+=cols[i];
        block->c[0]=0.125*(float)cos00;

        /* c01 , c10 */
        for (i=0;i<4;i++){
            cos10[i]=cols[i]-cols[7-i];
            cos01[i]=rows[i]-rows[7-i];
        }
        block->c[1]=0.17338*(float)cos10[0]
                   +0.14698*(float)cos10[1]
                   +0.098312*(float)cos10[2]
                   +0.034487*(float)cos10[3];
        block->c[3]=0.17338*(float)cos01[0]
                   +0.14698*(float)cos01[1]
                   +0.098312*(float)cos01[2]
                   +0.034487*(float)cos01[3];

        /* c02 , c20 */
        for (i=0;i<2;i++){
            cos20[i]=cols[i]-cols[3-i]-cols[4+i]+cols[7-i];
            cos02[i]=rows[i]-rows[3-i]-rows[4+i]+rows[7-i];
        }
        block->c[2]=0.16332*(float)cos20[0]
                   +0.06765*(float)cos20[1];

        block->c[5]=0.16332*(float)cos02[0]
                   +0.06765*(float)cos02[1];

        /* c11 */
        /* posative and negative block pointers*/
        ptp=block->pt;
        ptn=ptp+7;

        /* posative and negative half block pointers*/
        ptp_hb=ptp+block->jump*4+32;
        ptn_hb=ptp_hb+7;

        for (j2=0;j2<16;j2+=4){
            for (i=0;i<4;i++)
                cos11[j2+i]=(*ptp++)-(*ptn--);
            /* start new lines */
            ptp+=block->jump+4;
            ptn=ptp+7;
        }

        for (jn2=12;jn2>=0;jn2-=4){
            for (i=0;i<4;i++)
                cos11[jn2+i]+=(*ptn_hb--)-(*ptp_hb++);
            /* start new lines */
            ptp_hb+=block->jump+4;
            ptn_hb=ptp_hb+7;
        }
 
        block->c[4]=0.240485*(float)cos11[0]
                   +0.203873*(float)(cos11[1]+cos11[4])
                   +0.136224*(float)(cos11[2]+cos11[8])
                   +0.047835*(float)(cos11[3]+cos11[12])
                   +0.172835*(float)cos11[5]
                   +0.115485*(float)(cos11[6]+cos11[9])
                   +0.040553*(float)(cos11[7]+cos11[13])
                   +0.077165*(float)cos11[10]
                   +0.027097*(float)(cos11[11]+cos11[14])
                   +0.00951506*(float)cos11[15];
}

void code4x4m6
(BLOCK *block)
{
        int i,j,j2,jn2;
        int cols[4],rows[4];
        int *ptp,*ptn; /* posative moving pointer , negative going pointer */
        int *ptp_hb,*ptn_hb; /* pin,ptp + half a block offset */
	int cos00,cos01[2],cos10[2],cos11[4];

        /* clear memory */
        for (i=0;i<4;i++){
            cols[i]=0;
            rows[i]=0;
        }

        /* setup pointer for inital sums */
        ptp=block->pt;
        /* sum pixels */
        for (j=0;j<4;j++,ptp+=block->jump)
            for (i=0;i<4;i++){
                cols[i]+=*ptp;
                rows[j]+=(*ptp++);
            }

        /* c00 coeff */
        cos00=0;
        for (i=0;i<4;i++)
            cos00+=cols[i];
        block->c[0]=0.25*(float)cos00;

        /* c01 , c10 */
        for (i=0;i<2;i++){
            cos10[i]=cols[i]-cols[3-i];
            cos01[i]=rows[i]-rows[3-i];
        }
        block->c[1]=0.326641*(float)cos10[0]
                   +0.135299*(float)cos10[1];
        block->c[3]=0.326641*(float)cos01[0]
                   +0.135299*(float)cos01[1];

        /* c02 , c20 */
        block->c[2]=0.25*(float)(cols[0]-cols[1]-cols[2]+cols[3]);
        block->c[5]=0.25*(float)(rows[0]-rows[1]-rows[2]+rows[3]);
 
        /* c11 */
        /* posative and negative block pointers*/
        ptp=block->pt;
        ptn=ptp+3;
 
        /* posative and negative half block pointers*/
        ptp_hb=ptp+block->jump*2+8;
        ptn_hb=ptp_hb+3;
 
        for (j2=0;j2<4;j2+=2){
            for (i=0;i<2;i++)
                cos11[j2+i]=(*ptp++)-(*ptn--);
            /* start new lines */
            ptp+=block->jump+2;
            ptn=ptp+3;
        }
 
        for (jn2=2;jn2>=0;jn2-=2){
            for (i=0;i<2;i++)
                cos11[jn2+i]+=(*ptn_hb--)-(*ptp_hb++);
            /* start new lines */
            ptp_hb+=block->jump+2;
            ptn_hb=ptp_hb+3;
        }
 
        block->c[4]=0.4267766*(float)cos11[0]
                   +0.1767766*(float)(cos11[1]+cos11[2])
                   +0.0673220*(float)cos11[3];
 
}

void render2x2m4
(BLOCK *block)
{

        float g,a10,a01,a2;
        int *pix;
 
        /* return if block does not need to be rendered ie no coeff */
        if (block->c==NULL)
           return;
 
        /* calculate block multiplies */
        g=0.5*block->c[0];
        a10=0.5*block->c[1];
        a01=0.5*block->c[2];
        a2=0.5*block->c[3];

        /* make pointer to block easier to handle */
        pix=block->pt;
        (*pix++)+=(int) (g+a01+a10+a2);
        (*pix++)+=(int) (g+a01-a10-a2);
        pix+=block->jump;
        (*pix++)+=(int) (g-a01+a10-a2);
        *pix+=    (int) (g-a01-a10+a2);

}

void render4x4m6
( BLOCK *block)
{
        int i,j,sym,cnt,*pix;
        float cos00,cos10[4],cos20[4],cos01[4],cos02[4],cos11[16];
 
        /* return if block does not need to be rendered ie no coeff */
        if (block->c==NULL)
           return;

        /* setup coefficents */
        cos00=0.25*block->c[0];
 
        cos10[0]=0.326641*block->c[1];
        cos10[1]=0.135299*block->c[1];
 
        cos01[0]=0.326641*block->c[3];
        cos01[1]=0.135299*block->c[3];
 
        cos20[0]=0.25*block->c[2];
        cos02[0]=0.25*block->c[5];
        
        cos11[0]=0.4267766*block->c[4];
        cos11[1]=0.1767766*block->c[4];
        cos11[4]=cos11[1];
        cos11[5]=0.0673220*block->c[4];
 
        /* symetric extention of coefficients */
        for (i=1;i<2;i++){
            sym=1-i;
            cos20[i]=-cos20[sym];
            cos02[i]=-cos02[sym];
        }
 
        for (i=2;i<4;i++){
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

        /* remove common factors in additions */
        for (i=0;i<4;i++){
            cos10[i]+=cos00+cos20[i];
            cos01[i]+=cos02[i];
        }

	/* render information */
        pix=block->pt;
        cnt=0;
        for (j=0;j<4;j++,pix+=block->jump)
            for (i=0;i<4;i++)
            (*pix++)+=(int)(cos10[i]+cos01[j]+cos11[cnt++]);
 
}

void render8x8m6
( BLOCK *block)
{
        int i,j,sym,cnt,*pix;
        float cos00,cos10[8],cos20[8],cos01[8],cos02[8],cos11[64];
 
        /* return if block does not need to be rendered ie no coeff */
        if (block->c==NULL)
           return;
 
        /* setup coefficents */
        cos00=0.125*block->c[0];
 
        cos10[0]=0.17338*block->c[1];
        cos10[1]=0.14698*block->c[1];
        cos10[2]=0.098312*block->c[1];
        cos10[3]=0.034487*block->c[1];
 
        cos01[0]=0.17338*block->c[3];
        cos01[1]=0.14698*block->c[3];
        cos01[2]=0.098312*block->c[3];
        cos01[3]=0.034487*block->c[3];
 
        cos20[0]=0.16332*block->c[2];
        cos20[1]=0.06765*block->c[2];
        cos02[0]=0.16332*block->c[5];
        cos02[1]=0.06765*block->c[5];
        
        cos11[0]=0.240485*block->c[4];
        cos11[1]=0.203873*block->c[4];
        cos11[2]=0.136224*block->c[4];
        cos11[3]=0.047835*block->c[4];
        cos11[9]=0.172835*block->c[4];
        cos11[10]=0.115485*block->c[4];
        cos11[11]=0.040553*block->c[4];
        cos11[18]=0.077165*block->c[4];
        cos11[19]=0.027097*block->c[4];
        cos11[27]=0.00951506*block->c[4];

	/* reflect in xy direction */
	for (j=0;j<8;j++)
	    for (i=0;i<j;i++)
		cos11[j*8+i]=cos11[i*8+j];
 
        /* symetric extention of coefficients */
	/* c20 c02 */
        for (i=2;i<4;i++){
            sym=3-i;
            cos20[i]=-cos20[sym];
            cos02[i]=-cos02[sym];
        }
 
	/* c10 c01 c20 c02 */
        for (i=4;i<8;i++){
            sym=7-i;
            cos10[i]=-cos10[sym];
            cos01[i]=-cos01[sym];
 
            cos20[i]=cos20[sym];
            cos02[i]=cos02[sym];
        }
 
	/* c11 */
        for (j=4;j<8;j++)
            for (i=0;i<4;i++)
                cos11[j*8+i]=-cos11[(7-j)*8+i];
        for (j=0;j<8;j++)
            for (i=4;i<8;i++)
                cos11[j*8+i]=-cos11[8*j+7-i];
 
        /* remove common factors in additions */
        for (i=0;i<8;i++){
            cos10[i]+=cos00+cos20[i];
            cos01[i]+=cos02[i];
        }
 
	/* render information */
        pix=block->pt;
        cnt=0;
        for (j=0;j<8;j++,pix+=block->jump)
            for (i=0;i<8;i++)
            (*pix++)+=(int)(cos10[i]+cos01[j]+cos11[cnt++]);
 
}

void render16x16m6
( BLOCK *block)
{
        int i,j,sym,cnt,*pix;
        float cos00,cos10[16],cos20[16],cos01[16],cos02[16],cos11[256];
 
        /* return if block does not need to be rendered ie no coeff */
        if (block->c==NULL)
           return;
 
        /* setup coefficents */
        cos00=0.0625*block->c[0];
 
        cos10[0]=0.087962734*block->c[1];
        cos10[1]=0.084582375*block->c[1];
        cos10[2]=0.077951563*block->c[1];
        cos10[3]=0.068325117*block->c[1];
        cos10[4]=0.056072974*block->c[1];
        cos10[5]=0.041665979*block->c[1];
        cos10[6]=0.025657783*block->c[1];
        cos10[7]=0.008663573*block->c[1];
 
        cos01[0]=0.087962734*block->c[3];
        cos01[1]=0.084582375*block->c[3];
        cos01[2]=0.077951563*block->c[3];
        cos01[3]=0.068325117*block->c[3];
        cos01[4]=0.056072974*block->c[3];
        cos01[5]=0.041665979*block->c[3];
        cos01[6]=0.025657783*block->c[3];
        cos01[7]=0.008663573*block->c[3];
 
        cos20[0]=0.08669*block->c[2];
        cos20[1]=0.073492225*block->c[2];
        cos20[2]=0.049105935*block->c[2];
        cos20[3]=0.017243711*block->c[2];

        cos02[0]=0.08669*block->c[5];
        cos02[1]=0.073492225*block->c[5];
        cos02[2]=0.049105935*block->c[5];
        cos02[3]=0.017243711*block->c[5];

        cos11[0]=0.12379908*block->c[4];
        cos11[1]=0.119041551*block->c[4];
        cos11[2]=0.109709322*block->c[4];
        cos11[3]=0.096161025*block->c[4];
        cos11[4]=0.078917313*block->c[4];
        cos11[5]=0.058640854*block->c[4];
        cos11[6]=0.03611086*block->c[4];
        cos11[7]=0.012193145*block->c[4];
        cos11[17]=0.114466851*block->c[4];
        cos11[18]=0.105493254*block->c[4];
        cos11[19]=0.09246561*block->c[4];
        cos11[20]=0.075884565*block->c[4];
        cos11[21]=0.056387319*block->c[4];
        cos11[22]=0.03472314*block->c[4];
        cos11[23]=0.011724569*block->c[4];
        cos11[34]=0.09722314*block->c[4];
        cos11[35]=0.085216795*block->c[4];
        cos11[36]=0.069935616*block->c[4];
        cos11[37]=0.051966851*block->c[4];
        cos11[38]=0.032001029*block->c[4];
        cos11[39]=0.010805425*block->c[4];
        cos11[51]=0.074693145*block->c[4];
        cos11[52]=0.06129908*block->c[4];
        cos11[53]=0.045549326*block->c[4];
        cos11[54]=0.028049136*block->c[4];
        cos11[55]=0.009471034*block->c[4];
        cos11[68]=0.050306855*block->c[4];
        cos11[69]=0.037381365*block->c[4];
        cos11[70]=0.023019331*block->c[4];
        cos11[71]=0.007772677*block->c[4];
        cos11[85]=0.02777686*block->c[4];
        cos11[86]=0.017104906*block->c[4];
        cos11[87]=0.00577562*block->c[4];
        cos11[102]=0.010533149*block->c[4];
        cos11[103]=0.003556609*block->c[4];
        cos11[119]=0.00120092*block->c[4];

        /* reflect c11 in xy direction */
        for (j=0;j<16;j++)
            for (i=0;i<j;i++)
                cos11[16*j+i]=cos11[16*i+j];
 
        /* symetric extention of coefficients */
        /* c20 c02 */
        for (i=4;i<8;i++){
            sym=7-i;
            cos20[i]=-cos20[sym];
            cos02[i]=-cos02[sym];
        }
 
        /* c10 c01 c20 c02 */
        for (i=8;i<16;i++){
            sym=15-i;
            cos10[i]=-cos10[sym];
            cos01[i]=-cos01[sym];
 
            cos20[i]=cos20[sym];  
            cos02[i]=cos02[sym];
        }
 
        /* c11 */
        for (j=8;j<16;j++)
            for (i=0;i<8;i++)
                cos11[16*j+i]=-cos11[16*(15-j)+i];
        for (j=0;j<16;j++)
            for (i=8;i<16;i++)
                cos11[16*j+i]=-cos11[16*j+(15-i)];
 
        /* remove common factors in additions */  
        for (i=0;i<16;i++){
            cos10[i]+=cos00+cos20[i];
            cos01[i]+=cos02[i];
        }
 
	/* render information */
        pix=block->pt;
        cnt=0;         
        for (j=0;j<16;j++,pix+=block->jump)
            for (i=0;i<16;i++)
            (*pix++)+=(int)(cos10[i]+cos01[j]+cos11[cnt++]);
 
}                                                       

void render32x32m6
( BLOCK *block)
{
        int i,j,sym,cnt,*pix;
        float cos00,cos10[32],cos20[32],cos01[32],cos02[32],cos11[1024];

        if (block->c==NULL)
           return;

        cos00=0.03125*block->c[0];
	
	cos10[0]=block->c[1]*0.04414094;
	cos10[1]=block->c[1]*0.04371584;
	cos10[2]=block->c[1]*0.04286973;
	cos10[3]=block->c[1]*0.04161076;
	cos10[4]=block->c[1]*0.03995106;
	cos10[5]=block->c[1]*0.03790661;
	cos10[6]=block->c[1]*0.03549709;
	cos10[7]=block->c[1]*0.03274572;
	cos10[8]=block->c[1]*0.02967899;
	cos10[9]=block->c[1]*0.02632644;
	cos10[10]=block->c[1]*0.02272035;
	cos10[11]=block->c[1]*0.01889544;
	cos10[12]=block->c[1]*0.01488857;
	cos10[13]=block->c[1]*0.01073831;
	cos10[14]=block->c[1]*0.00648463;
	cos10[15]=block->c[1]*0.00216851;

	cos01[0]=block->c[3]*0.04414094;
	cos01[1]=block->c[3]*0.04371584;
	cos01[2]=block->c[3]*0.04286973;
	cos01[3]=block->c[3]*0.04161076;
	cos01[4]=block->c[3]*0.03995106;
	cos01[5]=block->c[3]*0.03790661;
	cos01[6]=block->c[3]*0.03549709;
	cos01[7]=block->c[3]*0.03274572;
	cos01[8]=block->c[3]*0.02967899;
	cos01[9]=block->c[3]*0.02632644;
	cos01[10]=block->c[3]*0.02272035;
	cos01[11]=block->c[3]*0.01889544;
	cos01[12]=block->c[3]*0.01488857;
	cos01[13]=block->c[3]*0.01073831;
	cos01[14]=block->c[3]*0.00648463;
	cos01[15]=block->c[3]*0.00216851;

	cos20[0]=block->c[2]*0.04398137;
	cos20[1]=block->c[2]*0.04229119;
	cos20[2]=block->c[2]*0.03897578;
	cos20[3]=block->c[2]*0.03416256;
	cos20[4]=block->c[2]*0.02803649;
	cos20[5]=block->c[2]*0.02083299;
	cos20[6]=block->c[2]*0.01282889;
	cos20[7]=block->c[2]*0.00433179;

	cos02[0]=block->c[5]*0.04398137;
	cos02[1]=block->c[5]*0.04229119;
	cos02[2]=block->c[5]*0.03897578;
	cos02[3]=block->c[5]*0.03416256;
	cos02[4]=block->c[5]*0.02803649;
	cos02[5]=block->c[5]*0.02083299;
	cos02[6]=block->c[5]*0.01282889;
	cos02[7]=block->c[5]*0.00433179;

	cos11[0]=block->c[4]*0.06234952;
	cos11[1]=block->c[4]*0.06174906;
	cos11[2]=block->c[4]*0.06055393;
	cos11[3]=block->c[4]*0.05877562;
	cos11[4]=block->c[4]*0.05643127;
	cos11[5]=block->c[4]*0.05354346;
	cos11[6]=block->c[4]*0.05014000;
	cos11[7]=block->c[4]*0.04625366;
	cos11[8]=block->c[4]*0.04192188;
	cos11[9]=block->c[4]*0.03718636;
	cos11[10]=block->c[4]*0.03209272;
	cos11[11]=block->c[4]*0.02669001;
	cos11[12]=block->c[4]*0.02103025;
	cos11[13]=block->c[4]*0.01516797;
	cos11[14]=block->c[4]*0.00915961;
	cos11[15]=block->c[4]*0.00306304;
	cos11[33]=block->c[4]*0.06115439;
	cos11[34]=block->c[4]*0.05997076;
	cos11[35]=block->c[4]*0.05820958;
	cos11[36]=block->c[4]*0.05588781;
	cos11[37]=block->c[4]*0.05302781;
	cos11[38]=block->c[4]*0.04965713;
	cos11[39]=block->c[4]*0.04580822;
	cos11[40]=block->c[4]*0.04151815;
	cos11[41]=block->c[4]*0.03682823;
	cos11[42]=block->c[4]*0.03178365;
	cos11[43]=block->c[4]*0.02643297;
	cos11[44]=block->c[4]*0.02082772;
	cos11[45]=block->c[4]*0.01502189;
	cos11[46]=block->c[4]*0.00907140;
	cos11[47]=block->c[4]*0.00303354;
	cos11[66]=block->c[4]*0.05881004;
	cos11[67]=block->c[4]*0.05708295;
	cos11[68]=block->c[4]*0.05480612;
	cos11[69]=block->c[4]*0.05200147;
	cos11[70]=block->c[4]*0.04869603;
	cos11[71]=block->c[4]*0.04492161;
	cos11[72]=block->c[4]*0.04071457;
	cos11[73]=block->c[4]*0.03611543;
	cos11[74]=block->c[4]*0.03116848;
	cos11[75]=block->c[4]*0.02592136;
	cos11[76]=block->c[4]*0.02042461;
	cos11[77]=block->c[4]*0.01473115;
	cos11[78]=block->c[4]*0.00889582;
	cos11[79]=block->c[4]*0.00297482;
	cos11[99]=block->c[4]*0.05540658;
	cos11[100]=block->c[4]*0.05319661;
	cos11[101]=block->c[4]*0.05047433;
	cos11[102]=block->c[4]*0.04726596;
	cos11[103]=block->c[4]*0.04360238;
	cos11[104]=block->c[4]*0.03951890;
	cos11[105]=block->c[4]*0.03505482;
	cos11[106]=block->c[4]*0.03025315;
	cos11[107]=block->c[4]*0.02516012;
	cos11[108]=block->c[4]*0.01982479;
	cos11[109]=block->c[4]*0.01429853;
	cos11[110]=block->c[4]*0.00863458;
	cos11[111]=block->c[4]*0.00288746;
	cos11[132]=block->c[4]*0.05107479;
	cos11[133]=block->c[4]*0.04846109;
	cos11[134]=block->c[4]*0.04538069;
	cos11[135]=block->c[4]*0.04186324;
	cos11[136]=block->c[4]*0.03794263;
	cos11[137]=block->c[4]*0.03365661;
	cos11[138]=block->c[4]*0.02904646;
	cos11[139]=block->c[4]*0.02415658;
	cos11[140]=block->c[4]*0.01903405;
	cos11[141]=block->c[4]*0.01372822;
	cos11[142]=block->c[4]*0.00829017;
	cos11[143]=block->c[4]*0.00277229;
	cos11[165]=block->c[4]*0.04598115;
	cos11[166]=block->c[4]*0.04305838;
	cos11[167]=block->c[4]*0.03972094;
	cos11[168]=block->c[4]*0.03600096;
	cos11[169]=block->c[4]*0.03193427;
	cos11[170]=block->c[4]*0.02756004;
	cos11[171]=block->c[4]*0.02292039;
	cos11[172]=block->c[4]*0.01806000;
	cos11[173]=block->c[4]*0.01302569;
	cos11[174]=block->c[4]*0.00786593;
	cos11[175]=block->c[4]*0.00263042;
	cos11[198]=block->c[4]*0.04032140;
	cos11[199]=block->c[4]*0.03719610;
	cos11[200]=block->c[4]*0.03371258;
	cos11[201]=block->c[4]*0.02990439;
	cos11[202]=block->c[4]*0.02580820;
	cos11[203]=block->c[4]*0.02146347;
	cos11[204]=block->c[4]*0.01691203;
	cos11[205]=block->c[4]*0.01219772;
	cos11[206]=block->c[4]*0.00736594;
	cos11[207]=block->c[4]*0.00246322;
	cos11[231]=block->c[4]*0.03431304;
	cos11[232]=block->c[4]*0.03109952;
	cos11[233]=block->c[4]*0.02758650;
	cos11[234]=block->c[4]*0.02380781;
	cos11[235]=block->c[4]*0.01979984;
	cos11[236]=block->c[4]*0.01560118;
	cos11[237]=block->c[4]*0.01125228;
	cos11[238]=block->c[4]*0.00679501;
	cos11[239]=block->c[4]*0.00227230;
	cos11[264]=block->c[4]*0.02818696;
	cos11[265]=block->c[4]*0.02500295;
	cos11[266]=block->c[4]*0.02157814;
	cos11[267]=block->c[4]*0.01794553;
	cos11[268]=block->c[4]*0.01414009;
	cos11[269]=block->c[4]*0.01019847;
	cos11[270]=block->c[4]*0.00615864;
	cos11[271]=block->c[4]*0.00205949;
	cos11[297]=block->c[4]*0.02217860;
	cos11[298]=block->c[4]*0.01914067;
	cos11[299]=block->c[4]*0.01591839;
	cos11[300]=block->c[4]*0.01254282;
	cos11[301]=block->c[4]*0.00904645;
	cos11[302]=block->c[4]*0.00546295;
	cos11[303]=block->c[4]*0.00182685;
	cos11[330]=block->c[4]*0.01651885;
	cos11[331]=block->c[4]*0.01373795;
	cos11[332]=block->c[4]*0.01082475;
	cos11[333]=block->c[4]*0.00780730;
	cos11[334]=block->c[4]*0.00471466;
	cos11[335]=block->c[4]*0.00157661;
	cos11[363]=block->c[4]*0.01142521;
	cos11[364]=block->c[4]*0.00900244;
	cos11[365]=block->c[4]*0.00649296;
	cos11[366]=block->c[4]*0.00392096;
	cos11[367]=block->c[4]*0.00131120;
	cos11[396]=block->c[4]*0.00709342;
	cos11[397]=block->c[4]*0.00511610;
	cos11[398]=block->c[4]*0.00308950;
	cos11[399]=block->c[4]*0.00103315;
	cos11[429]=block->c[4]*0.00368996;
	cos11[430]=block->c[4]*0.00222829;
	cos11[431]=block->c[4]*0.00074515;
	cos11[462]=block->c[4]*0.00134561;
	cos11[463]=block->c[4]*0.00044998;
	cos11[495]=block->c[4]*0.00015048;

	/* reflect c11 in xy direction */
        for (j=0;j<32;j++)
            for (i=0;i<j;i++)
                cos11[32*j+i]=cos11[32*i+j];

        for (i=8;i<16;i++){
            sym=15-i;
            cos20[i]=-cos20[sym];
            cos02[i]=-cos02[sym];
        }

        for (i=16;i<32;i++){
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

        for (i=0;i<32;i++){
            cos10[i]+=cos00+cos20[i];
            cos01[i]+=cos02[i];
        }
 
        pix=block->pt;
        cnt=0;
        for (j=0;j<32;j++,pix+=block->jump)
            for (i=0;i<32;i++)
            (*pix++)+=(int)(cos10[i]+cos01[j]+cos11[cnt++]);
 
}


/*
#define N 4
void render4x4m6
( BLOCK *block)
{
	int i,j,sym,cnt,*pix;
	float cos00,cos10[N],cos20[N],cos01[N],cos02[N],cos11[N*N];

        if (block->c==NULL)
           return;

	cos00=0.25*block->c[0];

	cos10[0]=0.326641*block->c[1];
	cos10[1]=0.135299*block->c[1];

	cos01[0]=0.326641*block->c[3];
	cos01[1]=0.135299*block->c[3];

	cos20[0]=0.25*block->c[2];
	cos02[0]=0.25*block->c[5];
	
	cos11[0]=0.4267766*block->c[4];
        cos11[1]=0.1767766*block->c[4];
        cos11[N]=cos11[1];
        cos11[1+N]=0.0673220*block->c[4];

	for (i=N>>2;i<N>>1;i++){
	    sym=(N>>1)-1-i;
	    cos20[i]=-cos20[sym];
	    cos02[i]=-cos02[sym];
	}

	for (i=N>>1;i<N;i++){
	    sym=N-1-i;
	    cos10[i]=-cos10[sym];
	    cos01[i]=-cos01[sym];

	    cos20[i]=cos20[sym];
	    cos02[i]=cos02[sym];
	}

	for (j=N>>1;j<N;j++)
	    for (i=0;i<N>>1;i++)
		cos11[j*N+i]=-cos11[(N-1-j)*N+i];
	for (j=0;j<N;j++)
	    for (i=N>>1;i<N;i++)
		cos11[j*N+i]=-cos11[N*j+(N-1-i)];

	for (i=0;i<N;i++){
	    cos10[i]+=cos00+cos20[i];
	    cos01[i]+=cos02[i];
	}

        block->pt=malloc_int(block->size*block->size,"allocate block mem-rendering"); 
	pix=block->pt;
	cnt=0;
	for (j=0;j<N;j++)
	    for (i=0;i<N;i++)
	    (*pix++)=(int)(cos10[i]+cos01[j]+cos11[cnt++]);

}
*/
