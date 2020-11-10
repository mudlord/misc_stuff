#ifndef MYWINDOW_H_INCLUDED
#define MYWINDOW_H_INCLUDED

#include <windows.h>
#include <Shlwapi.h>
#include "DropFileTarget.h"
#include "DlgTabCtrl.h"
#include "utf8conv.h"
#include <vector>


#include "mupenapi/mupencpp.h"
#include "SDL_keysym.h"

using namespace std;
using namespace utf8util;





class CPluginView : public CDialogImpl<CPluginView>
{
public:
	HWND m_hwndOwner; 
	enum { IDD = IDD_PLUGINS };
	BEGIN_MSG_MAP(CPluginView)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialogView1)
	END_MSG_MAP()

	LRESULT OnInitDialogView1(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return TRUE;
	}
};

class CInputView : public CDialogImpl<CInputView>
{
public:
	HWND m_hwndOwner; 
	enum { IDD = IDD_INPUT };
	BEGIN_MSG_MAP( CPluginView)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialogView2)
	END_MSG_MAP()

	LRESULT OnInitDialogView2(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return TRUE;
	}
};

class COptionsTab : public CDialogImpl<COptionsTab>
{
public:
	CDialogTabCtrl m_ctrlTab;
	CPluginView m_view1;
	CInputView m_view2;
	enum { IDD = IDD_SETTINGS };

	BEGIN_MSG_MAP( COptionsTab)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		REFLECT_NOTIFICATIONS();
	END_MSG_MAP()




	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();
		// set icons

		m_ctrlTab.SubclassWindow(GetDlgItem(IDC_TABCTRL));
		m_view1.Create(m_hWnd);
		m_view2.Create(m_hWnd);
		TCITEM tci = { 0 };
		tci.mask = TCIF_TEXT;
		tci.pszText = _T("Options");
		m_ctrlTab.InsertItem(0, &tci, m_view1);
		tci.pszText = _T("Paths");
		m_ctrlTab.InsertItem(1, &tci, m_view2);
		m_ctrlTab.SetCurSel(0);
		return TRUE;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add validation code
		EndDialog(wID);
		return 0;
	}

};

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
	CHyperLink website;
	CStatic version_number;
	CStatic builddate;
public:
	enum { IDD = IDD_ABOUT };
	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialogView1)
		COMMAND_ID_HANDLER(IDOK,OnButtonOK)
	END_MSG_MAP()
	LRESULT OnInitDialogView1(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
	/*	CString verinf0 = SVN_REVISION;
		CString date = SVN_TIME_NOW;
		builddate = GetDlgItem(IDC_BUILDDATE);
		builddate.SetWindowText(date);
		version_number = GetDlgItem(IDC_APPVER);
		version_number.SetWindowText(verinf0);
		website.SubclassWindow(GetDlgItem(IDC_LINK));
		website.SetHyperLink(_T("http://vba-m.com/forum/Forum-other-stuff"));*/
		return TRUE;
	}
	LRESULT OnButtonOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}
};

typedef struct {
	TCHAR     FileNameNoExt[MAX_PATH];
	TCHAR     FileName[MAX_PATH];
	TCHAR     InternalName[22];
	BYTE     Country;
}ROM_INFO;

class CMyWindow;

class CRomBrowser
{
public:
	HWND m_hRomList;
	HWND parent_hwnd;
	CString ROMDirectory;
	CString EmulatorDirectory;
	CString PluginDirectory;

	enum { IDC_ROMLIST = 223 };

	 std::vector<ROM_INFO>m_RomInfo;
	

	void reset(HWND parent, CString rom_directory)
	{
		
		ROMDirectory = rom_directory;
		
		RECT rcl; 
		GetClientRect( parent, &rcl );
		m_hRomList = (HWND)CreateWindowEx( 0,WC_LISTVIEW,NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER |
		LVS_REPORT,0,0,
		rcl.right - rcl.left,
		rcl.bottom - rcl.top - 99,
        (HWND)parent,(HMENU)IDC_ROMLIST,_Module.GetResourceInstance(),NULL);	

		EnableWindow((HWND)m_hRomList,TRUE);
		ShowWindow((HWND)m_hRomList,SW_SHOW);

		LVCOLUMN lvc = { 0 };
		lvc.mask = LVCF_TEXT | LVCF_WIDTH;
		lvc.pszText = _T("ROM Name");
		lvc.cx = 300;
		ListView_InsertColumn(m_hRomList, 0, &lvc);

		lvc.mask = LVCF_TEXT | LVCF_WIDTH;
		lvc.pszText = _T("Internal Name");
		lvc.cx = 200;
		ListView_InsertColumn(m_hRomList, 1, &lvc);

		lvc.mask = LVCF_TEXT | LVCF_WIDTH;
		lvc.pszText = _T("Country");
		lvc.cx = 100;
		ListView_InsertColumn(m_hRomList, 2, &lvc);

		fill_list();
	}

public:
	bool RomListNotify(int idCtrl, DWORD pnmh) {
		if (idCtrl != IDC_ROMLIST) { return false; }
		if (!RomBrowserVisible()) { return false; }

		switch (((LPNMHDR)pnmh)->code) {
			//case LVN_COLUMNCLICK: RomList_ColoumnSortList(pnmh); break;
			//case NM_RETURN:       RomList_OpenRom(pnmh); break;
		case NM_DBLCLK:       open_rom(pnmh); break;
			//	case LVN_GETDISPINFO: RomList_GetDispInfo(pnmh); break;
			//	case NM_RCLICK:       RomList_PopupMenu(pnmh); break;
		case NM_CLICK:
			{
				LONG iItem = ListView_GetNextItem((HWND)m_hRomList, -1, LVNI_SELECTED);
				if (iItem != -1) 
				{ 
					
				}
			}
			break;
		default:
			return false;
		}
		return true;
	}
	bool RomBrowserVisible (void) {
		if (!IsWindow((HWND)m_hRomList)) { return false; }
		if (!IsWindowVisible((HWND)m_hRomList)) { return false; }
		return true;
	}

	void show()
	{
		EnableWindow((HWND)m_hRomList,TRUE);
		ShowWindow((HWND)m_hRomList,SW_SHOW);
	}
	void hide()
	{
		EnableWindow((HWND)m_hRomList,FALSE);
		ShowWindow((HWND)m_hRomList,SW_HIDE);
	}
private:

	
	void open_rom(DWORD pnmh)
	{
		ROM_INFO * pRomInfo;
		LV_ITEM lvItem;
		LONG iItem;
		iItem = ListView_GetNextItem((HWND)m_hRomList, -1, LVNI_SELECTED);
		if (iItem == -1) { return; }
		memset(&lvItem, 0, sizeof(LV_ITEM));
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = iItem;
		if (!ListView_GetItem((HWND)m_hRomList, &lvItem)) { return; }
		if (lvItem.iItem < 0 || lvItem.iItem >= (LPARAM)m_RomInfo.size())
		{
			return;
		}
		pRomInfo = &m_RomInfo[lvItem.iItem];
		CMupen64Plus* instance = CMupen64Plus::GetSingleton( ) ;
		string utf8 = utf8_from_utf16(pRomInfo->FileName);
		hide();
		instance->run_rom(utf8.c_str());
		if(!instance->use_videoext)show();
		
	}

	m64p_error getRomHeader(CString path, m64p_rom_header* out)
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


	bool IsValidROM(TCHAR * filename)
	{
		 m64p_rom_header header;
		 CString rompath;
		 rompath = ROMDirectory;
		 rompath += "\\";
		 rompath += filename;
		 m64p_error result = getRomHeader(rompath, &header);
		 if (result != M64ERR_SUCCESS)
		 {
			 return false;
		 }
		 return true;
	}

	TCHAR* getCountryName(unsigned short countrycode)
	{
		switch (countrycode)
		{
		case 0x41:
			return L"Japan/USA";
		case 0x44:
			return L"Germany";
		case 0x45:
			return L"USA";
		case 0x46:
			return L"France";
		case 'I':
			return L"Italy";
		case 0x4A:
			return L"Japan";
		case 'S':
			return L"Spain";
		case 0x55: case 0x59:
			return L"Australia";
		case 0x50: case 0x58: case 0x20:
		case 0x21: case 0x38: case 0x70:
			return L"Europe";
		default:
			return L"Other";
		}
	}

	void build_list()
	{
		CString rom_path;
		CString cache_filename;

		TCHAR ownPth[MAX_PATH] = {0};
		// Will contain exe path
		HMODULE hModule = GetModuleHandle(NULL);
		if (hModule != NULL)
		{
			// When passing NULL to GetModuleHandle, it returns handle of exe itself
			GetModuleFileName(hModule,ownPth, (sizeof(ownPth))); 
			PathRemoveFileSpec(ownPth);
		}
		cache_filename = ownPth;
		cache_filename += "\\cache.dat";

		rom_path = ROMDirectory;
		rom_path += "\\*";

		WIN32_FIND_DATA ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		hFind = FindFirstFile(rom_path, &ffd);
		int i=0;
		if (hFind == INVALID_HANDLE_VALUE)
		{
			return;
		} 

		do
		{
			if (ffd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY && IsValidROM(ffd.cFileName))
			{
				ROM_INFO entry;
				m64p_rom_header header;
				

				TCHAR rom_name[MAX_PATH] = {0};
				lstrcpy(rom_name,ffd.cFileName);
				PathRemoveExtension(rom_name);
				CString rompath;
				rompath = ROMDirectory;
				rompath += "\\";
				rompath += ffd.cFileName;
				m64p_error result = getRomHeader(rompath, &header);
				if (result != M64ERR_SUCCESS)
				{
					continue;
				}
				wstring internal_str = utf16_from_utf8((char*)header.Name);			
				TCHAR internal_name[22] = {0};
				lstrcpy(internal_name,internal_str.c_str());

				//pack data into vector
				entry.Country = header.Country_code;
				lstrcpy(entry.FileName,rompath);
				lstrcpy(entry.FileNameNoExt,rom_name);
				lstrcpy(entry.InternalName,internal_name);
				m_RomInfo.push_back(entry);
				
				
			}
		}
		while (FindNextFile(hFind, &ffd) != 0);

	HANDLE hFile = CreateFile(cache_filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
	DWORD dwWritten;
	int Entries = m_RomInfo.size();
	WriteFile(hFile,&Entries,sizeof(Entries),&dwWritten,NULL);
	//Write Every Entry
	for (int count = 0; count < Entries; count++) {
		ROM_INFO * RomInfo = &m_RomInfo[count];
		WriteFile(hFile,RomInfo,sizeof(ROM_INFO),&dwWritten,NULL);
	}
	//Close the file handle
	CloseHandle(hFile);
	fill_list();
	}

    void fill_list()
	{
		bool is_cache = false;
		TCHAR ownPth[MAX_PATH] = {0};
		CString cache_filename;
		// Will contain exe path
		HMODULE hModule = GetModuleHandle(NULL);
		if (hModule != NULL)
		{
			// When passing NULL to GetModuleHandle, it returns handle of exe itself
			GetModuleFileName(hModule,ownPth, (sizeof(ownPth))); 
			PathRemoveFileSpec(ownPth);
		}
		cache_filename = ownPth;
		cache_filename += "\\cache.dat";

		m_RomInfo.clear();
		
		HANDLE hFile = CreateFile(cache_filename,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			//if file does not exist then refresh the data
			build_list();
			return;
		}

		DWORD dwRead;
		int Entries = 0;
		ReadFile(hFile,&Entries,sizeof(Entries),&dwRead,NULL);
		for (int count = 0; count < Entries; count++) {
			ROM_INFO RomInfo = {0};
			ReadFile(hFile,&RomInfo,sizeof(ROM_INFO),&dwRead,NULL);
			LVITEM   lv  = { 0 };
			lv.iItem = count;
			ListView_InsertItem(m_hRomList, &lv);
			ListView_SetItemText(m_hRomList, lv.iItem, 0,  RomInfo.FileNameNoExt);
			ListView_SetItemText(m_hRomList, lv.iItem, 1,  RomInfo.InternalName);
			ListView_SetItemText(m_hRomList, lv.iItem, 2,  getCountryName(RomInfo.Country));	
			m_RomInfo.push_back(RomInfo);
		}
		CloseHandle(hFile);
	}
};



class CMyWindow : public CFrameWindowImpl<CMyWindow>, public CDropFileTarget<CMyWindow>,public CMessageFilter
{
public:
	DECLARE_FRAME_WND_CLASS ( _T("emu_wtl"), IDR_MAINFRAME );
	BEGIN_MSG_MAP_EX(CMyWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_NOTIFY,OnNotify)
		MESSAGE_HANDLER(WM_KEYDOWN,OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP,OnKeyUp)

		COMMAND_ID_HANDLER_EX(IDC_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_RESET_HARDRESET,OnHardReset);
	    COMMAND_ID_HANDLER(ID_RESET_SOFTRESET,OnSoftReset);
		COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER_EX(IDC_ABOUT, OnAbout)
		COMMAND_ID_HANDLER(ID_EMULATION_END,OnEndEmulation);
		COMMAND_ID_HANDLER(ID_OPTIONS_PREFERENCES, OnOptions)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMyWindow>)
		CHAIN_MSG_MAP(CDropFileTarget<CMyWindow>)
		END_MSG_MAP()

		CRomBrowser browser;
		CMupen64Plus *emulator;
		DWORD SDLkeymap[323];

		BOOL PreTranslateMessage(MSG* pMsg)
		{
			return CWindow::IsDialogMessage(pMsg);
		}

		BOOL IsRunning()
		{
			return FALSE;
		}

		void DoFrame()
		{
	
		}

		void create_keymap()
		{
			for ( int i=0; i<ARRAYSIZE(SDLkeymap); ++i )
				SDLkeymap[i] = 0;

			SDLkeymap[VK_BACK] = SDLK_BACKSPACE;
			SDLkeymap[VK_TAB] = SDLK_TAB;
			SDLkeymap[VK_CLEAR] = SDLK_CLEAR;
			SDLkeymap[VK_RETURN] = SDLK_RETURN;
			SDLkeymap[VK_PAUSE] = SDLK_PAUSE;
			SDLkeymap[VK_ESCAPE] = SDLK_ESCAPE;
			SDLkeymap[VK_SPACE] = SDLK_SPACE;
			SDLkeymap[VK_APOSTROPHE] = SDLK_QUOTE;
			SDLkeymap[VK_COMMA] = SDLK_COMMA;
			SDLkeymap[VK_MINUS] = SDLK_MINUS;
			SDLkeymap[VK_PERIOD] = SDLK_PERIOD;
			SDLkeymap[VK_SLASH] = SDLK_SLASH;
			SDLkeymap[0x30] = SDLK_0;
			SDLkeymap[0x31] = SDLK_1;
			SDLkeymap[0x32] = SDLK_2;
			SDLkeymap[0x33] = SDLK_3;
			SDLkeymap[0x34] = SDLK_4;
			SDLkeymap[0x35] = SDLK_5;
			SDLkeymap[0x36] = SDLK_6;
			SDLkeymap[0x37] = SDLK_7;
			SDLkeymap[0x38] = SDLK_8;
			SDLkeymap[0x39] = SDLK_9;
			SDLkeymap[VK_SEMICOLON] = SDLK_SEMICOLON;
			SDLkeymap[VK_EQUALS] = SDLK_EQUALS;
			SDLkeymap[VK_LBRACKET] = SDLK_LEFTBRACKET;
			SDLkeymap[VK_BACKSLASH] = SDLK_BACKSLASH;
			SDLkeymap[VK_OEM_102] = SDLK_LESS;
			SDLkeymap[VK_RBRACKET] = SDLK_RIGHTBRACKET;
			SDLkeymap[VK_GRAVE] = SDLK_BACKQUOTE;
			SDLkeymap[VK_BACKTICK] = SDLK_BACKQUOTE;
			SDLkeymap[VK_A] = SDLK_a;
			SDLkeymap[VK_B] = SDLK_b;
			SDLkeymap[VK_C] = SDLK_c;
			SDLkeymap[VK_D] = SDLK_d;
			SDLkeymap[VK_E] = SDLK_e;
			SDLkeymap[VK_F] = SDLK_f;
			SDLkeymap[VK_G] = SDLK_g;
			SDLkeymap[VK_H] = SDLK_h;
			SDLkeymap[VK_I] = SDLK_i;
			SDLkeymap[VK_J] = SDLK_j;
			SDLkeymap[VK_K] = SDLK_k;
			SDLkeymap[VK_L] = SDLK_l;
			SDLkeymap[VK_M] = SDLK_m;
			SDLkeymap[VK_N] = SDLK_n;
			SDLkeymap[VK_O] = SDLK_o;
			SDLkeymap[VK_P] = SDLK_p;
			SDLkeymap[VK_Q] = SDLK_q;
			SDLkeymap[VK_R] = SDLK_r;
			SDLkeymap[VK_S] = SDLK_s;
			SDLkeymap[VK_T] = SDLK_t;
			SDLkeymap[VK_U] = SDLK_u;
			SDLkeymap[VK_V] = SDLK_v;
			SDLkeymap[VK_W] = SDLK_w;
			SDLkeymap[VK_X] = SDLK_x;
			SDLkeymap[VK_Y] = SDLK_y;
			SDLkeymap[VK_Z] = SDLK_z;
			SDLkeymap[VK_DELETE] = SDLK_DELETE;

			SDLkeymap[VK_NUMPAD0] = SDLK_KP0;
			SDLkeymap[VK_NUMPAD1] = SDLK_KP1;
			SDLkeymap[VK_NUMPAD2] = SDLK_KP2;
			SDLkeymap[VK_NUMPAD3] = SDLK_KP3;
			SDLkeymap[VK_NUMPAD4] = SDLK_KP4;
			SDLkeymap[VK_NUMPAD5] = SDLK_KP5;
			SDLkeymap[VK_NUMPAD6] = SDLK_KP6;
			SDLkeymap[VK_NUMPAD7] = SDLK_KP7;
			SDLkeymap[VK_NUMPAD8] = SDLK_KP8;
			SDLkeymap[VK_NUMPAD9] = SDLK_KP9;
			SDLkeymap[VK_DECIMAL] = SDLK_KP_PERIOD;
			SDLkeymap[VK_DIVIDE] = SDLK_KP_DIVIDE;
			SDLkeymap[VK_MULTIPLY] = SDLK_KP_MULTIPLY;
			SDLkeymap[VK_SUBTRACT] = SDLK_KP_MINUS;
			SDLkeymap[VK_ADD] = SDLK_KP_PLUS;

			SDLkeymap[VK_UP] = SDLK_UP;
			SDLkeymap[VK_DOWN] = SDLK_DOWN;
			SDLkeymap[VK_RIGHT] = SDLK_RIGHT;
			SDLkeymap[VK_LEFT] = SDLK_LEFT;
			SDLkeymap[VK_INSERT] = SDLK_INSERT;
			SDLkeymap[VK_HOME] = SDLK_HOME;
			SDLkeymap[VK_END] = SDLK_END;
			SDLkeymap[VK_PRIOR] = SDLK_PAGEUP;
			SDLkeymap[VK_NEXT] = SDLK_PAGEDOWN;

			SDLkeymap[VK_F1] = SDLK_F1;
			SDLkeymap[VK_F2] = SDLK_F2;
			SDLkeymap[VK_F3] = SDLK_F3;
			SDLkeymap[VK_F4] = SDLK_F4;
			SDLkeymap[VK_F5] = SDLK_F5;
			SDLkeymap[VK_F6] = SDLK_F6;
			SDLkeymap[VK_F7] = SDLK_F7;
			SDLkeymap[VK_F8] = SDLK_F8;
			SDLkeymap[VK_F9] = SDLK_F9;
			SDLkeymap[VK_F10] = SDLK_F10;
			SDLkeymap[VK_F11] = SDLK_F11;
			SDLkeymap[VK_F12] = SDLK_F12;
			SDLkeymap[VK_F13] = SDLK_F13;
			SDLkeymap[VK_F14] = SDLK_F14;
			SDLkeymap[VK_F15] = SDLK_F15;

			SDLkeymap[VK_NUMLOCK] = SDLK_NUMLOCK;
			SDLkeymap[VK_CAPITAL] = SDLK_CAPSLOCK;
			SDLkeymap[VK_SCROLL] = SDLK_SCROLLOCK;
			SDLkeymap[VK_RSHIFT] = SDLK_RSHIFT;
			SDLkeymap[VK_LSHIFT] = SDLK_LSHIFT;
			SDLkeymap[VK_RCONTROL] = SDLK_RCTRL;
			SDLkeymap[VK_LCONTROL] = SDLK_LCTRL;
			SDLkeymap[VK_RMENU] = SDLK_RALT;
			SDLkeymap[VK_LMENU] = SDLK_LALT;
			SDLkeymap[VK_RWIN] = SDLK_RSUPER;
			SDLkeymap[VK_LWIN] = SDLK_LSUPER;

			SDLkeymap[VK_HELP] = SDLK_HELP;
#ifdef VK_PRINT
			SDLkeymap[VK_PRINT] = SDLK_PRINT;
#endif
			SDLkeymap[VK_SNAPSHOT] = SDLK_PRINT;
			SDLkeymap[VK_CANCEL] = SDLK_BREAK;
			SDLkeymap[VK_APPS] = SDLK_MENU;


		}

		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			CMessageLoop* pLoop = _Module.GetMessageLoop();
			
			ATLASSERT(pLoop != NULL);
			bHandled = FALSE;
			RegisterDropTarget();

			create_keymap();

			emulator = CMupen64Plus::CreateInstance(m_hWnd ) ;
			browser.reset(m_hWnd,"C:\\ROMS");

			return 0;
		}

		DWORD make_dword( WORD lo, WORD hi )
		{
			return ( ( DWORD )hi << 16 ) | lo;
		}

		LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
		   if (emulator->isEmulating)
		   {
			    DWORD sdlmod = 0;
			    DWORD sdlkey = SDLkeymap[wParam];

			   bool lshift = (GetKeyState(VK_LSHIFT) & 0x8000 );
			   if (lshift) sdlkey = SDLK_LSHIFT;
			   bool rshift = (GetKeyState(VK_LCONTROL) & 0x8000 );
			   if (rshift) sdlkey = SDLK_RSHIFT;
			   bool lctrl = (GetKeyState(VK_LCONTROL) & 0x8000 );
			   if (lctrl) sdlkey = SDLK_LCTRL;
			   bool rctrl = (GetKeyState(VK_RCONTROL) & 0x8000 );
			   if (rctrl) sdlkey = SDLK_RCTRL;
			/*   switch(wParam)
			   {
			   case VK_LSHIFT:
				   sdlmod |=0x0001;
				   break;
			   case VK_RSHIFT: sdlmod |= 0x0002; break;
			   case VK_LCONTROL: sdlmod |= 0x0040; break;
			   case VK_RCONTROL : sdlmod |= 0x0080; break;
			   case VK_LMENU:  sdlmod |= 0x0100; break;
			   case VK_RMENU:  sdlmod |= 0x0200; break ;
			   default:
				   sdlmod = 0x0000;
				   break;
			   }*/
			   m64p_error retval =emulator->CoreDoCommand(M64CMD_SEND_SDL_KEYDOWN ,MAKELPARAM(sdlkey,sdlmod), NULL);
			}
			return 0;
		}

		LRESULT OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
		    if (emulator->isEmulating)
			{
				DWORD sdlmod = 0;
				DWORD sdlkey = SDLkeymap[wParam];

				bool lshift = (GetKeyState(VK_LSHIFT) & 1 );
				if (lshift) sdlkey =SDLK_LSHIFT;
				bool lctrl = (GetKeyState(VK_RCONTROL) & 1 );
				if (lctrl) sdlkey =SDLK_LCTRL;
				bool rshift = (GetKeyState(VK_RSHIFT) & 1 );
				if (rshift) sdlkey =SDLK_LSHIFT;
				bool rctrl = (GetKeyState(VK_RCONTROL) & 1 );
				if (rctrl) sdlkey =SDLK_RCTRL;
				m64p_error retval = emulator->CoreDoCommand(M64CMD_SEND_SDL_KEYUP ,MAKELPARAM(sdlkey,sdlmod), NULL);
			}
			return 0;
		}

		LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			if (browser.RomBrowserVisible() && !browser.RomListNotify(wParam,lParam)) {
				return 0;
			}
			return 0;
		}

		void ProcessFile(LPCTSTR lpszPath)
		{
			string ansi = utf8_from_utf16(lpszPath);
			browser.hide();
			emulator->run_rom(ansi.c_str());
		}

		LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			if(emulator->isEmulating)emulator->close_rom();
			emulator->kill();
			CMessageLoop* pLoop = _Module.GetMessageLoop();
			ATLASSERT(pLoop != NULL);
			pLoop->RemoveMessageFilter(this);
			bHandled = FALSE;
			return 0;
		}

		LRESULT OnEndEmulation(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
		{
			if(emulator->isEmulating)
			{
				emulator->close_rom();
				browser.show();
			}
			return 0;
		}

		LRESULT OnHardReset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
		{
			if(emulator->isEmulating)
			{
				emulator->CoreDoCommand(M64CMD_RESET ,1, NULL);
			}
			return 0;
		}

		LRESULT OnSoftReset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
		{
			if(emulator->isEmulating)
			{
				emulator->CoreDoCommand(M64CMD_RESET ,0, NULL);
			}
			return 0;
		}

		LRESULT OnOptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
		{
			COptionsTab dlg;
			dlg.DoModal();
			return 0;
		}

		void OnFileExit(UINT uCode, int nID, HWND hwndCtrl)
		{
			DestroyWindow();
			return;
		}

		LRESULT OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
		{
			/*CHAR szFileName[MAX_PATH];
			LPCTSTR sFiles =
				L"N64 ROMs (*.n64,*.v64, *.z64)\0*.n64;*.v64;*.z64\0"
				L"All Files (*.*)\0*.*\0\0";
			CFileDialog dlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, sFiles);
			if (dlg.DoModal() == IDOK)
			{
				TCHAR strfilename[MAX_PATH]= {0};
				string ansi = utf8_from_utf16(dlg.m_szFileName);
				// do stuff
				//browser.hide();
				//emulator->run_rom(ansi.c_str());

			}*/ 
			return 0;
		}

		void OnAbout(UINT uCode, int nID, HWND hwndCtrl)
		{
			CAboutDlg dlg;
			dlg.DoModal();
			return;
		}
};

class CEmuMessageLoop : public CMessageLoop
{
public:
	int Run( CMyWindow & gamewnd )
	{
		BOOL bDoIdle = TRUE;
		int nIdleCount = 0;
		BOOL bRet = FALSE;

		for(;;)
		{

			if ( gamewnd.IsRunning() ) {
				while ( gamewnd.IsRunning() && !::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE) )
				{
					while(bDoIdle && !::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE))
					{
						if(!OnIdle(nIdleCount++))
							bDoIdle = FALSE;
					}

					gamewnd.DoFrame();
				}


			}
			else
			{
				while(bDoIdle && !::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE))
				{
					if(!OnIdle(nIdleCount++))
						bDoIdle = FALSE;
				}
			}

			bRet = ::GetMessage(&m_msg, NULL, 0, 0);

			if(bRet == -1)
			{
				ATLTRACE2(atlTraceUI, 0, _T("::GetMessage returned -1 (error)\n"));
				continue;   // error, don't process
			}
			else if(!bRet)
			{
				ATLTRACE2(atlTraceUI, 0, _T("CMessageLoop::Run - exiting\n"));
				break;   // WM_QUIT, exit message loop
			}

			if(!PreTranslateMessage(&m_msg))
			{
				::TranslateMessage(&m_msg);
				::DispatchMessage(&m_msg);
			}

			if(IsIdleMessage(&m_msg))
			{
				bDoIdle = TRUE;
				nIdleCount = 0;
			}
		}

		return (int)m_msg.wParam;
	}
};


#endif  // ndef MYWINDOW_H_INCLUDED
