#include "stdafx.h"
#include "mupencpp.h"
#include "osal_dynamiclib.h"
#include "osal_files.h"

#include <windows.h>
#include <Shlwapi.h>
#include "../DropFileTarget.h"
#include "../DlgTabCtrl.h"
#include "../utf8conv.h"
#include <vector>

#include <gl/GL.h>

using namespace std;
using namespace utf8util;



/* definitions of pointers to Core front-end functions */

const char* DEFAULT_VIDEO_PLUGIN = "mupen64plus-video-rice.dll";
const char* DEFAULT_AUDIO_PLUGIN = "mupen64plus-audio-sdl.dll";
const char* DEFAULT_INPUT_PLUGIN = "mupen64plus-input-sdl.dll";
const char* DEFAULT_RSP_PLUGIN   = "mupen64plus-rsp-hle.dll";

#define CORE_API_VERSION   0x20001
#define CONFIG_API_VERSION 0x20000
#define CONFIG_PARAM_VERSION     1.00

static CRITICAL_SECTION cs;
m64p_error VidExt_GL_SwapBuffers()
{
	
	CMupen64Plus* instance = CMupen64Plus::GetSingleton( ) ;
	SwapBuffers( instance->g_hDC ); 
	return M64ERR_SUCCESS;
}

m64p_error VidExt_ResizeWindow(int w, int h)
{
	// TODO

	return M64ERR_SUCCESS;
}

void* VidExt_GL_GetProcAddress(const char* Proc)
{
	void* out = NULL;
	out = (void*)wglGetProcAddress(Proc);
	return out;
}

bool doublebuffer = true;
int buffersize = 32;
int depthsize = 24;
int redsize = 8;
int greensize = 8;
int bluesize = 8;
int alphasize = 8;

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
	return M64ERR_SUCCESS;
}

m64p_error VidExt_Quit()
{
	
	CMupen64Plus* instance = CMupen64Plus::GetSingleton( ) ;
	if( instance->g_hRC != NULL )
	{
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( instance->g_hRC );
		instance->g_hRC = NULL;
	}

	if( instance->g_hDC != NULL )
	{
		ReleaseDC( instance->emulator_hwnd, instance->g_hDC );
		instance->g_hDC = NULL;
	}

    return M64ERR_SUCCESS;
}

m64p_error VidExt_ListFullscreenModes(m64p_2d_size *SizeArray, int *NumSizes)
{
	// TODO: do better!!
	static m64p_2d_size s[3];
	s[0].uiWidth = 640;
	s[0].uiHeight = 480;
	s[1].uiWidth = 800;
	s[1].uiHeight = 600;
	s[2].uiWidth = 1024;
	s[2].uiHeight = 768;
	SizeArray = s;
	*NumSizes = 3;
	return M64ERR_SUCCESS;
}

m64p_error VidExt_SetCaption(const char *Title)
{
    return M64ERR_SUCCESS;
}

m64p_error VidExt_ToggleFullScreen()
{
	return M64ERR_SUCCESS;
}

m64p_error VidExt_SetVideoMode(int Width, int Height, int BitsPerPixel,
	/*m64p_video_mode*/ int ScreenMode,
	/*m64p_video_flags*/ int Flags)
{
	CMupen64Plus* instance = CMupen64Plus::GetSingleton( ) ;
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = BitsPerPixel;
	pfd.cDepthBits = 24;
	instance->g_hDC = GetDC( instance->emulator_hwnd );
	GLuint iPixelFormat = ChoosePixelFormat( instance->g_hDC, &pfd );
	if( iPixelFormat != 0 )
	{
		PIXELFORMATDESCRIPTOR bestMatch_pfd;
		DescribePixelFormat( instance->g_hDC, iPixelFormat, sizeof(pfd), &bestMatch_pfd );
		if( bestMatch_pfd.cDepthBits < pfd.cDepthBits )
		{
			return M64ERR_UNSUPPORTED;
		}
		if( SetPixelFormat( instance->g_hDC, iPixelFormat, &pfd) == FALSE )
		{
			DWORD dwErrorCode = GetLastError();
			return M64ERR_UNSUPPORTED;
		}
	}
	else
	{
		DWORD dwErrorCode = GetLastError();
		return M64ERR_UNSUPPORTED;
	}
	instance->g_hRC = wglCreateContext( instance->g_hDC );
	wglMakeCurrent( instance->g_hDC, instance->g_hRC );
	SetWindowPos(instance->emulator_hwnd,0,0,0,Width,Height+40,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	// clear both buffers with black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	SwapBuffers(instance->g_hDC);
	return M64ERR_SUCCESS;
}

void CMupen64Plus::install_videoext()
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
CMupen64Plus* CMupen64Plus::CreateInstance(HWND hwnd )
{
	if (0 == m_Instance)
	{
		m_Instance = new CMupen64Plus( ) ;
		m_Instance->init( hwnd) ;
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
	
	AttachCoreLib((const char*)coredllpath);
	m64p_error rval = (*CoreStartup)(CORE_API_VERSION, (const char*)corepath, (const char*)corepath, "Core", NULL, NULL, NULL);
	if (rval != M64ERR_SUCCESS)
	{
		DetachCoreLib();
		return false;
	}
	open_confighandles();
	if(use_videoext)install_videoext();
	return true;
}

 m64p_plugin_type CMupen64Plus::plugin_type(const char* filename)
{
	m64p_dynlib_handle handle;
	m64p_error rval = osal_dynlib_open(&handle, filename);
	ptr_PluginGetVersion PluginGetVersion = (ptr_PluginGetVersion) osal_dynlib_getproc(handle, "PluginGetVersion");
	if (PluginGetVersion == NULL)
	{
		osal_dynlib_close(handle);
		return M64PLUGIN_NULL;
	}
	m64p_plugin_type PluginType = (m64p_plugin_type) 0;
	int PluginVersion = 0;
	const char *PluginName = NULL;
	(*PluginGetVersion)(&PluginType, &PluginVersion, NULL, &PluginName, NULL);
	osal_dynlib_close(handle);
	return PluginType;
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
		 (*ConfigDeleteSection)("WinMupen");
		 (*ConfigOpenSection)("WinMupen", &l_ConfigUI);
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

	 /* Set default values for my Config parameters */
	 (*ConfigSetDefaultString)(l_ConfigUI, "PluginDir", OSAL_CURRENT_DIR, "Directory in which to search for plugins");
	 (*ConfigSetDefaultString)(l_ConfigUI, "VideoPlugin", "mupen64plus-video-rice" OSAL_DLL_EXTENSION, "Filename of video plugin");
	 (*ConfigSetDefaultString)(l_ConfigUI, "AudioPlugin", "mupen64plus-audio-sdl" OSAL_DLL_EXTENSION, "Filename of audio plugin");
	 (*ConfigSetDefaultString)(l_ConfigUI, "InputPlugin", "mupen64plus-input-sdl" OSAL_DLL_EXTENSION, "Filename of input plugin");
	 (*ConfigSetDefaultString)(l_ConfigUI, "RspPlugin", "mupen64plus-rsp-hle" OSAL_DLL_EXTENSION, "Filename of RSP plugin");
	 if (bSaveConfig && ConfigSaveSection != NULL) /* ConfigSaveSection was added in Config API v2.1.0 */
	rval = (*ConfigSaveSection)("WinMupen");

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
	 strcpy(g_PluginMap[index].filename, filename);
	 g_PluginMap[index].libname = PluginName;
	 g_PluginMap[index].libversion = PluginVersion;
	 return true;
 }

 bool CMupen64Plus::load_plugins()
 {
	 //g_PluginMap[0].type = M64PLUGIN_GFX;
	 plugin_map_node PluginMap[]	 = {
	     { M64PLUGIN_GFX,   "Video", NULL, "mupen64plus-video-glide64mk2.dll", NULL, 0 },
		 {M64PLUGIN_AUDIO, "Audio", NULL, "mupen64plus-audio-sdl.dll", NULL, 0 },
		 {M64PLUGIN_INPUT, "Input", NULL, "mupen64plus-input-sdl.dll", NULL, 0 },
		 {M64PLUGIN_RSP,   "RSP",   NULL, "mupen64plus-rsp-hle.dll", NULL, 0 }
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

		  TCHAR dllpath[512] = {0};
		  HMODULE hModule = GetModuleHandle(NULL);
		  if (hModule != NULL)
		  {
			  // When passing NULL to GetModuleHandle, it returns handle of exe itself
			  GetModuleFileName(hModule,dllpath, (sizeof(dllpath))); 
			  PathRemoveFileSpec(dllpath);
		  }
		  lstrcat(dllpath, L"\\");
		  wstring filename = utf16_from_utf8(g_PluginMap[i].filename);
		  lstrcat(dllpath,filename.c_str());
		  string str = utf8_from_utf16(dllpath);

		 load_plugin((char*)filename.c_str(),i);

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


 DWORD EmulatorThread(LPVOID lpdwThreadParam )
 {
	 CMupen64Plus* instance = CMupen64Plus::GetSingleton( ) ;
	 instance->CoreDoCommand(M64CMD_EXECUTE, 0, NULL);
	 instance->isEmulating = false;
	 //while(1)
	// {
	//	 CMupen64Plus* instance = CMupen64Plus::GetSingleton( ) ;
	//	 if(!instance->isEmulating)break;
	//	 Sleep(10);
	// }
	 return 0;
 }

bool CMupen64Plus::run_rom(const char *filename)
{                                          
	if (isEmulating) close_rom();
	wstring str = utf16_from_utf8(filename);
	HANDLE hFile = CreateFile(str.c_str(),GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	LARGE_INTEGER size;
	if (!GetFileSizeEx(hFile, &size)) return false;
	unsigned char *ROM_buffer = (unsigned char *) malloc(size.QuadPart);

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

	if (!loadedplugins)load_plugins();
	isEmulating = true;
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
		g_PluginMap[i].libname = NULL;
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
	g_hDC  = NULL;
	g_hRC  = NULL;
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