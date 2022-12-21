#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "resource.h"
#include "../../sdk/dup2.h"
#pragma comment (lib, "../../sdk/dup2.lib")

HINSTANCE hinst;
HWND hDialogPlugin;
MY_PLUGIN_DATA_STRUCTURE* current_plugin_data;

PLUGIN_INFO example_plugin_info = {
	DUP2_PLUGIN_VERSION,
	"com.mudlord.spc.player",
	"[SPC player]",
	"spc_playerdll.d2p"
};


PLUGIN_INFO * __stdcall DUP2_PluginInfo() {
	return &example_plugin_info;
}

void __stdcall  DUP2_EditPluginData(MY_PLUGIN_DATA_STRUCTURE* plugin_data) {
	current_plugin_data = plugin_data;
	DialogBoxParam(hinst, "DIALOG_PLUGIN",
	               GetDup2MainDialogHandle(),
	               (DLGPROC)DialogPlugin, 0);
}

const char* __stdcall  DUP2_ModuleDescription(MY_PLUGIN_DATA_STRUCTURE* plugin_data) {
	return "Plays back SPC files";
}

char local_buffer[1024];
int  DialogPlugin(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	switch ( message ) {
		case WM_INITDIALOG:
			hDialogPlugin = hwnd;
			StartChooseHideMethod(hwnd);
			LoadWindowPosition(hwnd, "com.mudlord.spc.player");
			LoadPluginData();
			break;
			
		case WM_COMMAND:
			switch ( LOWORD(wparam) ) {
				case BTN_CANCEL:
					SendMessage(hwnd, WM_CLOSE, 0, 0);
					break;
					
				case BTN_SAVE:
					SavePluginData();
					SendMessage(hwnd, WM_CLOSE, 0, 0);
					break;
					
				case BTN_LOAD:
					if ( d2k2_GetFilePath(local_buffer,
					                      "SNES music files [spc]\0*.spc;*\0",
					                      "c:\\",
					                      hDialogPlugin) ) {
						SetDlgItemText(hDialogPlugin, DLG_FILE, d2k2_FileNameOfPath(local_buffer));
					}
					break;
			}
			break;
			
		case WM_MOUSEMOVE:
			if ( wparam == MK_LBUTTON )
				SendMessage(hwnd, WM_SYSCOMMAND, 0xF012u, 0);
			break;
			
		case WM_CLOSE:
			EndChooseHideMethod();
			SaveWindowPosition(hwnd, "com.mudlord.spc.player", POS_NOSIZE);
			EndDialog(hwnd, 0);
			break;
			
		default :
			return FALSE;
	}
	return TRUE;
}


BYTE *Load_Input_File(char *FileName, DWORD *Size)
{
	BYTE *Memory;
	FILE *Input = fopen(FileName, "rb");
	if(!Input) return(NULL);
	// Get the filesize
	fseek(Input, 0, SEEK_END);
	*Size = ftell(Input);
	fseek(Input, 0, SEEK_SET);
	Memory = (BYTE *) malloc(*Size);
	if(!Memory) return(NULL);
	if(fread(Memory, 1, *Size, Input) != (size_t) *Size) return(NULL);
	if(Input) fclose(Input);
	Input = NULL;
	return(Memory);
}

void  SavePluginData() {
	DWORD Input_Size;
	MY_PLUGIN_DATA_STRUCTURE* song_struct;
	BYTE *song_data= Load_Input_File(local_buffer, &Input_Size);
	current_plugin_data = (MY_PLUGIN_DATA_STRUCTURE*) ResizeCurrentPluginDataMemory((sizeof(MY_PLUGIN_DATA_STRUCTURE)) + Input_Size, 0);
	memcpy(current_plugin_data->data,song_data,Input_Size);
	current_plugin_data->len = Input_Size;
	current_plugin_data->subsong = 0;
}

void LoadPluginData() {

}

BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD reason, LPVOID lpReserved ) {
	switch (reason) {
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			hinst = hModule;
			break;
	}
	return TRUE;
}
