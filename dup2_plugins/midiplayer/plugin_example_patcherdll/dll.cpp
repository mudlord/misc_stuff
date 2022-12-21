#include <windows.h>
#include <math.h>
#include "dll.h"
#include "sound_out.h"
#include "gme/spc.h"

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

bool IsPlaying = FALSE;
DWORD WINAPI AudioThread(LPVOID lpParameter)
{
	
	thrddata* instance = (thrddata*)lpParameter;
	SNES_SPC* snes_spc = spc_new();
	SPC_Filter* filter = spc_filter_new();
	spc_load_spc( snes_spc,instance->data,instance->len );
	/* Most SPC files have garbage data in the echo buffer, so clear that */
	spc_clear_echo( snes_spc );
	/* Clear filter before playing */
	spc_filter_clear( filter );

	short *samples_buffer = (short*)malloc(2048*2);
	int const sample_count = 2048;
	csound = create_sound_out();
	csound->open(instance->hwnd,32000,2,sample_count,sample_count/2);
	IsPlaying=TRUE;
	while(IsPlaying)
	{
	 short * ptr = (short * ) samples_buffer;
	// hvl_DecodeFrame( m_tune, ptr,ptr + 1, 2 );
	// play_siren(sample_count,(short*)ptr);
	spc_play( snes_spc,sample_count,(short*)ptr ); 
	spc_filter_run( filter,ptr, sample_count );
	csound->write_frame(ptr,sample_count,true);
	}
	spc_filter_delete( filter );
	spc_delete( snes_spc );


	return 0;

}

void DLL_Play(HWND hwnd,void *data, int len, int subsong)
{
	threaddata = (thrddata*)malloc( sizeof(thrddata));
	threaddata->data = (void*)data;
	threaddata->len = len;
	threaddata->subsong = subsong;
	threaddata->hwnd = hwnd;
	g_handle = CreateThread( NULL, 0, AudioThread,(LPVOID)threaddata,   NULL, &g_id );
}

void DLL_Stop()
{
	IsPlaying = FALSE;
	WaitForSingleObject(g_handle,INFINITE);
	free(threaddata);
	delete csound;
	CloseHandle(g_handle);
	
}