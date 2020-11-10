#ifndef MYWINDOW_H_INCLUDED
#define MYWINDOW_H_INCLUDED

#include <windows.h>
#include <Shlwapi.h>
#include "DropFileTarget.h"
#include "DlgTabCtrl.h"
#include "utf8conv.h"
#include <vector>

#include "mupenapi/osal_dynamiclib.h"
#include "mupenapi/mupencpp.h"
#include "svn_version.h"

#include "pluginsview.h"
#include "rombrowser.h"

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

	CComboBox video_plug,input_plug,rsp_plug,audio_plug,core_type;
	CButton compiled_jump, expansion_pak, autostateincrement,osd,use_vidext;
	CMupen64Plus *emulator_instance;

	vector<CMupen64Plus::plugin_map_node>plugins;

	

	LRESULT OnInitDialogView1(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		video_plug = GetDlgItem(IDC_VIDEOPLUGIN);
		input_plug=GetDlgItem(IDC_INPUTPLUGIN);
		rsp_plug=GetDlgItem(IDC_RSPPLUGIN);
		audio_plug=GetDlgItem(IDC_AUDIOPLUGIN);
		core_type=GetDlgItem(IDC_CORETYPE);
		compiled_jump=GetDlgItem(IDC_CORE_NOCOMPILEDJUMP);
		expansion_pak=GetDlgItem(IDC_CORE_NO8MBPAK);
		autostateincrement=GetDlgItem(IDC_CORE_AUTOSLOTINCREMENT);
		osd=GetDlgItem(IDC_CORE_OSD);
		use_vidext=GetDlgItem(IDC_VIDEXT);

		emulator_instance =  CMupen64Plus::GetSingleton( ) ;

		load_settings();
		return TRUE;
	}

	LRESULT OnSaveSettings(UINT /*uMsg*/, WPARAM /*wParam*/, HWND /*lParam*/, BOOL& /*bHandled*/)
	{
		save_settings();
		return 0;
	}

	bool load_plugin(char *filename)
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
		/* call the plugin's initialization function and make sure it starts okay */
		osal_dynlib_close(handle);
		/* plugin loaded successfully, so set the plugin map's members */

		CMupen64Plus::plugin_map_node entry;
		//wstring filename2 = (char*)filename;
		lstrcpy(entry.filename, (TCHAR*)filename);
		entry.type = PluginType;
		entry.libversion = PluginVersion;
		plugins.push_back(entry);
		return true;
	}

	m64p_plugin_type plugin_type(const char* filename)
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

	void load_settings()
	{
		const char *plugindir = emulator_instance->ConfigGetParamString(emulator_instance->l_ConfigUI, "PluginDir");
		wstring plugindir_w = utf16_from_utf8(plugindir);
		plugindir_w += L"\\*.dll";

		WIN32_FIND_DATA ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		hFind = FindFirstFile(plugindir_w.c_str(), &ffd);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			return;
		} 
		do
		{
				if ( plugin_type((const char*)ffd.cFileName) != M64PLUGIN_CORE)
				{
					bool load = load_plugin((char*)ffd.cFileName);
				}
		}
		while (FindNextFile(hFind, &ffd) != 0);

		//video plugins
		for (int i=0;i<plugins.size();i++)
		{
			if(plugins[i].type == M64PLUGIN_GFX)
			{
				video_plug.AddString(plugins[i].filename);
			}
		}
	
		const char *video_path =  emulator_instance->ConfigGetParamString( emulator_instance->l_ConfigUI, "VideoPlugin");
		wstring video_plugin = utf16_from_utf8(video_path);
		video_plug.SelectString(0,video_plugin.c_str());

		//audio
		for (int i=0;i<plugins.size();i++)
		{
			if(plugins[i].type == M64PLUGIN_AUDIO)
			{
				audio_plug.AddString(plugins[i].filename);
			}
		}
		const char *audio_path =  emulator_instance->ConfigGetParamString( emulator_instance->l_ConfigUI, "AudioPlugin");
		wstring audio_plugin = utf16_from_utf8(audio_path);
		audio_plug.SelectString(0,audio_plugin.c_str());
		//input
		for (int i=0;i<plugins.size();i++)
		{
			if(plugins[i].type == M64PLUGIN_INPUT)
			{
				input_plug.AddString(plugins[i].filename);
			}
		}
		const char *input_path =  emulator_instance->ConfigGetParamString( emulator_instance->l_ConfigUI, "InputPlugin");
		wstring input_plugin = utf16_from_utf8(input_path);
		input_plug.SelectString(0,input_plugin.c_str());
		//rsp
		for (int i=0;i<plugins.size();i++)
		{
			if(plugins[i].type == M64PLUGIN_RSP)
			{
				rsp_plug.AddString(plugins[i].filename);
			}
		}
		const char *rsp_path =  emulator_instance->ConfigGetParamString( emulator_instance->l_ConfigUI, "RspPlugin");
		wstring rsp_plugin = utf16_from_utf8(rsp_path);
		rsp_plug.SelectString(0,rsp_plugin.c_str());

		int vidext = emulator_instance->ConfigGetParamBool(emulator_instance->l_ConfigUI,"UseVideoExtension");
		use_vidext.SetCheck(vidext);
		//core
		int onscreen_display = emulator_instance->ConfigGetParamBool(emulator_instance->l_ConfigCore,"OnScreenDisplay");
		int nocompiledjump =  emulator_instance->ConfigGetParamBool(emulator_instance->l_ConfigCore,"NoCompiledJump");
	    int expansionpak =  emulator_instance->ConfigGetParamBool(emulator_instance->l_ConfigCore,"DisableExtraMem");
		int autoslot = emulator_instance->ConfigGetParamBool(emulator_instance->l_ConfigCore,"AutoStateSlotIncrement");

		compiled_jump.SetCheck(nocompiledjump);
		expansion_pak.SetCheck(expansionpak);
		autostateincrement.SetCheck(autoslot);
		osd.SetCheck(onscreen_display);
		int core = emulator_instance->ConfigGetParamInt(emulator_instance->l_ConfigCore,"R4300Emulator");

		core_type.AddString(L"Interpreter (pure)");
		core_type.AddString(L"Interpreter (cached)");
		core_type.AddString(L"Dynamic Recompiler");
		core_type.SetCurSel(core);	
	}

	void save_settings()
	{
		
		CString video_plugin;
		int selection = video_plug.GetCurSel();
		int n = video_plug.GetLBTextLen(selection);
		video_plug.GetLBText(selection,video_plugin.GetBuffer(n));
		string vid_plugin = utf8_from_utf16(video_plugin.GetBuffer(0));
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigUI, "VideoPlugin",M64TYPE_STRING, vid_plugin.c_str());
		CString audio_plugin;
		selection = audio_plug.GetCurSel();
		n = audio_plug.GetLBTextLen(selection);
		audio_plug.GetLBText(selection,audio_plugin.GetBuffer(n));
		string aud_plugin = utf8_from_utf16(audio_plugin.GetBuffer(0));
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigUI, "AudioPlugin",M64TYPE_STRING, aud_plugin.c_str());
		CString input_plugin;
		selection = input_plug.GetCurSel();
		n = input_plug.GetLBTextLen(selection);
		input_plug.GetLBText(selection,input_plugin.GetBuffer(n));
		string in_plugin = utf8_from_utf16(input_plugin.GetBuffer(0));
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigUI, "InputPlugin",M64TYPE_STRING, in_plugin.c_str());
		CString rsp_plugin;
		selection = rsp_plug.GetCurSel();
		n = rsp_plug.GetLBTextLen(selection);
		rsp_plug.GetLBText(selection,rsp_plugin.GetBuffer(n));
		string rsp_plugin2 = utf8_from_utf16(rsp_plugin.GetBuffer(0));
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigUI, "RspPlugin",M64TYPE_STRING, rsp_plugin2.c_str());

		int vidext = use_vidext.GetCheck();
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigUI,"UseVideoExtension",M64TYPE_BOOL, &vidext);
		//core settings
		int onscreen_display = osd.GetCheck();
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigCore,"OnScreenDisplay",M64TYPE_BOOL, &onscreen_display);
		int nocompiledjump = compiled_jump.GetCheck();
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigCore,"NoCompiledJump",M64TYPE_BOOL, &nocompiledjump);
		int expansionpak = expansion_pak.GetCheck();
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigCore,"DisableExtraMem",M64TYPE_BOOL, &expansionpak);
		int autoslot = autostateincrement.GetCheck();
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigCore,"AutoStateSlotIncrement",M64TYPE_BOOL, &autoslot);
		int coretype = core_type.GetCurSel();
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigCore,"R4300Emulator ",M64TYPE_INT, &coretype);
		m64p_error retval =emulator_instance->ConfigSaveFile();
		
	}

	

};

class CPathsView : public CDialogImpl<CPathsView>
{
public:
	HWND m_hwndOwner; 
	enum { IDD = IDD_PATHS };
	BEGIN_MSG_MAP( CPathsView)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialogView2)
		COMMAND_ID_HANDLER(IDC_SSHOTSDIR, OnBrowseScreenShots)
		COMMAND_ID_HANDLER(IDC_SSTATESDIR, OnBrowseSaves)
		COMMAND_ID_HANDLER(IDC_SDATADIR, OnBrowseSharedData)
		COMMAND_ID_HANDLER(IDC_PLUGINSDIR, OnBrowsePlugins)
		COMMAND_ID_HANDLER(IDC_ROMSDIR, OnBrowseROMs)
	END_MSG_MAP()

	CStatic sshots, sstates, sdata, plugins, roms;
	CButton sshots_but, sstates_but, sdata_but, plugins_but, roms_but;
	CMupen64Plus *emulator_instance;
	

	LRESULT OnInitDialogView2(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		sshots = GetDlgItem(IDC_SSHOTS);
		sshots_but = GetDlgItem(IDC_SSHOTSDIR);
		sstates = GetDlgItem(IDC_SSTATES);
		sstates_but = GetDlgItem(IDC_SSTATESDIR);
		sdata = GetDlgItem(IDC_SDATA);
		sdata_but = GetDlgItem(IDC_SDATADIR);
		plugins = GetDlgItem(IDC_PLUGINS);
		plugins_but = GetDlgItem(IDC_PLUGINSDIR);
		roms = GetDlgItem(IDC_ROMS);
		roms_but = GetDlgItem(IDC_ROMSDIR);

		emulator_instance = CMupen64Plus::GetSingleton();

		const char *sshots_path =  emulator_instance->ConfigGetParamString( emulator_instance->l_ConfigCore, "ScreenshotPath");
		wstring sshots_w = utf16_from_utf8(sshots_path);
		sshots.SetWindowTextW(sshots_w.c_str());
		const char *sstates_path =  emulator_instance->ConfigGetParamString( emulator_instance->l_ConfigCore, "SaveStatePath");
		wstring sstates_w = utf16_from_utf8(sstates_path);
		sstates.SetWindowTextW(sstates_w.c_str());
		const char *sdata_path =  emulator_instance->ConfigGetParamString( emulator_instance->l_ConfigCore, "SharedDataPath");
		wstring sdata_w = utf16_from_utf8(sdata_path);
		sdata.SetWindowTextW(sdata_w.c_str());
		const char *plugin_dir = emulator_instance->ConfigGetParamString( emulator_instance->l_ConfigUI, "PluginDir");
		wstring plugin_w = utf16_from_utf8(plugin_dir);
		plugins.SetWindowTextW(plugin_w.c_str());
		const char *rom_dir = emulator_instance->ConfigGetParamString( emulator_instance->l_ConfigUI, "RomDir");
		wstring rom_w = utf16_from_utf8(rom_dir);
		roms.SetWindowTextW(rom_w.c_str());
		return TRUE;
	}


	LRESULT OnBrowseScreenShots(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		TCHAR folderpath[MAX_PATH];
		BROWSEINFO bi;
		memset(&bi, 0, sizeof(bi));
		int retVal = false;
		bi.ulFlags = BIF_USENEWUI;
		bi.lpszTitle = L"Select the directory that will hold your screenshots...";
		::OleInitialize(NULL);
		LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);

		if(pIDL != NULL)
		{
			// Create a buffer to store the path, then
			// get the path.
			TCHAR buffer[_MAX_PATH] = {'\0'};
			if(::SHGetPathFromIDList(pIDL, buffer) != 0)
			{
				// Set the string value.
				_tcscpy(folderpath,buffer);
				retVal = true;
			}

			// free the item id list
			CoTaskMemFree(pIDL);
		}
		::OleUninitialize();
		if (retVal)
		{	
			sshots.SetWindowText(folderpath);
		}
		return 0;
	}

	LRESULT OnBrowseSaves(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		TCHAR folderpath[MAX_PATH];
		BROWSEINFO bi;
		memset(&bi, 0, sizeof(bi));
		int retVal = false;
		bi.ulFlags = BIF_USENEWUI;
		bi.lpszTitle = L"Select the directory that will hold your save states/game saves...";
		::OleInitialize(NULL);
		LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);

		if(pIDL != NULL)
		{
			// Create a buffer to store the path, then
			// get the path.
			TCHAR buffer[_MAX_PATH] = {'\0'};
			if(::SHGetPathFromIDList(pIDL, buffer) != 0)
			{
				// Set the string value.
				_tcscpy(folderpath,buffer);
				retVal = true;
			}

			// free the item id list
			CoTaskMemFree(pIDL);
		}
		::OleUninitialize();
		if (retVal)
		{	
			sstates.SetWindowText(folderpath);
		}
		return 0;
	}

	LRESULT OnBrowseSharedData(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		TCHAR folderpath[MAX_PATH];
		BROWSEINFO bi;
		memset(&bi, 0, sizeof(bi));
		int retVal = false;
		bi.ulFlags = BIF_USENEWUI;
		bi.lpszTitle = L"Select the directory that will hold the emulator's shared data files...";
		::OleInitialize(NULL);
		LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);

		if(pIDL != NULL)
		{
			// Create a buffer to store the path, then
			// get the path.
			TCHAR buffer[_MAX_PATH] = {'\0'};
			if(::SHGetPathFromIDList(pIDL, buffer) != 0)
			{
				// Set the string value.
				_tcscpy(folderpath,buffer);
				retVal = true;
			}

			// free the item id list
			CoTaskMemFree(pIDL);
		}
		::OleUninitialize();
		if (retVal)
		{	
			sdata.SetWindowText(folderpath);
		}
		return 0;
	}

	LRESULT OnBrowsePlugins(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		TCHAR folderpath[MAX_PATH];
		BROWSEINFO bi;
		memset(&bi, 0, sizeof(bi));
		int retVal = false;
		bi.ulFlags = BIF_USENEWUI;
		bi.lpszTitle = L"Select the directory that will hold your plugins...";
		::OleInitialize(NULL);
		LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);

		if(pIDL != NULL)
		{
			// Create a buffer to store the path, then
			// get the path.
			TCHAR buffer[_MAX_PATH] = {'\0'};
			if(::SHGetPathFromIDList(pIDL, buffer) != 0)
			{
				// Set the string value.
				_tcscpy(folderpath,buffer);
				retVal = true;
			}

			// free the item id list
			CoTaskMemFree(pIDL);
		}
		::OleUninitialize();
		if (retVal)
		{	
			plugins.SetWindowText(folderpath);
		}
		return 0;
	}

	LRESULT OnBrowseROMs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		TCHAR folderpath[MAX_PATH];
		BROWSEINFO bi;
		memset(&bi, 0, sizeof(bi));
		int retVal = false;
		bi.ulFlags = BIF_USENEWUI;
		bi.lpszTitle = L"Select the directory that will hold your ROMs...";
		::OleInitialize(NULL);
		LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);

		if(pIDL != NULL)
		{
			// Create a buffer to store the path, then
			// get the path.
			TCHAR buffer[_MAX_PATH] = {'\0'};
			if(::SHGetPathFromIDList(pIDL, buffer) != 0)
			{
				// Set the string value.
				_tcscpy(folderpath,buffer);
				retVal = true;
			}

			// free the item id list
			CoTaskMemFree(pIDL);
		}
		::OleUninitialize();
		if (retVal)
		{	
			roms.SetWindowText(folderpath);
		}
		return 0;
	}

	void save_settings()
	{
		CRomBrowser *browser = CRomBrowser::GetSingleton();
		
		TCHAR sshots_str[MAX_PATH] ={0};
		sshots.GetWindowText(sshots_str,MAX_PATH);
		TCHAR sstates_str[MAX_PATH] ={0};
		sstates.GetWindowText(sstates_str,MAX_PATH);
		TCHAR sdata_str[MAX_PATH] ={0};
		sdata.GetWindowText(sdata_str,MAX_PATH);
		TCHAR plugins_str[MAX_PATH] ={0};
		plugins.GetWindowText(plugins_str,MAX_PATH);
		TCHAR roms_str[MAX_PATH] ={0};
		roms.GetWindowText(roms_str,MAX_PATH);

		string converted = utf8_from_utf16(sshots_str);
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigCore, "ScreenshotPath",M64TYPE_STRING, converted.c_str());
		converted = utf8_from_utf16(sstates_str);
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigCore, "SaveStatePath",M64TYPE_STRING, converted.c_str());
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigCore, "SaveSRAMPath",M64TYPE_STRING, converted.c_str());
		converted = utf8_from_utf16(sdata_str);
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigCore, "SharedDataPath",M64TYPE_STRING, converted.c_str());
		converted = utf8_from_utf16(plugins_str);
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigUI, "PluginDir",M64TYPE_STRING, converted.c_str());
		converted = utf8_from_utf16(roms_str);
		emulator_instance->ConfigSetParameter(emulator_instance->l_ConfigUI, "RomDir",M64TYPE_STRING, converted.c_str());
		
		m64p_error retval =emulator_instance->ConfigSaveFile();
		browser->build_list(true);

	}

	LRESULT OnSavePaths(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		save_settings();
		return 0;
	}
};

class COptionsTab : public CDialogImpl<COptionsTab>
{
public:
	CDialogTabCtrl m_ctrlTab;
	CPluginView m_view1;
	CPathsView m_view2;
	enum { IDD = IDD_SETTINGS };

	BEGIN_MSG_MAP( COptionsTab)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		REFLECT_NOTIFICATIONS();
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();
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

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		m_view1.save_settings();
		m_view2.save_settings();
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
		CString verinf0 = SVN_REVISION;
		CString date = SVN_TIME_NOW;
		builddate = GetDlgItem(IDC_BUILDDATE);
		builddate.SetWindowText(date);
		version_number = GetDlgItem(IDC_APPVER);
		version_number.SetWindowText(verinf0);
		website.SubclassWindow(GetDlgItem(IDC_LINK));
		website.SetHyperLink(_T("http://vba-m.com/forum/Forum-other-stuff"));
		return TRUE;
	}
	LRESULT OnButtonOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}
	LRESULT OnStnClickedSdlcred(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};



class CMyWindow : public CFrameWindowImpl<CMyWindow>, public CDropFileTarget<CMyWindow>,public CMessageFilter
{
public:
	DECLARE_FRAME_WND_CLASS ( _T("emu_wtl"), IDR_MAINFRAME );
	BEGIN_MSG_MAP_EX(CMyWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_NOTIFY,OnNotify)
		MESSAGE_HANDLER(WM_SIZE,OnSize)

		COMMAND_ID_HANDLER_EX(IDC_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_RESET_HARDRESET,OnHardReset)
	    COMMAND_ID_HANDLER(ID_RESET_SOFTRESET,OnSoftReset)
		MESSAGE_HANDLER(WM_SYSKEYDOWN,OnFullscreen)
		COMMAND_ID_HANDLER(ID_OPTIONS_LOADSTATE, OnStateLoad)
		COMMAND_ID_HANDLER(ID_OPTIONS_SAVESTATE, OnStateSave)
		COMMAND_ID_HANDLER(ID_OPTIONS_PAUSE,OnPause);
		COMMAND_ID_HANDLER(ID_CONFIGUREPLUGINS_AUDIO,OnPluginAudioSDL)
		COMMAND_ID_HANDLER(ID_CONFIGUREPLUGINS_INPUT,OnPluginInputSDL)
		COMMAND_ID_HANDLER(ID_CONFIGUREPLUGINS_GRAPHICS,OnPluginVideo)
		COMMAND_ID_HANDLER_EX(IDC_ABOUT, OnAbout)
		COMMAND_ID_HANDLER(ID_EMULATION_END,OnEndEmulation);
		COMMAND_ID_HANDLER(ID_OPTIONS_PREFERENCES, OnOptions)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMyWindow>)
		CHAIN_MSG_MAP(CDropFileTarget<CMyWindow>)
		END_MSG_MAP()

		CRomBrowser *browser;
		CMupen64Plus *emulator;

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

		LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			int nWidth = LOWORD(lParam); 
			int nHeight = HIWORD(lParam);
			browser->resize(nWidth,nHeight);
			return 0;
		}

		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			CMessageLoop* pLoop = _Module.GetMessageLoop();
			ATLASSERT(pLoop != NULL);
			bHandled = FALSE;
			RegisterDropTarget();
			emulator = CMupen64Plus::CreateInstance() ;
			browser = CRomBrowser::CreateInstance();
			bool res = emulator->init(m_hWnd);
			if (!res)
			{
				MessageBox(L"Cannot initialize core libraries!\nThis frontend will now exit.",L"Error",MB_ICONSTOP);
				ExitProcess(0);
				
			}
			browser->reset(m_hWnd);

			return 0;
		}

		LRESULT OnFullscreen(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
		   if (emulator->isEmulating)
		   {
			   switch( wParam )
			   {
			   case VK_RETURN:
				   {
					   // Toggle full screen upon alt-enter 
					   DWORD dwMask = ( 1 << 29 );
					   if( ( lParam & dwMask ) != 0 ) // Alt is down also
					   {
						   // Toggle the full screen/window mode
						   int i = 0;
						 //  g_app_->render_->toggleFullScreen();
						   return 0;
					   }
				   }
				    break;
			   }
			  
		
			}
			return 0;
		}

		LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			if (browser->RomBrowserVisible() && !browser->RomListNotify(wParam,lParam)) {
				return 0;
			}
			return 0;
		}

		void ProcessFile(LPCTSTR lpszPath)
		{
			string ansi = utf8_from_utf16(lpszPath);
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
				browser->show();
			}
			return 0;
		}

		LRESULT OnHardReset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
		{
			if(emulator->isEmulating)
			{
				emulator->CoreDoCommand(M64CMD_RESET,1, NULL);
			}
			return 0;
		}

		LRESULT OnSoftReset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
		{
			if(emulator->isEmulating)
			{
				emulator->CoreDoCommand(M64CMD_RESET,0, NULL);
			}
			return 0;
		}

		LRESULT OnPause(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
		{
			if(emulator->isEmulating)
			{
				int paused = 0;
				emulator->CoreDoCommand(M64CMD_CORE_STATE_QUERY, M64CORE_EMU_STATE , &paused);
				switch (paused)
				{
				case 1:
				break;
				case 2:
					emulator->CoreDoCommand(M64CMD_PAUSE,0 ,0);
					break;
				case 3:
					emulator->CoreDoCommand(M64CMD_RESUME,0 ,0);
				}
	
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

		LRESULT OnStateLoad(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
		{
			LPCTSTR sFiles =
				L"Save states (*.m64p,*.pj)\0*.m64p;*.pj\0"
				L"All Files (*.*)\0*.*\0\0";
			CFileDialog dlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, sFiles);
			if (dlg.DoModal() == IDOK)
			{
				TCHAR strfilename[MAX_PATH]= {0};
				string filename = utf8_from_utf16(dlg.m_szFileName);
				if(emulator->isEmulating)
				{
					emulator->CoreDoCommand(M64CMD_STATE_LOAD, NULL, (char*)filename.c_str());
				}

			}
			return 0;
		}

		LRESULT OnStateSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
		{
			LPCTSTR sFiles=	L"Save states (*.m64p)\0*.m64p\0\0";
			CFileDialog dlg( FALSE, L"m64p", L"*.m64p", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, sFiles);
			if (dlg.DoModal() == IDOK)
			{
				TCHAR strfilename[MAX_PATH]= {0};
				string filename = utf8_from_utf16(dlg.m_szFileName);
				if(emulator->isEmulating)
				{
					emulator->CoreDoCommand(M64CMD_STATE_SAVE, 1, (char*)filename.c_str());
				}
			}
			return 0;
		}

		LRESULT OnPluginAudioSDL(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
		{
			CPluginAudioSDL dlg;
			dlg.DoModal();
			return 0;
		}

		LRESULT OnPluginVideo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
		{
			CPluginVideo dlg;
			dlg.DoModal();
			return 0;
		}

		LRESULT OnPluginInputSDL(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
		{
			if(!emulator->isEmulating){
			CPluginInputSDL dlg;
			dlg.DoModal();
			}
			else
			MessageBox(L"Cannot change keys while emulation is running!",L"ERROR",MB_ICONSTOP);

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
