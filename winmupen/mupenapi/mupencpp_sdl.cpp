#include "stdafx.h"
#include "mupencpp.h"
#include "osal_dynamiclib.h"
#include "osal_files.h"


#include "../SDL_keysym.h"

#include <windows.h>
#include <Shlwapi.h>
#include "../DropFileTarget.h"
#include "../DlgTabCtrl.h"
#include "../utf8conv.h"
#include <vector>

#include <gl/GL.h>
#include "../File_Extractor-master/fex/fex.h"

using namespace std;
using namespace utf8util;

typedef struct{
	BYTE scancode;
	SDLKey sym;
	SDLMod mod;
	WORD unicode;
} SDL_keysym;



/* definitions of pointers to Core front-end functions */

const char* DEFAULT_VIDEO_PLUGIN = "mupen64plus-video-rice.dll";
const char* DEFAULT_AUDIO_PLUGIN = "mupen64plus-audio-sdl.dll";
const char* DEFAULT_INPUT_PLUGIN = "mupen64plus-input-sdl.dll";
const char* DEFAULT_RSP_PLUGIN   = "mupen64plus-rsp-hle.dll";

#define CORE_API_VERSION   0x20001
#define CONFIG_API_VERSION 0x20000
#define CONFIG_PARAM_VERSION     1.00

#include <dinput.h>

LPDIRECTINPUT8       m_pDIObject;
LPDIRECTINPUTDEVICE8 m_pDIKeyboardDevice; //keyboard device (NEW)
static SDLKey DIK_keymap[256];
bool doublebuffer = true;
int buffersize = 32;
int depthsize = 24;
int redsize = 8;
int greensize = 8;
int bluesize = 8;
int alphasize = 8;
int width = 0;
int height = 0;
int bpp=0;
bool fullscreen = false;
LONG		windowedStyle;
LONG		windowedExStyle;
RECT		windowedRect;
HMENU		windowedMenu;
HDC	  g_hDC;
HGLRC g_hRC;


void build_keymap()
{
	int i;
	for ( i=0; i<ARRAYSIZE(DIK_keymap); ++i )
		DIK_keymap[i] = SDLK_UNKNOWN;
	/* Defined DIK_* constants */
	DIK_keymap[DIK_ESCAPE] = SDLK_ESCAPE;
	DIK_keymap[DIK_1] = SDLK_1;
	DIK_keymap[DIK_2] = SDLK_2;
	DIK_keymap[DIK_3] = SDLK_3;
	DIK_keymap[DIK_4] = SDLK_4;
	DIK_keymap[DIK_5] = SDLK_5;
	DIK_keymap[DIK_6] = SDLK_6;
	DIK_keymap[DIK_7] = SDLK_7;
	DIK_keymap[DIK_8] = SDLK_8;
	DIK_keymap[DIK_9] = SDLK_9;
	DIK_keymap[DIK_0] = SDLK_0;
	DIK_keymap[DIK_MINUS] = SDLK_MINUS;
	DIK_keymap[DIK_EQUALS] = SDLK_EQUALS;
	DIK_keymap[DIK_BACK] = SDLK_BACKSPACE;
	DIK_keymap[DIK_TAB] = SDLK_TAB;
	DIK_keymap[DIK_Q] = SDLK_q;
	DIK_keymap[DIK_W] = SDLK_w;
	DIK_keymap[DIK_E] = SDLK_e;
	DIK_keymap[DIK_R] = SDLK_r;
	DIK_keymap[DIK_T] = SDLK_t;
	DIK_keymap[DIK_Y] = SDLK_y;
	DIK_keymap[DIK_U] = SDLK_u;
	DIK_keymap[DIK_I] = SDLK_i;
	DIK_keymap[DIK_O] = SDLK_o;
	DIK_keymap[DIK_P] = SDLK_p;
	DIK_keymap[DIK_LBRACKET] = SDLK_LEFTBRACKET;
	DIK_keymap[DIK_RBRACKET] = SDLK_RIGHTBRACKET;
	DIK_keymap[DIK_RETURN] = SDLK_RETURN;
	DIK_keymap[DIK_LCONTROL] = SDLK_LCTRL;
	DIK_keymap[DIK_A] = SDLK_a;
	DIK_keymap[DIK_S] = SDLK_s;
	DIK_keymap[DIK_D] = SDLK_d;
	DIK_keymap[DIK_F] = SDLK_f;
	DIK_keymap[DIK_G] = SDLK_g;
	DIK_keymap[DIK_H] = SDLK_h;
	DIK_keymap[DIK_J] = SDLK_j;
	DIK_keymap[DIK_K] = SDLK_k;
	DIK_keymap[DIK_L] = SDLK_l;
	DIK_keymap[DIK_SEMICOLON] = SDLK_SEMICOLON;
	DIK_keymap[DIK_APOSTROPHE] = SDLK_QUOTE;
	DIK_keymap[DIK_GRAVE] = SDLK_BACKQUOTE;
	DIK_keymap[DIK_LSHIFT] = SDLK_LSHIFT;
	DIK_keymap[DIK_BACKSLASH] = SDLK_BACKSLASH;
	DIK_keymap[DIK_OEM_102] = SDLK_BACKSLASH;
	DIK_keymap[DIK_Z] = SDLK_z;
	DIK_keymap[DIK_X] = SDLK_x;
	DIK_keymap[DIK_C] = SDLK_c;
	DIK_keymap[DIK_V] = SDLK_v;
	DIK_keymap[DIK_B] = SDLK_b;
	DIK_keymap[DIK_N] = SDLK_n;
	DIK_keymap[DIK_M] = SDLK_m;
	DIK_keymap[DIK_COMMA] = SDLK_COMMA;
	DIK_keymap[DIK_PERIOD] = SDLK_PERIOD;
	DIK_keymap[DIK_SLASH] = SDLK_SLASH;
	DIK_keymap[DIK_RSHIFT] = SDLK_RSHIFT;
	DIK_keymap[DIK_MULTIPLY] = SDLK_KP_MULTIPLY;
	DIK_keymap[DIK_LMENU] = SDLK_LALT;
	DIK_keymap[DIK_SPACE] = SDLK_SPACE;
	DIK_keymap[DIK_CAPITAL] = SDLK_CAPSLOCK;
	DIK_keymap[DIK_F1] = SDLK_F1;
	DIK_keymap[DIK_F2] = SDLK_F2;
	DIK_keymap[DIK_F3] = SDLK_F3;
	DIK_keymap[DIK_F4] = SDLK_F4;
	DIK_keymap[DIK_F5] = SDLK_F5;
	DIK_keymap[DIK_F6] = SDLK_F6;
	DIK_keymap[DIK_F7] = SDLK_F7;
	DIK_keymap[DIK_F8] = SDLK_F8;
	DIK_keymap[DIK_F9] = SDLK_F9;
	DIK_keymap[DIK_F10] = SDLK_F10;
	DIK_keymap[DIK_NUMLOCK] = SDLK_NUMLOCK;
	DIK_keymap[DIK_SCROLL] = SDLK_SCROLLOCK;
	DIK_keymap[DIK_NUMPAD7] = SDLK_KP7;
	DIK_keymap[DIK_NUMPAD8] = SDLK_KP8;
	DIK_keymap[DIK_NUMPAD9] = SDLK_KP9;
	DIK_keymap[DIK_SUBTRACT] = SDLK_KP_MINUS;
	DIK_keymap[DIK_NUMPAD4] = SDLK_KP4;
	DIK_keymap[DIK_NUMPAD5] = SDLK_KP5;
	DIK_keymap[DIK_NUMPAD6] = SDLK_KP6;
	DIK_keymap[DIK_ADD] = SDLK_KP_PLUS;
	DIK_keymap[DIK_NUMPAD1] = SDLK_KP1;
	DIK_keymap[DIK_NUMPAD2] = SDLK_KP2;
	DIK_keymap[DIK_NUMPAD3] = SDLK_KP3;
	DIK_keymap[DIK_NUMPAD0] = SDLK_KP0;
	DIK_keymap[DIK_DECIMAL] = SDLK_KP_PERIOD;
	DIK_keymap[DIK_F11] = SDLK_F11;
	DIK_keymap[DIK_F12] = SDLK_F12;

	DIK_keymap[DIK_F13] = SDLK_F13;
	DIK_keymap[DIK_F14] = SDLK_F14;
	DIK_keymap[DIK_F15] = SDLK_F15;

	DIK_keymap[DIK_NUMPADEQUALS] = SDLK_KP_EQUALS;
	DIK_keymap[DIK_NUMPADENTER] = SDLK_KP_ENTER;
	DIK_keymap[DIK_RCONTROL] = SDLK_RCTRL;
	DIK_keymap[DIK_DIVIDE] = SDLK_KP_DIVIDE;
	DIK_keymap[DIK_SYSRQ] = SDLK_SYSREQ;
	DIK_keymap[DIK_RMENU] = SDLK_RALT;
	DIK_keymap[DIK_PAUSE] = SDLK_PAUSE;
	DIK_keymap[DIK_HOME] = SDLK_HOME;
	DIK_keymap[DIK_UP] = SDLK_UP;
	DIK_keymap[DIK_PRIOR] = SDLK_PAGEUP;
	DIK_keymap[DIK_LEFT] = SDLK_LEFT;
	DIK_keymap[DIK_RIGHT] = SDLK_RIGHT;
	DIK_keymap[DIK_END] = SDLK_END;
	DIK_keymap[DIK_DOWN] = SDLK_DOWN;
	DIK_keymap[DIK_NEXT] = SDLK_PAGEDOWN;
	DIK_keymap[DIK_INSERT] = SDLK_INSERT;
	DIK_keymap[DIK_DELETE] = SDLK_DELETE;
	DIK_keymap[DIK_LWIN] = SDLK_LMETA;
	DIK_keymap[DIK_RWIN] = SDLK_RMETA;
	DIK_keymap[DIK_APPS] = SDLK_MENU;
}


static SDL_keysym *TranslateKey(UINT scancode, SDL_keysym *keysym, int pressed)
{
	/* Set the keysym information */
	keysym->scancode = (unsigned char)scancode;
	keysym->sym = DIK_keymap[scancode];
	keysym->mod = KMOD_NONE;
	keysym->unicode = 0;
	return(keysym);
}


m64p_error VidExt_GL_SwapBuffers()
{
	CMupen64Plus* instance = CMupen64Plus::GetSingleton( ) ;
	SwapBuffers( g_hDC ); 
	BYTE    keys[256];
	build_keymap();
	
	// Get the input's device state, and put the state in keys - zero first
	ZeroMemory(keys, sizeof(keys) );
	HRESULT  result = m_pDIKeyboardDevice->GetDeviceState( sizeof(keys), keys );
			if ( (result == DIERR_INPUTLOST) ||
				(result == DIERR_NOTACQUIRED) ) {
				m_pDIKeyboardDevice->Acquire();
				m_pDIKeyboardDevice->Poll();
				 m_pDIKeyboardDevice->GetDeviceState( sizeof(keys), keys );
	}

	for(int i=0;i<256;i++)
	{
		if(keys[i] & 0x80)
		{
			SDL_keysym keysym;
			SDL_keysym *pressed =TranslateKey(i,&keysym,0);
			instance->CoreDoCommand(M64CMD_SEND_SDL_KEYDOWN,MAKELPARAM(pressed->sym,0), NULL);
		}
		else
		{
			SDL_keysym keysym;
			SDL_keysym *pressed =TranslateKey(i,&keysym,0);
			instance->CoreDoCommand(M64CMD_SEND_SDL_KEYUP,MAKELPARAM(pressed->sym,0), NULL);
		}


	}
	return M64ERR_SUCCESS;
}

m64p_error VidExt_ResizeWindow(int w, int h)
{
	// TODO for fullscreen
	CMupen64Plus* instance = CMupen64Plus::GetSingleton( ) ;
	if (!fullscreen)SetWindowPos(instance->emulator_hwnd,0,0,0,w,h+40,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	return M64ERR_SUCCESS;
}

void* VidExt_GL_GetProcAddress(const char* Proc)
{
	void* out = NULL;
	out = (void*)wglGetProcAddress(Proc);
	return out;
}



m64p_error VidExtFuncGLGetAttr(m64p_GLattr Attr, int* value)
{
	switch (Attr)
	{
	case M64P_GL_DOUBLEBUFFER:
		*value = doublebuffer;
		return M64ERR_SUCCESS;

	case M64P_GL_BUFFER_SIZE:
		*value = buffersize;
		return M64ERR_SUCCESS;

	case M64P_GL_DEPTH_SIZE:
		*value = depthsize;
		return M64ERR_SUCCESS;

	case M64P_GL_RED_SIZE:
		*value = redsize;
		return M64ERR_SUCCESS;

	case M64P_GL_GREEN_SIZE:
		*value = greensize;
		return M64ERR_SUCCESS;

	case M64P_GL_BLUE_SIZE:
		*value = bluesize;
		return M64ERR_SUCCESS;

	case M64P_GL_ALPHA_SIZE:
		*value = alphasize;
		return M64ERR_SUCCESS;

	case M64P_GL_SWAP_CONTROL:
		// FIXME: what's that?
		break;

	case M64P_GL_MULTISAMPLEBUFFERS:
		// FIXME: what's that?
		break;

	case M64P_GL_MULTISAMPLESAMPLES:
		// FIXME: what's that?
		break;
	}

	return M64ERR_INPUT_INVALID;
}

m64p_error VidExt_GL_SetAttribute(m64p_GLattr Attr, int Value)
{
	//printf(">>>>>>>>>>>> WX: VidExt_GL_SetAttribute\n");
	switch (Attr)
	{
	case M64P_GL_DOUBLEBUFFER:
		//printf("M64P_GL_DOUBLEBUFFER = %i\n", Value);
		doublebuffer = Value;
		break;

	case M64P_GL_BUFFER_SIZE:
		//printf("M64P_GL_BUFFER_SIZE = %i\n", Value);
		buffersize = Value;
		break;

	case M64P_GL_DEPTH_SIZE:
		//printf("M64P_GL_DEPTH_SIZE = %i\n", Value);
		depthsize = Value;
		break;

	case M64P_GL_RED_SIZE:
		//printf("M64P_GL_RED_SIZE = %i\n", Value);
		redsize = Value;
		break;

	case M64P_GL_GREEN_SIZE:
		//printf("M64P_GL_GREEN_SIZE = %i\n", Value);
		greensize = Value;
		break;

	case M64P_GL_BLUE_SIZE:
		//printf("M64P_GL_BLUE_SIZE = %i\n", Value);
		bluesize = Value;
		break;

	case M64P_GL_ALPHA_SIZE:
		//printf("M64P_GL_ALPHA_SIZE = %i\n", Value);
		alphasize = Value;
		break;

	case M64P_GL_SWAP_CONTROL:
		//printf("M64P_GL_SWAP_CONTROL = %i\n", Value);
		// FIXME: what's that?
		break;

	case M64P_GL_MULTISAMPLEBUFFERS:
		//printf("M64P_GL_MULTISAMPLEBUFFERS = %i\n", Value);
		// FIXME: what's that?
		break;

	case M64P_GL_MULTISAMPLESAMPLES:
		//printf("M64P_GL_MULTISAMPLESAMPLES = %i\n", Value);
		// FIXME: what's that?
		break;
	}

	return M64ERR_SUCCESS;
}

m64p_error VidExt_Init()
{
	m_pDIObject = NULL;
	m_pDIKeyboardDevice = NULL;
	DirectInput8Create(GetModuleHandle(NULL),DIRECTINPUT_VERSION,IID_IDirectInput8,(void**)&m_pDIObject,NULL);
	return M64ERR_SUCCESS;
}

m64p_error VidExt_Quit()
{
	if (m_pDIKeyboardDevice)m_pDIKeyboardDevice->Unacquire();
	if (m_pDIKeyboardDevice)m_pDIKeyboardDevice->Release();
	if (m_pDIKeyboardDevice)m_pDIKeyboardDevice = NULL;
	CMupen64Plus* instance = CMupen64Plus::GetSingleton( ) ;

	if (fullscreen)                      // if in fullscreen..
	{
		ChangeDisplaySettings(NULL, 0);   // switch back to desktop..
		if (windowedMenu)
			SetMenu( instance->emulator_hwnd, windowedMenu );

		SetWindowLong( instance->emulator_hwnd, GWL_STYLE, windowedStyle );
		SetWindowLong( instance->emulator_hwnd, GWL_EXSTYLE, windowedExStyle );
		SetWindowPos( instance->emulator_hwnd, NULL, windowedRect.left, windowedRect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
		fullscreen = false;
		ShowCursor( TRUE );
	}

	if( g_hRC != NULL )
	{
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( g_hRC );
		g_hRC = NULL;
	}

	if( g_hDC != NULL )
	{
		ReleaseDC( instance->emulator_hwnd, g_hDC );
		g_hDC = NULL;
	}
    return M64ERR_SUCCESS;
}

m64p_error VidExt_ListFullscreenModes(m64p_2d_size *SizeArray, int *NumSizes)
{
	// TODO: do better!!
	static m64p_2d_size s[9] =
	{
		{ 640, 480},
		{ 800, 600},
		{ 960, 720},
		{ 1024, 768},
		{ 1152, 864},
		{ 1280, 960},
		{ 1280, 1024 },
		{ 1440, 1080 },
		{ 1600, 1200 }
	};
	SizeArray = s;
	*NumSizes = 9;
	return M64ERR_SUCCESS;
}

m64p_error VidExt_SetCaption(const char *Title)
{
    return M64ERR_SUCCESS;
}

m64p_error VidExt_ToggleFullScreen()
{

	CMupen64Plus* instance = CMupen64Plus::GetSingleton( ) ;
	if (!fullscreen)                      // if in fullscreen..
	{
		DEVMODE fullscreenMode;
		memset( &fullscreenMode, 0, sizeof(DEVMODE) );
		fullscreenMode.dmSize = sizeof(DEVMODE);
		fullscreenMode.dmPelsWidth			= width;
		fullscreenMode.dmPelsHeight			= height;
		fullscreenMode.dmBitsPerPel			= bpp;
		fullscreenMode.dmDisplayFrequency	= 60;
		fullscreenMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
		if (ChangeDisplaySettings( &fullscreenMode, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL)
		{
			MessageBox( NULL, L"Failed to change display mode", L"ERROR", MB_ICONERROR | MB_OK );
			return M64ERR_INVALID_STATE;
		}
		ShowCursor( FALSE );
		windowedMenu = GetMenu( instance->emulator_hwnd );
		if (windowedMenu)
			SetMenu(  instance->emulator_hwnd, NULL );
		windowedExStyle = GetWindowLong(  instance->emulator_hwnd, GWL_EXSTYLE );
		windowedStyle = GetWindowLong(  instance->emulator_hwnd, GWL_STYLE );
		SetWindowLong(  instance->emulator_hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST );
		SetWindowLong(  instance->emulator_hwnd, GWL_STYLE, WS_POPUP );
		GetWindowRect(  instance->emulator_hwnd, &windowedRect );
		SetWindowPos( instance->emulator_hwnd, NULL, 0, 0,	width,height, SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW );
		fullscreen = true;
	}
	else
	{
		RECT windowRect;
		ChangeDisplaySettings( NULL, 0 );
		ShowCursor( TRUE );
		if (windowedMenu)
			SetMenu( instance->emulator_hwnd, windowedMenu );
		SetWindowLong(instance->emulator_hwnd, GWL_STYLE, windowedStyle );
		SetWindowLong( instance->emulator_hwnd, GWL_EXSTYLE, windowedExStyle );
		SetWindowPos( instance->emulator_hwnd, NULL, windowedRect.left, windowedRect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
		GetClientRect(instance->emulator_hwnd, &windowRect );
		windowRect.right = windowRect.left + width - 1;
		windowRect.bottom = windowRect.top + height - 1 + 40;
		AdjustWindowRect( &windowRect, GetWindowLong( instance->emulator_hwnd, GWL_STYLE ), GetMenu( instance->emulator_hwnd ) != NULL );
		SetWindowPos( instance->emulator_hwnd, NULL, 0, 0,	windowRect.right - windowRect.left + 1,
			windowRect.bottom - windowRect.top + 1, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE );
		fullscreen = false;

	}
	return M64ERR_SUCCESS;
}

m64p_error VidExt_SetVideoMode(int Width, int Height, int BitsPerPixel,
	/*m64p_video_mode*/ int ScreenMode,
	/*m64p_video_flags*/ int Flags)
{

	CMupen64Plus* instance = CMupen64Plus::GetSingleton( ) ;
	if ((Flags &  M64VIDEOFLAG_SUPPORT_RESIZING) == M64VIDEOFLAG_SUPPORT_RESIZING)
	{
		VidExt_Quit();
		m_pDIObject = NULL;
		m_pDIKeyboardDevice = NULL;
		DirectInput8Create(GetModuleHandle(NULL),DIRECTINPUT_VERSION,IID_IDirectInput8,(void**)&m_pDIObject,NULL);
	}
	g_hDC  = NULL;
	g_hRC  = NULL;

	width = Width;
	height = Height;

	m_pDIObject->CreateDevice(GUID_SysKeyboard,&m_pDIKeyboardDevice,NULL);
	m_pDIKeyboardDevice->SetDataFormat(&c_dfDIKeyboard);
	m_pDIKeyboardDevice->SetCooperativeLevel(instance->emulator_hwnd,DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	m_pDIKeyboardDevice->Acquire();
	if(BitsPerPixel ==0)BitsPerPixel=32;
	bpp=BitsPerPixel;
	DWORD flags = PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL;	
	if(doublebuffer)flags |=PFD_DOUBLEBUFFER;
	
	PIXELFORMATDESCRIPTOR pfd = { 
		sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd 
		1,                                // version number 
		flags,                 // double buffered 
		PFD_TYPE_RGBA,                    // RGBA type 
	    BitsPerPixel,								  // color depth 
		0, 0, 0, 0, 0, 0,                 // color bits ignored 
		8,                                // no alpha buffer 
		0,                                // shift bit ignored 
		0,                                // no accumulation buffer 
		0, 0, 0, 0,                       // accum bits ignored 
		depthsize,						  // z-buffer     
		0,                                // no stencil buffer 
		0,                                // no auxiliary buffer 
		PFD_MAIN_PLANE,                   // main layer 
		0,                                // reserved 
		0, 0, 0                           // layer masks ignored 
	};


	if (ScreenMode == M64VIDEO_FULLSCREEN)   // try full screen if fullscreen is true
	{
		DEVMODE fullscreenMode;
		memset( &fullscreenMode, 0, sizeof(DEVMODE) );
		fullscreenMode.dmSize = sizeof(DEVMODE);
		fullscreenMode.dmPelsWidth			= Width;
		fullscreenMode.dmPelsHeight			= Height;
		fullscreenMode.dmBitsPerPel			= BitsPerPixel;
		fullscreenMode.dmDisplayFrequency	= 60;
		fullscreenMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

		if (ChangeDisplaySettings( &fullscreenMode, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL)
		{
			MessageBox( NULL, L"Failed to change display mode", L"ERROR", MB_ICONERROR | MB_OK );
			return M64ERR_UNSUPPORTED;
		}

		windowedMenu = GetMenu( instance->emulator_hwnd );
		if (windowedMenu)SetMenu( instance->emulator_hwnd, NULL );
		windowedExStyle = GetWindowLong( instance->emulator_hwnd, GWL_EXSTYLE );
		windowedStyle = GetWindowLong( instance->emulator_hwnd, GWL_STYLE );
		SetWindowLong( instance->emulator_hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST );
		SetWindowLong( instance->emulator_hwnd, GWL_STYLE, WS_POPUP );
		GetWindowRect( instance->emulator_hwnd, &windowedRect );
		SetWindowPos( instance->emulator_hwnd, NULL, 0, 0,	Width,Height, SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW );
		ShowCursor( FALSE );
		fullscreen = true;
	}
	else
	{
		SetWindowPos(instance->emulator_hwnd,0,0,0,Width,Height+40,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
		fullscreen = false;
	}


	g_hDC = GetDC( instance->emulator_hwnd );
	GLuint iPixelFormat = ChoosePixelFormat( g_hDC, &pfd );
	SetPixelFormat( g_hDC, iPixelFormat, &pfd);
	g_hRC = wglCreateContext(g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );   
	glViewport(0, 0, Width, Height);
	
	// clear both buffers with black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	SwapBuffers(g_hDC);

	if ((Flags &  M64VIDEOFLAG_SUPPORT_RESIZING) == M64VIDEOFLAG_SUPPORT_RESIZING)
	{
		instance->CoreDoCommand( M64CMD_CORE_STATE_SET,M64CORE_VIDEO_SIZE,(int*)(width << 16) + height);
	}
	
	
	return M64ERR_SUCCESS;
}

m64p_error CMupen64Plus::AttachCoreLib(const char *CoreLibFilepath)
{
	/* check if Core DLL is already attached */
	if (CoreHandle != NULL)
		return M64ERR_INVALID_STATE;
	/* load the DLL */
	m64p_error rval = M64ERR_INTERNAL;
	/* first, try a library path+name that was given on the command-line */
	if (CoreLibFilepath != NULL)
	{
		rval = osal_dynlib_open(&CoreHandle, CoreLibFilepath);
	}
	/* then try a library path that was given at compile time */
	/* if we haven't found a good core library by now, then we're screwed */
	if (rval != M64ERR_SUCCESS || CoreHandle == NULL)
	{
		CoreHandle = NULL;
		return M64ERR_INPUT_NOT_FOUND;
	}
	// attach and call the PluginGetVersion function, check the Core and API versions
	// for compatibility with this front-end
	ptr_PluginGetVersion CoreVersionFunc;
	CoreVersionFunc = (ptr_PluginGetVersion) osal_dynlib_getproc(CoreHandle, "PluginGetVersion");
	if (CoreVersionFunc == NULL)
	{
		osal_dynlib_close(CoreHandle);
		CoreHandle = NULL;
		return M64ERR_INPUT_INVALID;
	}
	/* attach and call the CoreGetAPIVersion function, check Config API version for compatibility */
	ptr_CoreGetAPIVersions CoreAPIVersionFunc;
	CoreAPIVersionFunc = (ptr_CoreGetAPIVersions) osal_dynlib_getproc(CoreHandle, "CoreGetAPIVersions");
	if (CoreAPIVersionFunc == NULL)
	{
		fprintf(stderr, "AttachCoreLib() Error: Library '%s' broken; no CoreAPIVersionFunc() function found.\n",
			CoreLibFilepath);
		osal_dynlib_close(CoreHandle);
		CoreHandle = NULL;
		return M64ERR_INPUT_INVALID;
	}
	int ConfigAPIVersion, DebugAPIVersion, VidextAPIVersion;
	(*CoreAPIVersionFunc)(&ConfigAPIVersion, &DebugAPIVersion, &VidextAPIVersion, NULL);
	if ((ConfigAPIVersion & 0xffff0000) != (CONFIG_API_VERSION & 0xffff0000))
	{
		osal_dynlib_close(CoreHandle);
		CoreHandle = NULL;
		return M64ERR_INCOMPATIBLE;
	}
	// get function pointers to the common and front-end functions
	CoreStartup        = (ptr_CoreStartup) osal_dynlib_getproc(CoreHandle, "CoreStartup");
	CoreShutdown       = (ptr_CoreShutdown) osal_dynlib_getproc(CoreHandle, "CoreShutdown");
	CoreAttachPlugin   = (ptr_CoreAttachPlugin) osal_dynlib_getproc(CoreHandle, "CoreAttachPlugin");
	CoreDetachPlugin   = (ptr_CoreDetachPlugin) osal_dynlib_getproc(CoreHandle, "CoreDetachPlugin");
	CoreDoCommand      = (ptr_CoreDoCommand) osal_dynlib_getproc(CoreHandle, "CoreDoCommand");
	CoreOverrideVidExt = (ptr_CoreOverrideVidExt) osal_dynlib_getproc(CoreHandle, "CoreOverrideVidExt");
	CoreAddCheat       = (ptr_CoreAddCheat) osal_dynlib_getproc(CoreHandle, "CoreAddCheat");
	CoreCheatEnabled   = (ptr_CoreCheatEnabled) osal_dynlib_getproc(CoreHandle, "CoreCheatEnabled");
	// get function pointers to the configuration functions
	ConfigListSections = (ptr_ConfigListSections) osal_dynlib_getproc(CoreHandle, "ConfigListSections");
	ConfigOpenSection = (ptr_ConfigOpenSection) osal_dynlib_getproc(CoreHandle, "ConfigOpenSection");
	ConfigDeleteSection = (ptr_ConfigDeleteSection) osal_dynlib_getproc(CoreHandle, "ConfigDeleteSection");
	ConfigSaveSection = (ptr_ConfigSaveSection) osal_dynlib_getproc(CoreHandle, "ConfigSaveSection");
	ConfigListParameters = (ptr_ConfigListParameters) osal_dynlib_getproc(CoreHandle, "ConfigListParameters");
	ConfigSaveFile = (ptr_ConfigSaveFile) osal_dynlib_getproc(CoreHandle, "ConfigSaveFile");
	ConfigSetParameter = (ptr_ConfigSetParameter) osal_dynlib_getproc(CoreHandle, "ConfigSetParameter");
	ConfigGetParameter = (ptr_ConfigGetParameter) osal_dynlib_getproc(CoreHandle, "ConfigGetParameter");
	ConfigGetParameterType = (ptr_ConfigGetParameterType) osal_dynlib_getproc(CoreHandle, "ConfigGetParameterType");
	ConfigGetParameterHelp = (ptr_ConfigGetParameterHelp) osal_dynlib_getproc(CoreHandle, "ConfigGetParameterHelp");
	ConfigSetDefaultInt = (ptr_ConfigSetDefaultInt) osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultInt");
	ConfigSetDefaultFloat = (ptr_ConfigSetDefaultFloat) osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultFloat");
	ConfigSetDefaultBool = (ptr_ConfigSetDefaultBool) osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultBool");
	ConfigSetDefaultString = (ptr_ConfigSetDefaultString) osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultString");
	ConfigGetParamInt = (ptr_ConfigGetParamInt) osal_dynlib_getproc(CoreHandle, "ConfigGetParamInt");
	ConfigGetParamFloat = (ptr_ConfigGetParamFloat) osal_dynlib_getproc(CoreHandle, "ConfigGetParamFloat");
	ConfigGetParamBool = (ptr_ConfigGetParamBool) osal_dynlib_getproc(CoreHandle, "ConfigGetParamBool");
	ConfigGetParamString = (ptr_ConfigGetParamString) osal_dynlib_getproc(CoreHandle, "ConfigGetParamString");
	ConfigGetSharedDataFilepath = (ptr_ConfigGetSharedDataFilepath) osal_dynlib_getproc(CoreHandle, "ConfigGetSharedDataFilepath");
	ConfigGetUserConfigPath     = (ptr_ConfigGetUserConfigPath)     osal_dynlib_getproc(CoreHandle, "ConfigGetUserConfigPath");
	ConfigGetUserDataPath       = (ptr_ConfigGetUserDataPath)       osal_dynlib_getproc(CoreHandle, "ConfigGetUserDataPath");
	ConfigGetUserCachePath      = (ptr_ConfigGetUserCachePath)      osal_dynlib_getproc(CoreHandle, "ConfigGetUserCachePath");
	
	return M64ERR_SUCCESS;
}

m64p_error CMupen64Plus::DetachCoreLib(void)
{
	if (CoreHandle == NULL)
		return M64ERR_INVALID_STATE;

	/* set the core function pointers to NULL */
	CoreStartup = NULL;
	CoreShutdown = NULL;
	CoreAttachPlugin = NULL;
	CoreDetachPlugin = NULL;
	CoreDoCommand = NULL;
	CoreOverrideVidExt = NULL;
	CoreAddCheat = NULL;
	CoreCheatEnabled = NULL;

	ConfigListSections = NULL;
	ConfigOpenSection = NULL;
	ConfigDeleteSection = NULL;
	ConfigSaveSection = NULL;
	ConfigListParameters = NULL;
	ConfigSetParameter = NULL;
	ConfigGetParameter = NULL;
	ConfigGetParameterType = NULL;
	ConfigGetParameterHelp = NULL;
	ConfigSetDefaultInt = NULL;
	ConfigSetDefaultBool = NULL;
	ConfigSetDefaultString = NULL;
	ConfigGetParamInt = NULL;
	ConfigGetParamBool = NULL;
	ConfigGetParamString = NULL;

	ConfigGetSharedDataFilepath = NULL;
	ConfigGetUserDataPath = NULL;
	ConfigGetUserCachePath = NULL;

	/* detach the shared library */
	osal_dynlib_close(CoreHandle);
	CoreHandle = NULL;

	return M64ERR_SUCCESS;
}

CMupen64Plus* CMupen64Plus::m_Instance = 0 ;
CMupen64Plus* CMupen64Plus::CreateInstance()
{
	if (0 == m_Instance)
	{
		m_Instance = new CMupen64Plus( ) ;
	}
	return m_Instance ;
}

CMupen64Plus* CMupen64Plus::GetSingleton( )
{
	return m_Instance ;
}


bool CMupen64Plus::init(HWND hwnd)
{
	isEmulating = false;
	emulator_hwnd = hwnd;
	TCHAR corepath[512] = {0};
	TCHAR coredllpath[512] = {0};
	// Will contain exe path
	HMODULE hModule = GetModuleHandle(NULL);
	if (hModule != NULL)
	{
		// When passing NULL to GetModuleHandle, it returns handle of exe itself
		GetModuleFileName(hModule,corepath, (sizeof(corepath))); 
		PathRemoveFileSpec(corepath);
	}
	lstrcat(coredllpath,corepath);
	lstrcat(coredllpath,L"\\mupen64plus.dll");
	
	m64p_error err = AttachCoreLib((const char*)coredllpath);
	if (err != M64ERR_SUCCESS)return false;
	string core_str = utf8_from_utf16(corepath);
	m64p_error rval = (*CoreStartup)(CORE_API_VERSION, (const char*)core_str.c_str(), (const char*)core_str.c_str(), "Core", NULL, NULL, NULL);
	if (rval != M64ERR_SUCCESS)
	{
		DetachCoreLib();
		return false;
	}
	open_confighandles();
	return true;
}

 bool CMupen64Plus::open_confighandles()
 {
	 float fConfigParamsVersion;
	 int bSaveConfig = 0;
	 m64p_error rval;

	 /* Open Configuration sections for core library and console User Interface */
	 rval = (*ConfigOpenSection)("Core", &l_ConfigCore);
	 if (rval != M64ERR_SUCCESS)
	 {
		 return rval;
	 }
	 rval = (*ConfigOpenSection)("Video-General", &l_ConfigVideo);
	 if (rval != M64ERR_SUCCESS)
	 {
		 return rval;
	 }
	 rval = (*ConfigOpenSection)("WinMupen", &l_ConfigUI);
	 if (rval != M64ERR_SUCCESS)
	 {
		 return rval;
	 } 
	 if ((*ConfigGetParameter)(l_ConfigUI, "Version", M64TYPE_FLOAT, &fConfigParamsVersion, sizeof(float)) != M64ERR_SUCCESS)
	 {
		  rval = (*ConfigDeleteSection)("WinMupen");
		  rval = (*ConfigOpenSection)("WinMupen", &l_ConfigUI);
		 bSaveConfig = 1;
	 }
	 else if (((int) fConfigParamsVersion) != ((int) CONFIG_PARAM_VERSION))
	 {
		 (*ConfigDeleteSection)("WinMupen");
		 (*ConfigOpenSection)("WinMupen", &l_ConfigUI);
		 bSaveConfig = 1;
	 }
	 else if ((CONFIG_PARAM_VERSION - fConfigParamsVersion) >= 0.0001f)
	 {
		 /* handle upgrades */
		 float fVersion = CONFIG_PARAM_VERSION;
		 ConfigSetParameter(l_ConfigUI, "Version", M64TYPE_FLOAT, &fVersion);
		 bSaveConfig = 1;
	 }


	 TCHAR dllpath[512] = {0};
	 TCHAR rompath[512] = {0};
	 HMODULE hModule = GetModuleHandle(NULL);
	 if (hModule != NULL)
	 {
		 // When passing NULL to GetModuleHandle, it returns handle of exe itself
		 GetModuleFileName(hModule,dllpath, (sizeof(dllpath))); 
		 PathRemoveFileSpec(dllpath);
	 }
	 lstrcpy(rompath,dllpath);
	 lstrcat(rompath,L"\\roms");
	 string plugin_dir = utf8_from_utf16(dllpath);
	 string rom_dir = utf8_from_utf16(rompath);



	 /* Set default values for my Config parameters */
	  (*ConfigSetDefaultFloat)(l_ConfigUI, "Version", CONFIG_PARAM_VERSION,  "WinMupen config parameter set version number.  Please don't change this version number.");
	  rval =(*ConfigSetDefaultString)(l_ConfigUI, "PluginDir", plugin_dir.c_str(), "Directory in which to search for plugins");
	  rval =(*ConfigSetDefaultString)(l_ConfigUI, "RomDir", rom_dir.c_str(), "Directory in which to search for ROMs");
	  rval =(*ConfigSetDefaultString)(l_ConfigUI, "VideoPlugin", "mupen64plus-video-glide64mk2" OSAL_DLL_EXTENSION, "Filename of video plugin");
	  rval =(*ConfigSetDefaultString)(l_ConfigUI, "AudioPlugin", "mupen64plus-audio-sdl" OSAL_DLL_EXTENSION, "Filename of audio plugin");
	  rval =(*ConfigSetDefaultString)(l_ConfigUI, "InputPlugin", "mupen64plus-input-sdl" OSAL_DLL_EXTENSION, "Filename of input plugin");
	  rval =(*ConfigSetDefaultString)(l_ConfigUI, "RspPlugin", "mupen64plus-rsp-hle" OSAL_DLL_EXTENSION, "Filename of RSP plugin");
	 if (bSaveConfig && ConfigSaveSection != NULL) /* ConfigSaveSection was added in Config API v2.1.0 */
	 rval = (*ConfigSaveFile)();

 }

 bool CMupen64Plus::load_plugin(char *filename, int index)
 {
	 /* try to open a shared library at the given filepath */
	 m64p_dynlib_handle handle;
	 m64p_error rval = osal_dynlib_open(&handle, filename);
	 if (rval != M64ERR_SUCCESS)
		 return rval;
	 /* call the GetVersion function for the plugin and check compatibility */
	 ptr_PluginGetVersion PluginGetVersion = (ptr_PluginGetVersion) osal_dynlib_getproc(handle, "PluginGetVersion");
	 if (PluginGetVersion == NULL)
	 {
		 osal_dynlib_close(handle);
		 return false;
	 }
	 m64p_plugin_type PluginType = (m64p_plugin_type) 0;
	 int PluginVersion = 0;
	 const char *PluginName = NULL;
	 (*PluginGetVersion)(&PluginType, &PluginVersion, NULL, &PluginName, NULL);
	 if (PluginType != g_PluginMap[index].type)
	 {
		 /* the type of this plugin doesn't match with the type that was requested by the caller */
		 osal_dynlib_close(handle);
		 return false;
	 }
	 /* call the plugin's initialization function and make sure it starts okay */
	 ptr_PluginStartup PluginStartup = (ptr_PluginStartup) osal_dynlib_getproc(handle, "PluginStartup");
	 if (PluginStartup == NULL)
	 {
		 osal_dynlib_close(handle);
		 return false;
	 }
	 rval = (*PluginStartup)(CoreHandle, g_PluginMap[index].name, NULL);  /* DebugCallback is in main.c */
	 if (rval != M64ERR_SUCCESS)
	 {
		 osal_dynlib_close(handle);
		 return false;
	 }
	 /* plugin loaded successfully, so set the plugin map's members */
	 g_PluginMap[index].handle = handle;
	 lstrcpy(g_PluginMap[index].filename, (TCHAR*)filename);
	 strcpy(g_PluginMap[index].libname,PluginName);
	 g_PluginMap[index].libversion = PluginVersion;
	 return true;
 }

 bool CMupen64Plus::load_plugins()
 {
	 //g_PluginMap[0].type = M64PLUGIN_GFX;
	 plugin_map_node PluginMap[]	 = {
	     { M64PLUGIN_GFX,   "Video", NULL, NULL, NULL, 0 },
		 {M64PLUGIN_AUDIO, "Audio", NULL, NULL, NULL, 0 },
		 {M64PLUGIN_INPUT, "Input", NULL, NULL, NULL, 0 },
		 {M64PLUGIN_RSP,   "RSP",   NULL, NULL, NULL, 0 }
     };

	 for (int i=0;i<4;i++)
	 {
		 g_PluginMap[i] = PluginMap[i];
		 m64p_plugin_type type = g_PluginMap[i].type;

		 const char      *cmdline_path = NULL;
		 const char      *config_var = NULL;
		 int              use_dummy = 0;

		 switch (type)
		 {
		 case M64PLUGIN_RSP:    config_var = "RspPlugin";   break;
		 case M64PLUGIN_GFX:    config_var = "VideoPlugin"; break;
		 case M64PLUGIN_AUDIO:  config_var = "AudioPlugin"; break;
		 case M64PLUGIN_INPUT:  config_var = "InputPlugin"; break;
		 default:               cmdline_path = NULL;           config_var = "";
		 }
		 const char *plugindir = (*ConfigGetParamString)(l_ConfigUI, "PluginDir");
		 const char *config_path = (*ConfigGetParamString)(l_ConfigUI, config_var);
		 string plugin_path = plugindir;
		 plugin_path += "\\";
		 plugin_path +=config_path;
		 wstring path = utf16_from_utf8(plugin_path);
		 load_plugin((char*)path.c_str(),i);
		 if ((*CoreAttachPlugin)(g_PluginMap[i].type, g_PluginMap[i].handle) != M64ERR_SUCCESS)
		 {
			 (*CoreDoCommand)(M64CMD_ROM_CLOSE, 0, NULL);
			 (*CoreShutdown)();
			 DetachCoreLib();
			 return false;
		 }
	 }

	 loadedplugins = true;

	 return loadedplugins;
 }
#include "../rombrowser.h"
 DWORD EmulatorThread(LPVOID lpdwThreadParam )
 {
	 CMupen64Plus* instance = CMupen64Plus::GetSingleton( ) ;
	 CRomBrowser* browser = CRomBrowser::GetSingleton( ) ;
	  if(!instance->use_videoext)
	  {
		browser->show();
	  }
	  else
	  {
		  browser->hide();
	  }
	 m64p_error retval= instance->CoreDoCommand(M64CMD_EXECUTE, 0, NULL);
	 instance->isEmulating = false;
	 instance->close_rom();
	
	 SetWindowPos(instance->emulator_hwnd,0,0,0,640,480,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	 browser->show();
	 return 0;
 }

 m64p_error CMupen64Plus::getRomHeader(TCHAR* path, m64p_rom_header* out)
 {
	 TCHAR * exts[] = { L".z64", L".v64", L".n64", L".zip" };
	 TCHAR* ext = PathFindExtension(path);
	 if (!lstrcmp(ext,L".zip"))
	 {
		 fex_t* fex;
		 const char* path_utf8 = fex_wide_to_path(path);
		 fex_err_t err = fex_open( &fex, path_utf8 );
		 while ( !fex_done( fex ) )
		 {
			 if ( fex_has_extension( fex_name( fex ), ".z64" )||fex_has_extension( fex_name( fex ), ".v64" )
				 ||fex_has_extension( fex_name( fex ), ".n64" ) )
			 {
				 fex_stat( fex );
				 fex_read( fex, out, sizeof(m64p_rom_header) ); 
				 fex_close( fex );
				 return M64ERR_SUCCESS;
				 break;
			 }
			 fex_next( fex );
		 }
		 fex_close( fex );
		 return M64ERR_FILES;
	 }
	 else
	 {
		 HANDLE hFile = CreateFile(path,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
		 if (!hFile) return M64ERR_FILES;
		 DWORD count;
		 ReadFile(hFile,out,sizeof(m64p_rom_header),&count,NULL);
		 if (count == 0)
		 {
			 CloseHandle(hFile);
			 return M64ERR_FILES;
		 }
		 CloseHandle(hFile);
		 return M64ERR_SUCCESS;
	 }
	
 }

 bool CMupen64Plus::IsValidExt(TCHAR* filename)
 {
	 TCHAR * exts[] = { L".z64", L".v64", L".n64", L".zip" };
	 TCHAR* ext = PathFindExtension(filename);
	 for ( unsigned n = 0; n < ARRAYSIZE( exts ); ++n )
	 {
		 if ( ! lstrcmp( ext, exts[ n ] ) ) return true;
	 }
	 return false;
 }

 bool CMupen64Plus::IsValidROM(TCHAR * filename)
 {
	 if (!IsValidExt(filename))return false;
	 CMupen64Plus *emulator_instance = CMupen64Plus::GetSingleton();
	 const char *rom_dir = emulator_instance->ConfigGetParamString( emulator_instance->l_ConfigUI, "RomDir");
	 wstring rom_w = utf16_from_utf8(rom_dir);
	 m64p_rom_header header;
	 wstring rompath;
	 rompath = rom_w.c_str();
	 rompath += L"\\";
	 rompath += filename;
	 m64p_error result = getRomHeader((TCHAR*)rompath.c_str(), &header);
	 if (result != M64ERR_SUCCESS)
	 {
		 return false;
	 }
	 return true;
 }

bool CMupen64Plus::run_rom(const char *filename)
{                                          
	if (isEmulating) close_rom();

	wstring str = utf16_from_utf8(filename);
	unsigned char *ROM_buffer;

	int vidext = ConfigGetParamBool(l_ConfigUI,"UseVideoExtension");
	use_videoext = vidext;
	if(vidext)
	{
		m64p_video_extension_functions f;
		f.Functions            = 0xFFFFFFFF; // FIXME: what's this field? it's not documented
		f.VidExtFuncGLGetProc  = &VidExt_GL_GetProcAddress;
		f.VidExtFuncGLSetAttr  = &VidExt_GL_SetAttribute;
		f.VidExtFuncGLSwapBuf  = &VidExt_GL_SwapBuffers;
		f.VidExtFuncInit       = &VidExt_Init;
		f.VidExtFuncListModes  = &VidExt_ListFullscreenModes;
		f.VidExtFuncQuit       = &VidExt_Quit;
		f.VidExtFuncSetCaption = &VidExt_SetCaption;
		f.VidExtFuncSetMode    = &VidExt_SetVideoMode;
		f.VidExtFuncGLGetAttr  = &VidExtFuncGLGetAttr;
		f.VidExtFuncToggleFS   = &VidExt_ToggleFullScreen;
		f.VidExtFuncResizeWindow = &VidExt_ResizeWindow;
		m64p_error ret = (*CoreOverrideVidExt)(&f);
	}
	else
	{
		m64p_video_extension_functions f;
		f.Functions            = 0xFFFFFFFF; // FIXME: what's this field? it's not documented
		f.VidExtFuncGLGetProc  = NULL;
		f.VidExtFuncGLSetAttr  = NULL;
		f.VidExtFuncGLSwapBuf  = NULL;
		f.VidExtFuncInit       =NULL;
		f.VidExtFuncListModes  = NULL;
		f.VidExtFuncQuit       = NULL;
		f.VidExtFuncSetCaption = NULL;
		f.VidExtFuncSetMode    = NULL;
		f.VidExtFuncGLGetAttr  =NULL;
		f.VidExtFuncToggleFS   = NULL;
		f.VidExtFuncResizeWindow =NULL;
		m64p_error ret = (*CoreOverrideVidExt)(&f);
	}


	TCHAR* ext = PathFindExtension(str.c_str());
	if (!lstrcmp(ext,L".zip"))
	{
		fex_t* fex;
		fex_err_t err = fex_open( &fex, filename );
		while ( !fex_done( fex ) )
		{
			if ( fex_has_extension( fex_name( fex ), ".z64" )||fex_has_extension( fex_name( fex ), ".v64" )
				||fex_has_extension( fex_name( fex ), ". n64" ) )
			{
				fex_stat( fex );
				int size = fex_size( fex );
				ROM_buffer = (unsigned char *) malloc(size);
				fex_read( fex, ROM_buffer, size ); 
				if ((*CoreDoCommand)(M64CMD_ROM_OPEN, (int)size, ROM_buffer) != M64ERR_SUCCESS)
				{
					fex_close(fex);
					free(ROM_buffer);
					(*CoreShutdown)();
					DetachCoreLib();
					return false;
				}
				break;
			}
			fex_next( fex );
		}
		fex_close( fex );
	}
	else
	{
		HANDLE hFile = CreateFile(str.c_str(),GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return false;
		}
		LARGE_INTEGER size;
		if (!GetFileSizeEx(hFile, &size)) return false;
		ROM_buffer = (unsigned char *) malloc(size.QuadPart);

		DWORD dwRead;
		int Entries = 0;
		ReadFile(hFile,ROM_buffer,size.QuadPart,&dwRead,NULL);

		if ((*CoreDoCommand)(M64CMD_ROM_OPEN, (int) size.QuadPart, ROM_buffer) != M64ERR_SUCCESS)
		{
			free(ROM_buffer);
			(*CoreShutdown)();
			DetachCoreLib();
			return false;
		}
		CloseHandle(hFile);
	}
	free(ROM_buffer);

	if (!loadedplugins)load_plugins();
	isEmulating = true;
	threadID = 0;
	if (use_videoext)
	{
		CreateThread(NULL, //Choose default security
			0, //Default stack size
			(LPTHREAD_START_ROUTINE)EmulatorThread,
			//Routine to execute
			(LPVOID)0, //Thread parameter
			0, //Immediately run the thread
			&threadID); 
		
	}
	else
	{
	  CoreDoCommand(M64CMD_EXECUTE, 0, NULL);
	  close_rom();
	  
	}
	
	
	

	

	return false;
}

void CMupen64Plus::close_rom()
{
	if(use_videoext)
	{
		m64p_error retval = CoreDoCommand(M64CMD_STOP, 0, NULL);
		while (isEmulating) { Sleep(10);}
		TerminateThread(&threadID,0);
	}
	
    for (int i = 0; i < 4; i++)(*CoreDetachPlugin)(g_PluginMap[i].type);
	typedef m64p_error (*ptr_PluginShutdown)(void);
	ptr_PluginShutdown PluginShutdown;
	int i;

	/* shutdown each type of plugin */
	for (i = 0; i < 4; i++)
	{
		if (g_PluginMap[i].handle == NULL)
			continue;
		/* call the destructor function for the plugin and release the library */
		PluginShutdown = (ptr_PluginShutdown) osal_dynlib_getproc(g_PluginMap[i].handle, "PluginShutdown");
		if (PluginShutdown != NULL)
			(*PluginShutdown)();
		osal_dynlib_close(g_PluginMap[i].handle);
		/* clear out the plugin map's members */
		g_PluginMap[i].handle = NULL;
		g_PluginMap[i].filename[0] = 0;
		g_PluginMap[i].libname[0] = 0;
		g_PluginMap[i].libversion = 0;
	}
	(*CoreDoCommand)(M64CMD_ROM_CLOSE, 0, NULL);
	loadedplugins=false;
	isEmulating = false;
	

}

CMupen64Plus::CMupen64Plus()
{
	ptr_CoreStartup         CoreStartup = NULL;
	ptr_CoreShutdown        CoreShutdown = NULL;
	ptr_CoreAttachPlugin    CoreAttachPlugin = NULL;
	ptr_CoreDetachPlugin    CoreDetachPlugin = NULL;
	ptr_CoreDoCommand       CoreDoCommand = NULL;
	ptr_CoreOverrideVidExt  CoreOverrideVidExt = NULL;
	ptr_CoreAddCheat        CoreAddCheat = NULL;
	ptr_CoreCheatEnabled    CoreCheatEnabled = NULL;

	/* definitions of pointers to Core config functions */
	ptr_ConfigListSections     PtrConfigListSections = NULL;
	ptr_ConfigOpenSection      PtrConfigOpenSection = NULL;
	ptr_ConfigListParameters   PtrConfigListParameters = NULL;
	ptr_ConfigSaveFile         PtrConfigSaveFile = NULL;
	ptr_ConfigSetParameter     PtrConfigSetParameter = NULL;
	ptr_ConfigGetParameter     PtrConfigGetParameter = NULL;
	ptr_ConfigGetParameterType PtrConfigGetParameterType = NULL;
	ptr_ConfigGetParameterHelp PtrConfigGetParameterHelp = NULL;
	ptr_ConfigSetDefaultInt    PtrConfigSetDefaultInt = NULL;
	ptr_ConfigSetDefaultFloat  PtrConfigSetDefaultFloat = NULL;
	ptr_ConfigSetDefaultBool   PtrConfigSetDefaultBool = NULL;
	ptr_ConfigSetDefaultString PtrConfigSetDefaultString = NULL;
	ptr_ConfigGetParamInt      PtrConfigGetParamInt = NULL;
	ptr_ConfigGetParamFloat    PtrConfigGetParamFloat = NULL;
	ptr_ConfigGetParamBool     PtrConfigGetParamBool = NULL;
	ptr_ConfigGetParamString   PtrConfigGetParamString = NULL;

	ptr_ConfigGetSharedDataFilepath PtrConfigGetSharedDataFilepath = NULL;
	ptr_ConfigGetUserConfigPath     PtrConfigGetUserConfigPath = NULL;
	ptr_ConfigGetUserDataPath       PtrConfigGetUserDataPath = NULL;
	ptr_ConfigGetUserCachePath      PtrConfigGetUserCachePath = NULL;
	CoreHandle = NULL;
    isEmulating = false;
	loadedplugins = false;
	use_videoext = true;
}

CMupen64Plus::~CMupen64Plus(void)
{
	
}

void CMupen64Plus::kill()
{
	(*CoreShutdown)();
	DetachCoreLib();
	
}