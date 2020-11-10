#ifndef MUPENCPP_H_INCLUDED
#define MUPENCPP_H_INCLUDED


#include "m64p_types.h"
#include "m64p_common.h"
#include "m64p_frontend.h"
#include "m64p_config.h"
#include <gl/GL.h>

#ifdef __cplusplus
extern "C" {
#endif

class CMupen64Plus
{
private:
	static	CMupen64Plus* m_Instance ;
	m64p_dynlib_handle CoreHandle;
	
	
	m64p_error AttachCoreLib(const char *CoreLibFilepath);
	m64p_error DetachCoreLib(void);

	bool load_plugins();
	bool load_plugin(char *filename, int index);
	bool open_confighandles();
	DWORD threadID;
	HANDLE threadhandle;
public:
	typedef struct {
		m64p_plugin_type    type;
		char                name[8];
		m64p_dynlib_handle  handle;
		TCHAR                filename[MAX_PATH];
		char          libname[100];
		int                 libversion;
	} plugin_map_node;
	plugin_map_node g_PluginMap[4];
	ptr_CoreStartup         CoreStartup;
	ptr_CoreShutdown        CoreShutdown;
	ptr_CoreAttachPlugin    CoreAttachPlugin;
	ptr_CoreDetachPlugin    CoreDetachPlugin;
	ptr_CoreDoCommand       CoreDoCommand;
	ptr_CoreOverrideVidExt  CoreOverrideVidExt;
	ptr_CoreAddCheat        CoreAddCheat;
	ptr_CoreCheatEnabled    CoreCheatEnabled;
	/* definitions of pointers to Core config functions */
	ptr_ConfigListSections     ConfigListSections;
	ptr_ConfigOpenSection      ConfigOpenSection;
	ptr_ConfigDeleteSection    ConfigDeleteSection;
	ptr_ConfigSaveSection      ConfigSaveSection;
	ptr_ConfigListParameters   ConfigListParameters;
	ptr_ConfigSaveFile         ConfigSaveFile;
	ptr_ConfigSetParameter     ConfigSetParameter;
	ptr_ConfigGetParameter     ConfigGetParameter;
	ptr_ConfigGetParameterType ConfigGetParameterType;
	ptr_ConfigGetParameterHelp ConfigGetParameterHelp;
	ptr_ConfigSetDefaultInt    ConfigSetDefaultInt;
	ptr_ConfigSetDefaultFloat  ConfigSetDefaultFloat;
	ptr_ConfigSetDefaultBool   ConfigSetDefaultBool;
	ptr_ConfigSetDefaultString ConfigSetDefaultString;
	ptr_ConfigGetParamInt      ConfigGetParamInt;
	ptr_ConfigGetParamFloat    ConfigGetParamFloat;
	ptr_ConfigGetParamBool     ConfigGetParamBool;
	ptr_ConfigGetParamString   ConfigGetParamString;
	ptr_ConfigGetSharedDataFilepath ConfigGetSharedDataFilepath;
	ptr_ConfigGetUserConfigPath     ConfigGetUserConfigPath;
	ptr_ConfigGetUserDataPath       ConfigGetUserDataPath;
	ptr_ConfigGetUserCachePath      ConfigGetUserCachePath;
	m64p_handle l_ConfigCore;
	m64p_handle l_ConfigVideo;
	m64p_handle l_ConfigUI;

	HWND emulator_hwnd;
	bool isEmulating;
	bool loadedplugins;
	bool use_videoext;
	static CMupen64Plus* CreateInstance() ;
	static	CMupen64Plus* GetSingleton( ) ;
	CMupen64Plus();
	~CMupen64Plus();

	bool init(HWND hwnd);
	void kill();
	void run_frame();
	bool run_rom(const char *filename);
	void close_rom();
	static m64p_error getRomHeader(TCHAR* path, m64p_rom_header* out);
	static bool IsValidROM(TCHAR * filename);
	static bool IsValidExt(TCHAR* filename);

};

#ifdef __cplusplus
}
#endif

#endif MUPENCPP_H_INCLUDED
