//////////////////////////////////////////////////////////////////////
//					  CyberGuard Frontend							//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "graphics.h"
#include <time.h>
#include "codec.h"

void MainProc();
bool isWindowMode=true;

Screen *screen;
Page *page;

VideoData *frame0;
VideoData *frame1;
VideoData *frame2;
VideoData *frame3;

VideoData * ReadPPM(char *fname);
void RGBtoYUV(double *RGB, double *YUV);



/////////////////////////////////////////////////////////////////////////////////////////////
// WinProc - Handle Windows messages													   //
/////////////////////////////////////////////////////////////////////////////////////////////

long PASCAL WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool ok=true;

	if (page)
		page->WindowsMessages(message,wParam,lParam);

	switch(message)
	{

		case WM_CREATE:
		break;

		case WM_KEYDOWN:
			switch(wParam)
			{
		 		case VK_ESCAPE:
					PostMessage(hWnd, WM_DESTROY, 0, 0);
					break;	
			}
			break;

			
		case WM_TIMER:
			break;
			
		case WM_DESTROY:
			PostQuitMessage (0) ;
		  	break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void DrawGrid(char *videoName,int width,int height,int size)
{ 
	int i;
	Sprite *sprite=page->GetSprite(videoName);
	for (i=0;i<width;i+=size)
		page->DrawBoxOnVideoWindow(sprite,i,0,size+1,height,COLOUR_WHITE);
	for (i=0;i<height;i+=size)
		page->DrawBoxOnVideoWindow(sprite,0,i,width,size+1,COLOUR_WHITE);

}

/////////////////////////////////////////////////////////////////////////////////////////////

double MSE
(VideoData *f0,
VideoData *f1)
{
	double diff;
	int sum=0,i,j;
	unsigned char *Y0=f0->m_Y;
	unsigned char *Y1=f1->m_Y;
	int jump=f0->m_pitch;

	for (j=0;j<f0->m_height;j++,Y0+=jump,Y1+=jump)
	{
		for (i=0;i<f0->m_width;i++)
		{
			diff=(double)(Y0[i]-Y1[i]);
			sum+=diff*diff;
		}
	}

	return sum;
}

int Diff
(VideoData *f0,
VideoData *f1,
VideoData *f2)
{
	int diff,sum=0,i,j;
	unsigned char *Y0=f0->m_Y;
	unsigned char *Y1=f1->m_Y;
	unsigned char *Y2=f2->m_Y;
	int jump=f0->m_pitch;

	for (j=0;j<f0->m_height;j++,Y0+=jump,Y1+=jump,Y2+=jump)
	{
		for (i=0;i<f0->m_width;i++)
		{
			diff=(Y0[i]-Y1[i]);
			if (diff>0)
				Y2[i]=diff;
			else
				Y2[i]=-diff;
		}
	}

	return sum;
}

Qtree *qtree;

BOOL InitApp(HINSTANCE hInst, int nCmdShow)
{
	WNDCLASS WndClass;
	RECT rect;
	HWND hWnd;

	WndClass.style = CS_DBLCLKS;
	WndClass.lpfnWndProc = WinProc;
	WndClass.cbClsExtra = 0;
 	WndClass.cbWndExtra = 0;
 	WndClass.hInstance = hInst;
	WndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.lpszMenuName = 0;
	WndClass.lpszClassName = "Watch Dog";
	RegisterClass(&WndClass);

	if (isWindowMode)
	{
		rect.top=0;
		rect.bottom=600;
		rect.left=0;
		rect.right=800;
		AdjustWindowRect(&rect,WS_CAPTION,FALSE);   
		rect.right-=rect.left;
		rect.bottom-=rect.top;

		hWnd= CreateWindow(
				"Watch Dog",
				"Watch Dog",
 				WS_CAPTION |WS_SYSMENU ,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				rect.right,
				rect.bottom,
				NULL,
				NULL,
				hInst,
				NULL);

	}
	else
	{
	hWnd= CreateWindow(
				"Watch Dog",
				"Watch Dog",
 				WS_POPUP ,
				0,
				0,
				GetSystemMetrics(SM_CXSCREEN),
				GetSystemMetrics(SM_CYSCREEN),
				NULL,
				NULL,
				hInst,
				NULL);
	}
	if(!hWnd) return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	screen = new Screen();
	if (isWindowMode)
		screen->CreateWindowed(hWnd,800,600);
	else
		screen->CreateFullScreen(hWnd,800,600,32);
	screen->Fill(0x0);
	screen->Flip();
	screen->Fill(0x0);
	screen->Flip();

	page=new Page;
	page->AttachScreen(screen);
	page->SetSourceDirectory("");
	page->AddBackground("blank.*",0,0);
	page->ChangeFont("Courier New",16,850,true);

	frame0=ReadPPM("0.ppm");
	
	frame1=VideoData_Create(352,352,288,false,false);
	frame2=VideoData_Create(352,352,288,false,false);

	page->AddVideoWindow("OLDVID",0,0,352,288);
	page->AddVideoWindow("NEWVID",352,0,352,288);
	page->AddVideoWindow("DIFF",352,288,352,288);
	page->AddStaticTextBox(0,340,200,20,"MSE","MSE");
	page->AddStaticTextBox(0,360,200,20,"SPEED","MSE");

	double bbp=0.3;
	qtree=Qtree_Create();
	qtree->m_FFBPP=bbp;
	qtree->m_BPP=bbp;
	qtree->m_noUpdate=true;
	Qtree_CompressImage(qtree,frame0);
	Qtree_DecompressImage(qtree,0,0);


//	page->SendVideoToWindow("OLDVID",frame0);
//	memset(qtree->m_last->m_U,128,176*288);
//	memset(qtree->m_last->m_V,128,176*288);

	page->SendVideoToWindow("OLDVID",qtree->m_last);

	char dummy[256];
///	double mse=MSE(frame1,frame2);
//	sprintf(dummy,"MSE = %.1f",mse/352.0/288.0);
//	page->ChangeText("MSE",dummy);

//	sprintf(dummy,"Speed= %.2f",(double)(end-start)/1.0);
//	page->ChangeText("SPEED",dummy);


	Qtree *qtree2=Qtree_Create();
	qtree2->m_FFBPP=bbp;
	qtree2->m_BPP=bbp;
	qtree2->m_noUpdate=true;
	qtree2->m_postProcess=true;
	Qtree_CompressImage(qtree2,frame0);
	Qtree_DecompressImage(qtree2,0,0);
	page->SendVideoToWindow("NEWVID",qtree2->m_last);

	Diff(qtree->m_last,qtree2->m_last,frame1);

	page->SendVideoToWindow("DIFF",frame1);

	return TRUE;
}
 
/////////////////////////////////////////////////////////////////////////////////////////////
// WinMain - The main program loop
/////////////////////////////////////////////////////////////////////////////////////////////

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
 {
	MSG msg;
	int first_time=1;

	if(!InitApp(hInst, nCmdShow)) return FALSE;

	while(1)
	{

		if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if(!GetMessage(&msg, NULL, 0, 0 )) return msg.wParam;
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		} 
		else 
		{
			MainProc();
		}
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////

void MainProc()
{
	GraphicsObject *object;
	TextBox *text;
	if (!page)
		return;
	object=page->CheckMouse();
	page->Update();
	page->Flip();
	return;
} 

/////////////////////////////////////////////////////////////////////////////////////////////

VideoData * ReadPPM(char *fname)
{
	char dummy[256];
	BYTE *Y,*U,*V;
	int width,height;
	int i,j;
	unsigned char tmp[3];
    double RGB[3];
    double YUV[3];
	VideoData * buffer;
	FILE *fp=NULL;

	// get file from disk 
	fp=fopen(fname,"rb");

	if (!fp)
	   return NULL;

    // remove P5/6 line 
    fgets(dummy,255,fp);

    //sets up colour 
    if (strncmp(dummy,"P6",2))
		return NULL;
 
	//remove comment lines 
    do {
       fgets(dummy,255,fp);
	} while (strncmp(dummy,"#",1)==0);
 
    // read in image stats          
    sscanf(dummy,"%d %d",&width,&height);

	// remove 255 line 
    fgets(dummy,255,fp);

	buffer=VideoData_Create(width,width,height,false,false);

	Y=buffer->m_Y;
	U=buffer->m_U;
	V=buffer->m_V;

	// read in image data -> file is 3 times as large as image (RGB)
	for (j=0;j<height;j++)
        for (i=0;i<width;i+=2)
		{
	        fread(tmp,sizeof(unsigned char),3,fp);
			RGB[0]=(double)tmp[0];	// R
			RGB[1]=(double)tmp[1];	// G
			RGB[2]=(double)tmp[2];	// B
			RGBtoYUV(RGB,YUV);
			*Y++=(BYTE)limit((int)rint(YUV[0]));	//Y
			*U++=(BYTE)limit((int)rint(YUV[1]+128));  //U
			*V++=(BYTE)limit((int)rint(YUV[2]+128));  //V

			fread(tmp,sizeof(unsigned char),3,fp);
			RGB[0]=(double)tmp[0];	// R
			RGB[1]=(double)tmp[1];	// G
			RGB[2]=(double)tmp[2];	// B
			RGBtoYUV(RGB,YUV);
			*Y++=(BYTE)limit((int)rint(YUV[0]));		//Y
		}
 
	fclose(fp);
	return buffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void RGBtoYUV(double *RGB, double *YUV)
{

    // convert to YUV 
	//  		R				G				B
    YUV[0]= 0.2990*RGB[0]+0.5870*RGB[1]+0.1140*RGB[2]; // Y
    YUV[1]=-0.1686*RGB[0]-0.3311*RGB[1]+0.4997*RGB[2]; // U
    YUV[2]= 0.4998*RGB[0]-0.4185*RGB[1]-0.0813*RGB[2]; // V

}

////////////////////////////////////////////////////////////////////////////////////////////

