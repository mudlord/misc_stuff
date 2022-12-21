#include <windows.h>
#include <math.h>
#include "main.h"
#include "../../sdk/dup2patcher.h"
#pragma comment (lib, "../../sdk/dup2patcher.lib")
#include "sound_out.h"
#include "gme/gme.h"

sound_out *csound;
HANDLE g_handle; 
DWORD  g_id;     

struct thrddata {
	void *data;
	int len;
	int subsong;
	HWND hwnd;
};
thrddata* threaddata;

void play_siren( long count, short* out )
{
	static double a, a2;
	while ( count-- )
		*out++ = 0x2000 * sin( a += .1 + .05*sin( a2+=.00005 ) );
}

DWORD WINAPI AudioThread(LPVOID lpParameter)
{
	thrddata* instance = (thrddata*)lpParameter;
	Music_Emu* emu = NULL;
	short *samples_buffer = (short*)malloc(4096*2);
	int const sample_count = 4096;
	csound = create_sound_out();
	csound->open(instance->hwnd,44100,2,sample_count,sample_count/2);
	gme_err_t err = gme_open_data(instance->data,instance->len,&emu,44100);
	if (err != NULL)return 0;
	gme_start_track( emu, instance->subsong );
	while(true)
	{
		short * ptr = (short * ) samples_buffer;
		// hvl_DecodeFrame( m_tune, ptr,ptr + 1, 2 );
		// play_siren(sample_count,(short*)ptr);
		gme_play( emu, sample_count,(short*)ptr );
		csound->write_frame(ptr,sample_count,true);
	}
	gme_delete(emu);
	return 0;
}

void *plugin_data;
void startdll()
{
	MY_PLUGIN_DATA_STRUCTURE* plugin_data = (MY_PLUGIN_DATA_STRUCTURE*)GetPluginDataMemory("com.mudlord.nsf.player");
	threaddata = (thrddata*)malloc( sizeof(thrddata));
	threaddata->data = &plugin_data->data;
	threaddata->len = plugin_data->len;
	threaddata->subsong = plugin_data->subsong;
	threaddata->hwnd = GetForegroundWindow();
	g_handle = CreateThread( NULL, 0, AudioThread,(LPVOID)threaddata,   NULL, &g_id );
}

void DLL_Stop()
{
	TerminateThread(g_handle,0);
	free(threaddata);
	//delete csound;
	CloseHandle(g_handle);
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
