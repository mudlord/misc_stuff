#ifndef ___MAIN__H__H__
#define ___MAIN__H__H__


// functions prototypes
int DialogPlugin(HWND, UINT, WPARAM, LPARAM);
void LoadPluginData();
void SavePluginData();

// structures
typedef struct MY_PLUGIN_DATA_STRUCTURE {
	int len;
	int subsong;
	BYTE data[];
} MY_PLUGIN_DATA_STRUCTURE, *LPMY_PLUGIN_DATA_STRUCTURE;



#endif //___MAIN__H__H__