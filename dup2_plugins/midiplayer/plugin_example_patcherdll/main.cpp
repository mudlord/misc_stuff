#include <windows.h>
#include <math.h>
#include "main.h"
#include <MMSystem.h>
#include "cSound.h"
#pragma comment(lib, "winmm.lib")
#include "../../sdk/dup2patcher.h"
#pragma comment (lib, "../../sdk/dup2patcher.lib")

HANDLE g_handle; 
DWORD  g_id;   
cSound i_Sound;

char tempfile[MAX_PATH];
void *plugin_data;
void startdll()
{
	char temppath[MAX_PATH];
	
	HANDLE hfile;
	DWORD len,c;
	MY_PLUGIN_DATA_STRUCTURE* plugin_data = (MY_PLUGIN_DATA_STRUCTURE*)GetPluginDataMemory("com.mudlord.midi.player");
	GetTempPath(MAX_PATH,temppath);
	GetTempFileName(temppath,"mid",0,tempfile);
	/* write BASS.DLL to the temporary file */
	if (INVALID_HANDLE_VALUE==(hfile=CreateFile(tempfile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY,NULL))) {
		return;
	}
	WriteFile(hfile,plugin_data->data,plugin_data->len,&c,NULL);
	CloseHandle(hfile);

	i_Sound.PlaySoundFile(tempfile);
	
}

void DLL_Stop()
{
	DeleteFile(tempfile);
	free(plugin_data);
}

HINSTANCE hinst;

/////////////////////////////////////////////////////////////////////////
// PLUGIN_Action is called by the patcher during the patching procedure
// PLUGIN_Action must return TRUE if everything went OK, else return FALSE!
////////////////////////////////////////////////////////////////////////
BOOL __stdcall PLUGIN_Action(MY_PLUGIN_DATA_STRUCTURE* _plugin_data) {
	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// The patcher dll is loaded when the patcher window is created (WM_INITDIALOG)
// The patcher dll is unloaded when the patcher window is closed (WM_CLOSE)
////////////////////////////////////////////////////////////////////////
BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD reason, LPVOID lpReserved ) {
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			startdll();
			break;
		//case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			DLL_Stop();
			break;
	}
	return TRUE;
}
