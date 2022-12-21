#include <windows.h>
#include <math.h>
#include "main.h"
#include "../../sdk/dup2patcher.h"
#pragma comment (lib, "../../sdk/dup2patcher.lib")
#define BASSDEF(f) (WINAPI *f)	// define the functions as pointers
#include "bass.h"
#include "bass_dll.h"

extern "C" unsigned char bass_dll;
extern "C" DWORD bass_dll_size;

char tempfile[MAX_PATH];	// temporary BASS.DLL
#define LOADBASSFUNCTION(f) *((void**)&f)=(void*)GetProcAddress(module,#f)
HINSTANCE module=0;
void *plugin_data;
void startdll()
{
	MY_PLUGIN_DATA_STRUCTURE* plugin_data = (MY_PLUGIN_DATA_STRUCTURE*)GetPluginDataMemory("com.mudlord.mp3.player");
	char temppath[MAX_PATH];
	HANDLE hfile;
	DWORD len,c;
	DWORD chan;
	GetTempPath(MAX_PATH,temppath);
	GetTempFileName(temppath,"bas",0,tempfile);
	/* write BASS.DLL to the temporary file */
	if (INVALID_HANDLE_VALUE==(hfile=CreateFile(tempfile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY,NULL))) {
		return;
	}
	WriteFile(hfile,bass_data,bass_len,&c,NULL);
	CloseHandle(hfile);
	if (!(module=LoadLibrary(tempfile))) {
		return;
	}
	LOADBASSFUNCTION(BASS_ErrorGetCode);
	LOADBASSFUNCTION(BASS_Init);
	LOADBASSFUNCTION(BASS_Free);
	LOADBASSFUNCTION(BASS_StreamCreateFile);
	LOADBASSFUNCTION(BASS_ChannelPlay);
	BASS_Init(-1,44100,0,GetPatcherWindowHandle(),NULL);
	chan=BASS_StreamCreateFile(TRUE,&plugin_data->data,0,plugin_data->len,BASS_SAMPLE_LOOP);
	BASS_ChannelPlay(chan,TRUE);
}

void DLL_Stop()
{
	if (!module) return;
	FreeLibrary(module);
	module=0;
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
