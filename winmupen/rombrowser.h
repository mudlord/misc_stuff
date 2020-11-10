#ifndef ROMBROWSER_H_INCLUDED
#define ROMBROWSER_H_INCLUDED

#include <windows.h>
#include <Shlwapi.h>
#include "DropFileTarget.h"
#include "DlgTabCtrl.h"
#include "utf8conv.h"
#include <vector>
#include "mupenapi/mupencpp.h"

using namespace std;
using namespace utf8util;

//static CRomBrowser * CRomBrowser::m_Instance = 0;


class CRomBrowser
{
public:
	HWND m_hRomList;
	HWND parent_hwnd;

	typedef struct {
		TCHAR     FileNameNoExt[MAX_PATH];
		TCHAR     FileName[MAX_PATH];
		TCHAR     InternalName[22];
		BYTE     Country;
	}ROM_INFO;

	enum { IDC_ROMLIST = 223 };

	std::vector<ROM_INFO>m_RomInfo;
	static	CRomBrowser* m_Instance;

	static CRomBrowser* CreateInstance();
	static	CRomBrowser* GetSingleton( );


	void reset(HWND parent)
	{
		m_hRomList = CreateWindowEx( WS_EX_CLIENTEDGE,WC_LISTVIEW,NULL,
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | 
			LVS_SINGLESEL| LVS_REPORT | LVS_SHOWSELALWAYS ,
			0,0,640 , 480,parent,(HMENU)IDC_ROMLIST,GetModuleHandle(NULL),NULL);
		ListView_SetExtendedListViewStyle(m_hRomList, LVS_EX_FULLROWSELECT );
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

	void resize(int width, int height)
	{
		SetWindowPos(m_hRomList,0,0,0,width,height,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	}

	void rebuild_list()
	{
		CMupen64Plus *emulator_instance = CMupen64Plus::GetSingleton();
		const char *rom_dir = emulator_instance->ConfigGetParamString( emulator_instance->l_ConfigUI, "RomDir");
		wstring rom_w = utf16_from_utf8(rom_dir);

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

	}

	void build_list(bool reset_cache)
	{
		CMupen64Plus *emulator_instance = CMupen64Plus::GetSingleton();
		const char *rom_dir = emulator_instance->ConfigGetParamString( emulator_instance->l_ConfigUI, "RomDir");
		wstring rom_w = utf16_from_utf8(rom_dir);

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

		if (reset_cache)DeleteFile(cache_filename);

		rom_path = rom_w.c_str();
		rom_path += "\\*";

		WIN32_FIND_DATA ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		hFind = FindFirstFile(rom_path, &ffd);
		int i=0;
		if (hFind == INVALID_HANDLE_VALUE)
		{
			return;
		} 
		m_RomInfo.clear();
		do
		{
			if (ffd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY && CMupen64Plus::IsValidROM(ffd.cFileName))
			{
				ROM_INFO entry;
				m64p_rom_header header;


				TCHAR rom_name[MAX_PATH] = {0};
				lstrcpy(rom_name,ffd.cFileName);
				PathRemoveExtension(rom_name);
				wstring rompath;
				rompath = rom_w.c_str();
				rompath += L"\\";
				rompath += ffd.cFileName;
				m64p_error result = CMupen64Plus::getRomHeader((TCHAR*)rompath.c_str(), &header);
				if (result != M64ERR_SUCCESS)
				{
					continue;
				}
				wstring internal_str = utf16_from_utf8((char*)header.Name);			
				TCHAR internal_name[22] = {0};
				lstrcpy(internal_name,internal_str.c_str());

				//pack data into vector
				entry.Country = header.Country_code;
				lstrcpy(entry.FileName,rompath.c_str());
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
		instance->run_rom(utf8.c_str());
		if(!instance->use_videoext)show();

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

	void fill_list()
	{
		CMupen64Plus *emulator_instance = CMupen64Plus::GetSingleton();
		const char *rom_dir = emulator_instance->ConfigGetParamString( emulator_instance->l_ConfigUI, "RomDir");
		wstring rom_w = utf16_from_utf8(rom_dir);
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
			build_list(false);
			return;
		}
		ListView_DeleteAllItems(m_hRomList);
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



#endif ROMBROWSER_H_INCLUDED