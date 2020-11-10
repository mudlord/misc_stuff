

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <time.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"

#include "nyancat.h"
#include "rainbow.h"
#include "stars.h"



//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND                    g_hWnd          = NULL;
LPDIRECT3D9             g_pD3D          = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice    = NULL;

struct TFrame
{
	BOOL up; //up or down?
	D3DXVECTOR3 pos;
	int frameNumber;
	DWORD lastUpdate;
};
// The starz
TFrame *starz=NULL;
// The rainbow segments
TFrame *rainbowz=NULL;
// The number of starz
const int kNumStars=24;
//number of rainbow segments
const int kNumRainbows=13;


LPD3DXSPRITE nyansprite=NULL;					// nyan! :3
LPDIRECT3DTEXTURE9 nyantexture=NULL;			// nyan texture
LPD3DXSPRITE starsprite=NULL;					// bg sprite 1
LPDIRECT3DTEXTURE9 startexture=NULL;				
LPD3DXSPRITE rainbowsprite=NULL;			   //rainbows! NYAN!
LPDIRECT3DTEXTURE9 rainbowtexture=NULL;		   //nyan nyan nyan nyan :3

#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_TEX1 )

#define BASSDEF(f) (WINAPI *f)	// define the functions as pointers
#include "bass.h"
#include "nyan_tune.h"
#include "bass_dll.h"
char tempfile[MAX_PATH];	// temporary BASS.DLL
HINSTANCE bass=0;			// bass handle
void music()
{
	DWORD len,c;
	HANDLE hfile;
	char temppath[MAX_PATH];
	GetTempPath(MAX_PATH,temppath);
	GetTempFileName(temppath,"bas",0,tempfile);
	if (INVALID_HANDLE_VALUE==(hfile=CreateFile(tempfile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY,NULL))) {
		MessageBox(NULL,"Can't write BASS.DLL!","FAIL",MB_ICONSTOP);
		ExitProcess(0);
	}
	WriteFile(hfile,bass_data,bass_len,&c,NULL);
	CloseHandle(hfile);
	if (!(bass=LoadLibrary(tempfile))) {
		MessageBox(NULL,"Can't load BASS.DLL!","FAIL",MB_ICONSTOP);
		ExitProcess(0);
	}
	/* "load" all the BASS functions that are to be used */
#define LOADBASSFUNCTION(f) *((void**)&f)=GetProcAddress(bass,#f)
	LOADBASSFUNCTION(BASS_ErrorGetCode);
	LOADBASSFUNCTION(BASS_Init);
	LOADBASSFUNCTION(BASS_Free);
	LOADBASSFUNCTION(BASS_StreamCreateFile);
	LOADBASSFUNCTION(BASS_ChannelPlay);
	BASS_Init(-1,44100,0,g_hWnd,NULL);
	HSTREAM str = BASS_StreamCreateFile(TRUE,nyan_tune,0,nyan_tune_len,BASS_SAMPLE_LOOP);
	BASS_ChannelPlay(str,TRUE);
}

void FreeBASS()
{
	if (!bass) return;
	BASS_Free();
	FreeLibrary(bass);
	bass=0;
	DeleteFile(tempfile);
}

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTextures(void);
void loadNyan();
void loadStarfield();
void loadRainbows();
void init(void);
void shutDown(void);
void render(void);


//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     lpCmdLine,
					int       nCmdShow )
{
	WNDCLASSEX winClass; 
	MSG        uMsg;
    memset(&uMsg,0,sizeof(uMsg));
	winClass.lpszClassName = "MY_WINDOWS_CLASS";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
	winClass.hIcon	       = LoadIcon(hInstance, (LPCTSTR)IDI_ICON1);
    winClass.hIconSm	   = LoadIcon(hInstance, (LPCTSTR)IDI_ICON1);
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;

	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
		                     "nyan ^-^",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 640, 480, NULL, NULL, hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();
	music();

	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
        else
		{
			render();
		}
	}
	shutDown();

    UnregisterClass( "MY_WINDOWS_CLASS", winClass.hInstance );

	return uMsg.wParam;
}

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
	static POINT ptLastMousePosit;
	static POINT ptCurrentMousePosit;
	static bool bMousing;
	
    switch( msg )
	{	
        case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;
			}
		}
        break;
		case WM_CLOSE:
        case WM_DESTROY:
		{
			FreeBASS();
            PostQuitMessage(0);
		}
        break;

		default:
		{
			return DefWindowProc( hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Name: loadTexture()
// Desc: 
//-----------------------------------------------------------------------------
void loadTextures( void )
{
	loadNyan();
	loadStarfield();
	loadRainbows();

}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
void init( void )
{
    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
	D3DCAPS9 d3dCaps;
	g_pD3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps );
    D3DDISPLAYMODE d3ddm;
    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed               = TRUE;
	d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd,
                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice );
	time_t seconds;
	time(&seconds);
	srand((unsigned int) seconds);
	loadTextures();
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();
    if( g_pD3D != NULL )
        g_pD3D->Release();
}

void loadRainbows()
{
	D3DXIMAGE_INFO d3dxImageInfo;
	D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice,rainbow,rainbow_len,9, // I had to set width manually. D3DPOOL_DEFAULT works for textures but causes problems for D3DXSPRITE.
		18, // I had to set height manually. D3DPOOL_DEFAULT works for textures but causes problems for D3DXSPRITE.
		1,   // Don't create mip-maps when you plan on using D3DXSPRITE. It throws off the pixel math for sprite animation.
		D3DPOOL_DEFAULT,
		D3DFMT_UNKNOWN,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f),
		&d3dxImageInfo,
		NULL,
		&rainbowtexture);
/*	D3DXCreateTextureFromFileEx( g_pd3dDevice,
		"sprites/rainbow.png",
		9, // I had to set width manually. D3DPOOL_DEFAULT works for textures but causes problems for D3DXSPRITE.
		18, // I had to set height manually. D3DPOOL_DEFAULT works for textures but causes problems for D3DXSPRITE.
		1,   // Don't create mip-maps when you plan on using D3DXSPRITE. It throws off the pixel math for sprite animation.
		D3DPOOL_DEFAULT,
		D3DFMT_UNKNOWN,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f),
		&d3dxImageInfo,
		NULL,
		&rainbowtexture );*/
	D3DXCreateSprite(g_pd3dDevice,&rainbowsprite);
	rainbowz=new TFrame[kNumRainbows];
	RECT screenRect;
	GetClientRect(g_hWnd,&screenRect);
	for (int i=0;i<kNumRainbows;i++)
	{
		float pos = 110;
		rainbowz[i].frameNumber=0;
		BOOL up = ( i % 2 == 0 )? TRUE : FALSE;
		pos--;
		if (up) pos+=2;
		rainbowz[i].up = up;
		rainbowz[i].pos=D3DXVECTOR3(i*9,pos,0.0f);
		rainbowz[i].lastUpdate=timeGetTime();
	}
}

void DrawRainbows()
{

	D3DXMATRIX spriteMatrix;
	rainbowsprite->Begin(D3DXSPRITE_ALPHABLEND);
	for (int i=0;i<kNumRainbows;i++)
	{			
		RECT srcRect;
		srcRect.top    = 0;
		srcRect.left   = 0;
		srcRect.bottom = 18;
		srcRect.right  = 9;
		starz[i].pos.z = 2;
		D3DXVECTOR2 scaling(2.0f, 2.0f);
		D3DXMatrixTransformation2D(&spriteMatrix, 0, 0, &scaling, 0, 0,0);
		rainbowsprite->SetTransform(&spriteMatrix);
		rainbowsprite->Draw(rainbowtexture,&srcRect,NULL, &rainbowz[i].pos,D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f));
	}
	rainbowsprite->End();
	DWORD timeNow=timeGetTime();
	RECT screenRect;
	const float kMoveAmount=2.0f;
	const int kTimeBetweenUpdated=200;
	GetClientRect(g_hWnd,&screenRect);

	for (int i=0;i<kNumRainbows;i++)
	{
		if (timeNow-rainbowz[i].lastUpdate>kTimeBetweenUpdated)
		{
			if (rainbowz[i].up == TRUE)
			{
				rainbowz[i].pos.y -= kMoveAmount;
				rainbowz[i].up = FALSE;
			}
			else
			{
				rainbowz[i].pos.y += kMoveAmount;
				rainbowz[i].up = TRUE;
			}
			// Increase frame count and move
		//	rainbowz[i].pos.y;
			// Remember the last update time
			rainbowz[i].lastUpdate=timeNow;
		}
	}


}

void loadStarfield()
{

	D3DXIMAGE_INFO d3dxImageInfo;
	D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice,stars,stars_len,7, // I had to set width manually. D3DPOOL_DEFAULT works for textures but causes problems for D3DXSPRITE.
		42, // I had to set height manually. D3DPOOL_DEFAULT works for textures but causes problems for D3DXSPRITE.
		1,   // Don't create mip-maps when you plan on using D3DXSPRITE. It throws off the pixel math for sprite animation.
		D3DPOOL_DEFAULT,
		D3DFMT_UNKNOWN,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f),
		&d3dxImageInfo,
		NULL,
		&startexture);
/*	D3DXCreateTextureFromFileEx( g_pd3dDevice,
		"sprites/stars_alpha.png",
		7, // I had to set width manually. D3DPOOL_DEFAULT works for textures but causes problems for D3DXSPRITE.
		42, // I had to set height manually. D3DPOOL_DEFAULT works for textures but causes problems for D3DXSPRITE.
		1,   // Don't create mip-maps when you plan on using D3DXSPRITE. It throws off the pixel math for sprite animation.
		D3DPOOL_DEFAULT,
		D3DFMT_UNKNOWN,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f),
		&d3dxImageInfo,
		NULL,
		&startexture );*/
	D3DXCreateSprite(g_pd3dDevice,&starsprite);
	starz=new TFrame[kNumStars];
	RECT screenRect;
	GetClientRect(g_hWnd,&screenRect);
	for (int i=0;i<kNumStars;i++)
	{
			float xPos=(float)(rand()%screenRect.right);
			starz[i].frameNumber=rand()%5;
			starz[i].pos=D3DXVECTOR3(xPos,i*6.0f,0.0f);
			starz[i].lastUpdate=timeGetTime();
	}
}
void DrawStarfield()
{
	D3DXMATRIX spriteMatrix;
	starsprite->Begin(D3DXSPRITE_ALPHABLEND);
	const float kMoveAmount=10.0f;
	const int kTimeBetweenUpdated=70;
	for (int i=0;i<kNumStars;i++)
	{			
		RECT srcRect;
		srcRect.top    = starz[i].frameNumber * 7;
		srcRect.left   = 0;
		srcRect.bottom = srcRect.top  + 7;
		srcRect.right  = 7;
		starz[i].pos.z = 1;
		D3DXVECTOR2 scaling(3.0f, 3.0f);
		D3DXMatrixTransformation2D(&spriteMatrix, 0, 0, &scaling, 0, 0,0);
		starsprite->SetTransform(&spriteMatrix);
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		starsprite->Draw(startexture,&srcRect,NULL, &starz[i].pos,D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f));
	}
	starsprite->End();


	DWORD timeNow=timeGetTime();

	RECT screenRect;
	GetClientRect(g_hWnd,&screenRect);

	for (int i=0;i<kNumStars;i++)
	{
		if (timeNow-starz[i].lastUpdate>kTimeBetweenUpdated)
		{
			// Increase frame count and move
			starz[i].frameNumber++;
			starz[i].pos.x-=kMoveAmount;
			// Loop to first frame
			if (starz[i].frameNumber>5)
			starz[i].frameNumber=0;
			// Check for edge of screen - if reached, wrap!
			if (starz[i].pos.x<screenRect.left)starz[i].pos.x=screenRect.right-7.0f;
			// Remember the last update time
			starz[i].lastUpdate=timeNow;
		}
	}


}

void loadNyan()
{
	D3DXIMAGE_INFO d3dxImageInfo;

	D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice,nyancat,nyancat_len,34, // I had to set width manually. D3DPOOL_DEFAULT works for textures but causes problems for D3DXSPRITE.
		126, // I had to set height manually. D3DPOOL_DEFAULT works for textures but causes problems for D3DXSPRITE.
		1,   // Don't create mip-maps when you plan on using D3DXSPRITE. It throws off the pixel math for sprite animation.
		D3DPOOL_DEFAULT,
		D3DFMT_UNKNOWN,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f),
		&d3dxImageInfo,
		NULL,
		&nyantexture);
	    D3DXCreateSprite(g_pd3dDevice,&nyansprite);
	/*D3DXCreateTextureFromFileEx( g_pd3dDevice,
		"sprites/nyancat.png",
		34, // I had to set width manually. D3DPOOL_DEFAULT works for textures but causes problems for D3DXSPRITE.
		126, // I had to set height manually. D3DPOOL_DEFAULT works for textures but causes problems for D3DXSPRITE.
		1,   // Don't create mip-maps when you plan on using D3DXSPRITE. It throws off the pixel math for sprite animation.
		D3DPOOL_DEFAULT,
		D3DFMT_UNKNOWN,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f),
		&d3dxImageInfo,
		NULL,
		&nyantexture );
	*/
}

void DrawNyan()
{
	const int kTimeBetweenUpdated=40; //ms between frames
	DWORD timeNow=timeGetTime();
	static BOOL init_nyantime = FALSE;
	static int frame_no = 0;
	static DWORD nyan_lastupdate = 0;
	if (!init_nyantime)
	{
		nyan_lastupdate = timeGetTime();
		init_nyantime = TRUE;
	}
	RECT srcRect;
	srcRect.top    = frame_no * 21;
	srcRect.left   = 0;
	srcRect.bottom = srcRect.top  + 21;
	srcRect.right  = 34;
	nyansprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_FRONTTOBACK);
	D3DXVECTOR2 pos( 200, 200 );
	D3DXVECTOR2 scaling(3.5f,3.5f);
	D3DXMATRIX spriteMatrix;
	D3DXMatrixTransformation2D(&spriteMatrix, 0, 0, &scaling, 0, 0, &pos);
	nyansprite->SetTransform(&spriteMatrix);
	//remove bilinear filtering
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	nyansprite->Draw(nyantexture,
		&srcRect,
		NULL,
		NULL,
		D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f) );
	nyansprite->End();

	if (timeNow-nyan_lastupdate>kTimeBetweenUpdated)
	{
		++frame_no;
		if( frame_no > 5 )
			frame_no = 0;
		nyan_lastupdate=timeNow;
	}
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,51,102), 1.0f, 0 );
	g_pd3dDevice->BeginScene();
	DrawStarfield();
	DrawRainbows();
    DrawNyan();
	g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

