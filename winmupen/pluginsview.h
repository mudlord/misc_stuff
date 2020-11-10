#ifndef PLUGINVIEW_H_INCLUDED
#define PLUGINVIEW_H_INCLUDED

#include <windows.h>
#include <Shlwapi.h>
#include "DropFileTarget.h"
#include "DlgTabCtrl.h"
#include "utf8conv.h"
#include <vector>

#include "SDL2/SDL.h"
#include "mupenapi/sdl_key_converter.h"



using namespace std;
using namespace utf8util;

class CPluginAudioSDL : public CDialogImpl<CPluginAudioSDL>
{
public:
	HWND m_hwndOwner; 
	enum { IDD = IDD_AUDIO };

	BEGIN_MSG_MAP(CPluginAudioSDL)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialogView)
		COMMAND_ID_HANDLER(IDCANCEL,OnButtonCancel)
		COMMAND_ID_HANDLER(IDOK,OnButtonOK)
		
	END_MSG_MAP()

	m64p_handle l_ConfigAudio;
	CEdit default_freq, primbuf, primbufsize,secbuf,voladjust,defvolume;
	CButton swap_channels;
	CComboBox resampler;
	CMupen64Plus *emulator_instance;

	

	LRESULT OnInitDialogView(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		emulator_instance =  CMupen64Plus::GetSingleton( ) ;
		wstring resample_str[17] = {L"src-sinc-best-quality",L"src-sinc-medium-quality", L"src-sinc-fastest", L"src-zero-order-hold", L"src-linear", 
		L"speex-fixed-0",L"speex-fixed-1",L"speex-fixed-2",L"speex-fixed-3",L"speex-fixed-4",L"speex-fixed-5",L"speex-fixed-6",L"speex-fixed-7",L"speex-fixed-8",L"speex-fixed-9",
		L"speex-fixed-10", L"trivial"};

		m64p_error rval;

		default_freq = GetDlgItem(IDC_SDL_AUDFREQ);
		primbuf = GetDlgItem(IDC_SDL_PRIMBUF);
		primbufsize = GetDlgItem(IDC_SDL_PRIMBUFTARGET);
		secbuf = GetDlgItem(IDC_SDL_SECBUFFERSIZE);
		voladjust = GetDlgItem(IDC_SDL_VOLADJUST);
		defvolume = GetDlgItem(IDC_SDL_DEFVOLUME);
		swap_channels = GetDlgItem(IDC_SDL_SWAPAUDIO);
		resampler = GetDlgItem(IDC_SDL_RESAMPLER);

		/* Open Configuration sections for core library and console User Interface */
		rval = emulator_instance->ConfigOpenSection("Audio-SDL", &l_ConfigAudio);

		TCHAR strings[64]={0};

		int deffreq = emulator_instance->ConfigGetParamInt(l_ConfigAudio,"DEFAULT_FREQUENCY");
		wsprintf(strings,L"%i",deffreq);
		default_freq.SetWindowText(strings);
		int swapchannels = emulator_instance->ConfigGetParamBool(l_ConfigAudio,"SWAP_CHANNELS");
		swap_channels.SetCheck(swapchannels);
		int primary_buffer = emulator_instance->ConfigGetParamInt(l_ConfigAudio,"PRIMARY_BUFFER_SIZE");
		wsprintf(strings,L"%i",primary_buffer);
		primbuf.SetWindowText(strings);
		int primbuftarget = emulator_instance->ConfigGetParamInt(l_ConfigAudio,"PRIMARY_BUFFER_TARGET");
		wsprintf(strings,L"%i",primbuftarget);
		primbufsize.SetWindowText(strings);
		int secondary_buffer = emulator_instance->ConfigGetParamInt(l_ConfigAudio,"SECONDARY_BUFFER_SIZE");
		wsprintf(strings,L"%i",secondary_buffer);
		secbuf.SetWindowText(strings);
		const char *resample =  emulator_instance->ConfigGetParamString( l_ConfigAudio, "RESAMPLE");
		wstring resample_pick = utf16_from_utf8(resample);
		for (int i=0;i<17;i++)resampler.AddString(resample_str[i].c_str());
		int j;
		for (j=0;j<17;j++)if (resample_pick == resample_str[j])break;
		resampler.SetCurSel(j);
		int volumeadjust = emulator_instance->ConfigGetParamInt(l_ConfigAudio,"VOLUME_ADJUST");
		wsprintf(strings,L"%i",volumeadjust);
		voladjust.SetWindowText(strings);
		int volumedefault = emulator_instance->ConfigGetParamInt(l_ConfigAudio,"VOLUME_DEFAULT");
		wsprintf(strings,L"%i",volumedefault);
		defvolume.SetWindowText(strings);

		return 0;

	}

	LRESULT OnButtonCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnButtonOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		wstring resample_str[17] = {L"src-sinc-best-quality",L"src-sinc-medium-quality", L"src-sinc-fastest", L"src-zero-order-hold", L"src-linear", 
			L"speex-fixed-0",L"speex-fixed-1",L"speex-fixed-2",L"speex-fixed-3",L"speex-fixed-4",L"speex-fixed-5",L"speex-fixed-6",L"speex-fixed-7",L"speex-fixed-8",L"speex-fixed-9",
			L"speex-fixed-10", L"trivial"};

		TCHAR strings[64]={0};
		default_freq.GetWindowText(strings,64);
		int deffreq = _wtol(strings);
		emulator_instance->ConfigSetParameter(l_ConfigAudio,"DEFAULT_FREQUENCY",M64TYPE_INT, &deffreq);
		int swapchannels = swap_channels.GetCheck();
		emulator_instance->ConfigSetParameter(l_ConfigAudio,"SWAP_CHANNELS",M64TYPE_BOOL, &swapchannels);
		primbuf.GetWindowText(strings,64);
		int primbuf_i = _wtol(strings);
		emulator_instance->ConfigSetParameter(l_ConfigAudio,"PRIMARY_BUFFER_SIZE",M64TYPE_INT, &primbuf_i);
		primbufsize.GetWindowText(strings,64);
		int primbuftarget_i = _wtol(strings);
		emulator_instance->ConfigSetParameter(l_ConfigAudio,"PRIMARY_BUFFER_TARGET",M64TYPE_INT, &primbuftarget_i);
		secbuf.GetWindowText(strings,64);
		int secsize_i = _wtol(strings);
		emulator_instance->ConfigSetParameter(l_ConfigAudio,"SECONDARY_BUFFER_SIZE",M64TYPE_INT, &secsize_i);
		int sel = resampler.GetCurSel();
		string resample = utf8_from_utf16(resample_str[sel]);
		emulator_instance->ConfigSetParameter(l_ConfigAudio, "RESAMPLE",M64TYPE_STRING, resample.c_str());
		voladjust.GetWindowText(strings,64);
		int voladjust_i = _wtol(strings);
		emulator_instance->ConfigSetParameter(l_ConfigAudio,"VOLUME_ADJUST",M64TYPE_INT, &voladjust_i);
		defvolume.GetWindowText(strings,64);
		int voldef_i = _wtol(strings);
		emulator_instance->ConfigSetParameter(l_ConfigAudio,"VOLUME_DEFAULT",M64TYPE_INT, &voldef_i);
		m64p_error retval =emulator_instance->ConfigSaveFile();

		
		EndDialog(wID);
		return 0;
	}

};

#include "pressbutton.h"
class CPluginInputKeys
{
public:
	
	int error;
	TCHAR str[50];

	void poll(int fulldef = 1)
	{
		SDL_Window *window;
		SDL_Renderer *renderer;
		SDL_RWops *file;
		SDL_Surface *image;
		SDL_Texture* image_tex;
			 if (!SDL_WasInit(SDL_INIT_VIDEO))
	 {
      int result = SDL_InitSubSystem(SDL_INIT_VIDEO);
	 }
	if (!SDL_WasInit(SDL_INIT_JOYSTICK))
	 {
      int result = SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	 }
		error = SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
		SDL_GetError();
		memset(str,0,sizeof(str));
		SDL_JoystickEventState(SDL_ENABLE);
		for (int i=0; i<SDL_NumJoysticks(); i++) 
		{
			m_joysticks.push_back(SDL_JoystickOpen(i));
		}
		done = false;
		window = SDL_CreateWindow("Select Buttons",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			320, 240,
			SDL_WINDOW_SHOWN);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
		file = SDL_RWFromMem(pressbutton,pressbutton_len);
		image = SDL_LoadBMP_RW(file, 1);
		image_tex = SDL_CreateTextureFromSurface(renderer, image);
		SDL_FreeSurface(image);  // we got the texture now -> free surface
		bool done = false;
		while(done == false)
		{
			SDL_Rect offset;
			//Give the offsets to the rectangle
			offset.x = 0;
			offset.y = 0;
			offset.w = 320;
			offset.h=240;
			//Blit the surface
			SDL_RenderCopy(renderer, image_tex, &offset, &offset);
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_KEYDOWN && event.key.state == SDL_PRESSED)
				{
					if (fulldef)
					wsprintf(str,L"key(%i)",sdl_scancode2keysym(event.key.keysym.scancode));
					else
					{
						wsprintf(str,L"%i",sdl_scancode2keysym(event.key.keysym.scancode));
					}
					done = true;

				}
			}
			  
			SDL_RenderPresent(renderer);

		}
		

		if(done)
		{
			for (unsigned int i=0; i<m_joysticks.size(); i++) 
			{
				SDL_JoystickClose(m_joysticks[i]);
			}

			// Close the frame
			SDL_DestroyTexture(image_tex);
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
			SDL_QuitSubSystem(SDL_INIT_VIDEO);
			SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		}

	}
	
private:
	bool done;
	UINT m_iTimerID;
	std::vector<SDL_Joystick*> m_joysticks;
};



class CPluginInputSDL1 : public CDialogImpl<CPluginInputSDL1>
{
public:
	enum { IDD = IDD_SDLINPUT1};
	BEGIN_MSG_MAP(CPluginInputSDL1)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialogView)

		COMMAND_HANDLER_EX(IDC_DPADDOWN1, EN_SETFOCUS, OnDPadDown)
		COMMAND_HANDLER_EX(IDC_DPADUP1, EN_SETFOCUS, OnDPadUp)
		COMMAND_HANDLER_EX(IDC_DPADLEFT1, EN_SETFOCUS, OnDPadLeft)
		COMMAND_HANDLER_EX(IDC_DPADRIGHT1, EN_SETFOCUS, OnDPadRight)
		COMMAND_HANDLER_EX(IDC_STARTINPUT1, EN_SETFOCUS, OnStart)
		COMMAND_HANDLER_EX(IDC_ZINPUT1, EN_SETFOCUS, OnZ)
		COMMAND_HANDLER_EX(IDC_AINPUT1, EN_SETFOCUS, OnA)
		COMMAND_HANDLER_EX(IDC_BINPUT1, EN_SETFOCUS, OnB)
		COMMAND_HANDLER_EX(IDC_LINPUT1, EN_SETFOCUS, OnL)
		COMMAND_HANDLER_EX(IDC_RINPUT1, EN_SETFOCUS, OnR)
		COMMAND_HANDLER_EX(IDC_MEMPAK1, EN_SETFOCUS, OnMempak)
		COMMAND_HANDLER_EX(IDC_RUMBLEINPUT1, EN_SETFOCUS, OnRumblePak)
		COMMAND_HANDLER_EX(IDC_CPADDOWN1,EN_SETFOCUS,OnCPadDown)
		COMMAND_HANDLER_EX(IDC_CPADUP1,EN_SETFOCUS,OnCPadUp)
		COMMAND_HANDLER_EX(IDC_CPADLEFT1,EN_SETFOCUS,OnCPadLeft)
		COMMAND_HANDLER_EX(IDC_CPADRIGHT1,EN_SETFOCUS,OnCPadRight)
		COMMAND_HANDLER_EX(IDC_XINPUT1,EN_SETFOCUS,OnXAxis)
		COMMAND_HANDLER_EX(IDC_RINPUT1,EN_SETFOCUS,OnYAxis)
	END_MSG_MAP()

	CButton plugged_in, use_mousebuttons;
	CComboBox config,paktype,controller;
	CEdit mouse_scalefactors, analogdeadzone,analogpeak;
	CEdit dpadup,dpaddown,dpadleft,dpadright;
	CEdit start, a,b,z,l,r,mempaktoggle,rumblepaktoggle;
	CEdit xaxis,yaxis,cpadup,cpaddown,cpadleft,cpadright;

	m64p_handle l_ConfigInput1;

	LRESULT OnInitDialogView(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		plugged_in = GetDlgItem(IDC_SDLINPUTPLUGGED1);
		use_mousebuttons = GetDlgItem(IDC_MOUSEINPUT1);
		config = GetDlgItem(IDC_SDLINPUTCONFIG1);
		paktype = GetDlgItem(IDC_INPUTPAKTYPE1);
		controller = GetDlgItem(IDC_SDLINPUTCONTROLLER1);
		mouse_scalefactors = GetDlgItem(IDC_INPUTMOUSESCALE1);
		analogdeadzone = GetDlgItem(IDC_INPUTANADEADZONE1);
		analogpeak = GetDlgItem(IDC_ANAPEAK1);
		dpadup =GetDlgItem(IDC_DPADUP1);
		dpaddown=GetDlgItem(IDC_DPADDOWN1);
		dpadleft=GetDlgItem(IDC_DPADLEFT1);
		dpadright=GetDlgItem(IDC_DPADRIGHT1);
		start=GetDlgItem(IDC_STARTINPUT1);
		z=GetDlgItem(IDC_ZINPUT1);
		a=GetDlgItem(IDC_AINPUT1);
		b=GetDlgItem(IDC_BINPUT1);
		l=GetDlgItem(IDC_LINPUT1);
		r=GetDlgItem(IDC_RINPUT1);
		mempaktoggle=GetDlgItem(IDC_MEMPAK1);
		rumblepaktoggle=GetDlgItem(IDC_RUMBLEINPUT1);
		xaxis=GetDlgItem(IDC_XINPUT1);
		yaxis=GetDlgItem(IDC_YINPUT1);
		cpadup=GetDlgItem(IDC_CPADUP1);
		cpaddown=GetDlgItem(IDC_CPADDOWN1);
		cpadleft=GetDlgItem(IDC_CPADLEFT1);
		cpadright=GetDlgItem(IDC_CPADRIGHT1);

		load_settings();
		return 0;
	}

	void load_settings()
	{
		wstring str_w;
		const char* str;
		wstring config_mode[3] = {L"Manual",L"Auto with named SDL device", L"Fully automatic"};
		wstring pak_type[3] = {L"None",L"Mem Pak", L"Rumble Pak"};
		for (int i=0;i<3;i++)config.AddString(config_mode[i].c_str());
		for (int i=0;i<3;i++)paktype.AddString(pak_type[i].c_str());
		CMupen64Plus *emulator_instance = CMupen64Plus::GetSingleton();
		m64p_error rval = emulator_instance->ConfigOpenSection("Input-SDL-Control1", &l_ConfigInput1);

		int configmode = emulator_instance->ConfigGetParamInt(l_ConfigInput1,"mode");
		config.SetCurSel(configmode);
		int pluggedin = emulator_instance->ConfigGetParamBool(l_ConfigInput1,"plugged");
		plugged_in.SetCheck(pluggedin);

		int pak = emulator_instance->ConfigGetParamInt(l_ConfigInput1,"plugin");
		switch (pak)
		{
		case 1:
			paktype.SetCurSel(0);
			break;
		case 2:
			paktype.SetCurSel(1);
			break;
		case 5:
			paktype.SetCurSel(2);
			break;
		}
	
		int error = SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
		SDL_GetError();
		const char* controller_str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"name");
		wstring controller_pick = utf16_from_utf8(controller_str);
		for (int i=0; i<SDL_NumJoysticks(); i++) 
		{
			wstring str = utf16_from_utf8( SDL_JoystickNameForIndex(i));
			controller.AddString(str.c_str());
		}
		controller.AddString(L"Keyboard");
		int sel =controller.FindString(0,controller_pick.c_str());
		controller.SetCurSel(sel);
		SDL_Quit();

		int usemouse = emulator_instance->ConfigGetParamInt(l_ConfigInput1,"mouse");
		use_mousebuttons.SetCheck(usemouse);

		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"MouseSensitivity");
		str_w = utf16_from_utf8(str);
		mouse_scalefactors.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"AnalogDeadzone");
		str_w = utf16_from_utf8(str);
		analogdeadzone.SetWindowTextW(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"AnalogPeak");
		str_w = utf16_from_utf8(str);
		analogpeak.SetWindowTextW(str_w.c_str());

		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad R");
		str_w = utf16_from_utf8(str);
		dpadright.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad L");
		str_w = utf16_from_utf8(str);
		dpadleft.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad D");
		str_w = utf16_from_utf8(str);
		dpaddown.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad U");
		str_w = utf16_from_utf8(str);
		dpadup.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Start");
		str_w = utf16_from_utf8(str);
		start.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Z Trig");
		str_w = utf16_from_utf8(str);
		z.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"B Button");
		str_w = utf16_from_utf8(str);
		b.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"A Button");
		str_w = utf16_from_utf8(str);
		a.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button R");
		str_w = utf16_from_utf8(str);
		cpadright.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button L");
		str_w = utf16_from_utf8(str);
		cpadleft.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button D");
		str_w = utf16_from_utf8(str);
		cpaddown.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button U");
		str_w = utf16_from_utf8(str);
		cpadup.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"R Trig");
		str_w = utf16_from_utf8(str);
		r.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"L Trig");
		str_w = utf16_from_utf8(str);
		l.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Mempak switch");
		str_w = utf16_from_utf8(str);
		mempaktoggle.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Rumblepak switch");
		str_w = utf16_from_utf8(str);
		rumblepaktoggle.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"X Axis");
		str_w = utf16_from_utf8(str);
		xaxis.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Y Axis");
		str_w = utf16_from_utf8(str);
		yaxis.SetWindowText(str_w.c_str());
	}



	void OnDPadDown(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpaddown.SetWindowText(input.str);
	}

	void OnDPadUp(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpadup.SetWindowText(input.str);
	}

	void OnDPadLeft(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpadleft.SetWindowText(input.str);
	}

	void OnDPadRight(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpadright.SetWindowText(input.str);
	}

	void OnStart(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		start.SetWindowText(input.str);
	}

	void OnZ(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		z.SetWindowText(input.str);
	}

	void OnA(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		a.SetWindowText(input.str);
	}

	void OnB(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		b.SetWindowText(input.str);
	}

	void OnL(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		l.SetWindowText(input.str);
	}

	void OnR(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		r.SetWindowText(input.str);
	}

	void OnMempak(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		mempaktoggle.SetWindowText(input.str);
	}

	void OnRumblePak(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		rumblepaktoggle.SetWindowText(input.str);
	}

	void OnCPadDown(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpaddown.SetWindowText(input.str);
	}

	void OnCPadUp(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpadup.SetWindowText(input.str);
	}

	void OnCPadLeft(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpadleft.SetWindowText(input.str);
	}

	void OnCPadRight(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpadright.SetWindowText(input.str);
	}

	void OnXAxis(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		TCHAR str[50]={0};
		TCHAR str2[50]={0};
		TCHAR str3[50]={0};
		MessageBox(L"The following will set the button for Left on the Control Stick");
		input.poll(0);
		lstrcpy(str,input.str);
		MessageBox(L"The following will set the button for Right on the Control Stick");
		input.poll(0);
		lstrcpy(str2,input.str);
		wsprintf(str3,L"key(%s,%s)",str,str2);
		xaxis.SetWindowText(str3);
	}

	void OnYAxis(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		TCHAR str[50]={0};
		TCHAR str2[50]={0};
		TCHAR str3[50]={0};
		MessageBox(L"The following will set the button for Up on the Control Stick");
		input.poll(0);
		lstrcpy(str,input.str);
		MessageBox(L"The following will set the button for Down on the Control Stick");
		input.poll(0);
		lstrcpy(str2,input.str);
		wsprintf(str3,L"key(%s,%s)",str,str2);
	    yaxis.SetWindowText(str3);
	}


	void save_settings()
	{
		CMupen64Plus *emulator_instance = CMupen64Plus::GetSingleton();
		TCHAR strings[64]={0};
		int configmode =  config.GetCurSel();
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"mode",M64TYPE_INT,&configmode);
		int plugged = plugged_in.GetCheck();
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"plugged",M64TYPE_BOOL, &plugged);

		int controller_sel = controller.GetCurSel();
		int paknum= controller_sel;
		controller.GetLBText(controller_sel,strings);
		if (!lstrcmp(strings,L"Keyboard")) paknum = -1;
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"device",M64TYPE_INT,&paknum);
		string string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "name",M64TYPE_STRING, string_utf8.c_str());

		int paktypeser = paktype.GetCurSel();
		int pak = 0;
		if(paktypeser == 0)pak =1;
		if(paktypeser == 1)pak =2;
		if(paktypeser == 2)pak =5;
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"plugin",M64TYPE_INT,&pak);

		int mouse = use_mousebuttons.GetCheck();
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"mouse",M64TYPE_BOOL, &mouse);

		mouse_scalefactors.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "MouseSensitivity",M64TYPE_STRING, string_utf8.c_str());
		analogdeadzone.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "AnalogDeadzone",M64TYPE_STRING, string_utf8.c_str());
		analogpeak.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "AnalogPeak",M64TYPE_STRING, string_utf8.c_str());

		dpadright.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad R",M64TYPE_STRING, string_utf8.c_str());
		dpadleft.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad L",M64TYPE_STRING, string_utf8.c_str());
		dpadup.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad U",M64TYPE_STRING, string_utf8.c_str());
		dpaddown.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad D",M64TYPE_STRING, string_utf8.c_str());
		start.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Start",M64TYPE_STRING, string_utf8.c_str());
		z.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Z Trig",M64TYPE_STRING, string_utf8.c_str());
		b.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "B Button",M64TYPE_STRING, string_utf8.c_str());
		a.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "A Button",M64TYPE_STRING, string_utf8.c_str());
		cpadright.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button R",M64TYPE_STRING, string_utf8.c_str());
		cpadleft.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button L",M64TYPE_STRING, string_utf8.c_str());
		cpadup.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button U",M64TYPE_STRING, string_utf8.c_str());
		cpaddown.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button D",M64TYPE_STRING, string_utf8.c_str());
		r.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "R Trig",M64TYPE_STRING, string_utf8.c_str());
		l.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "L Trig",M64TYPE_STRING, string_utf8.c_str());
		mempaktoggle.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Mempak switch",M64TYPE_STRING, string_utf8.c_str());
		rumblepaktoggle.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Rumblepak switch",M64TYPE_STRING, string_utf8.c_str());
		xaxis.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "X Axis",M64TYPE_STRING, string_utf8.c_str());
		yaxis.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Y Axis",M64TYPE_STRING, string_utf8.c_str());
		m64p_error retval =emulator_instance->ConfigSaveFile();
	}

	
};

class CPluginInputSDL2 : public CDialogImpl<CPluginInputSDL2>
{
public:
	enum { IDD = IDD_SDLINPUT2};
	BEGIN_MSG_MAP(CPluginInputSDL2)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialogView)
		COMMAND_HANDLER_EX(IDC_DPADDOWN2, EN_SETFOCUS, OnDPadDown)
		COMMAND_HANDLER_EX(IDC_DPADUP2, EN_SETFOCUS, OnDPadUp)
		COMMAND_HANDLER_EX(IDC_DPADLEFT2, EN_SETFOCUS, OnDPadLeft)
		COMMAND_HANDLER_EX(IDC_DPADRIGHT2, EN_SETFOCUS, OnDPadRight)
		COMMAND_HANDLER_EX(IDC_STARTINPUT2, EN_SETFOCUS, OnStart)
		COMMAND_HANDLER_EX(IDC_ZINPUT2, EN_SETFOCUS, OnZ)
		COMMAND_HANDLER_EX(IDC_AINPUT2, EN_SETFOCUS, OnA)
		COMMAND_HANDLER_EX(IDC_BINPUT2, EN_SETFOCUS, OnB)
		COMMAND_HANDLER_EX(IDC_LINPUT2, EN_SETFOCUS, OnL)
		COMMAND_HANDLER_EX(IDC_RINPUT2, EN_SETFOCUS, OnR)
		COMMAND_HANDLER_EX(IDC_MEMPAK2, EN_SETFOCUS, OnMempak)
		COMMAND_HANDLER_EX(IDC_RUMBLEINPUT2, EN_SETFOCUS, OnRumblePak)
		COMMAND_HANDLER_EX(IDC_CPADDOWN2,EN_SETFOCUS,OnCPadDown)
		COMMAND_HANDLER_EX(IDC_CPADUP2,EN_SETFOCUS,OnCPadUp)
		COMMAND_HANDLER_EX(IDC_CPADLEFT2,EN_SETFOCUS,OnCPadLeft)
		COMMAND_HANDLER_EX(IDC_CPADRIGHT2,EN_SETFOCUS,OnCPadRight)
		COMMAND_HANDLER_EX(IDC_XINPUT2,EN_SETFOCUS,OnXAxis)
		COMMAND_HANDLER_EX(IDC_RINPUT2,EN_SETFOCUS,OnYAxis)
	END_MSG_MAP()

	CButton plugged_in, use_mousebuttons;
	CComboBox config,paktype,controller;
	CEdit mouse_scalefactors, analogdeadzone,analogpeak;
	CEdit dpadup,dpaddown,dpadleft,dpadright;
	CEdit start, a,b,z,l,r,mempaktoggle,rumblepaktoggle;
	CEdit xaxis,yaxis,cpadup,cpaddown,cpadleft,cpadright;

	m64p_handle l_ConfigInput1;


	LRESULT OnInitDialogView(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		plugged_in = GetDlgItem(IDC_SDLINPUTPLUGGED2);
		use_mousebuttons = GetDlgItem(IDC_MOUSEINPUT2);
		config = GetDlgItem(IDC_SDLINPUTCONFIG2);
		paktype = GetDlgItem(IDC_INPUTPAKTYPE2);
		controller = GetDlgItem(IDC_SDLINPUTCONTROLLER2);
		mouse_scalefactors = GetDlgItem(IDC_INPUTMOUSESCALE2);
		analogdeadzone = GetDlgItem(IDC_INPUTANADEADZONE2);
		analogpeak = GetDlgItem(IDC_ANAPEAK2);
		dpadup =GetDlgItem(IDC_DPADUP2);
		dpaddown=GetDlgItem(IDC_DPADDOWN2);
		dpadleft=GetDlgItem(IDC_DPADLEFT2);
		dpadright=GetDlgItem(IDC_DPADRIGHT2);
		start=GetDlgItem(IDC_STARTINPUT2);
		z=GetDlgItem(IDC_ZINPUT2);
		a=GetDlgItem(IDC_AINPUT2);
		b=GetDlgItem(IDC_BINPUT2);
		l=GetDlgItem(IDC_LINPUT2);
		r=GetDlgItem(IDC_RINPUT2);
		mempaktoggle=GetDlgItem(IDC_MEMPAK2);
		rumblepaktoggle=GetDlgItem(IDC_RUMBLEINPUT2);
		xaxis=GetDlgItem(IDC_XINPUT2);
		yaxis=GetDlgItem(IDC_YINPUT2);
		cpadup=GetDlgItem(IDC_CPADUP2);
		cpaddown=GetDlgItem(IDC_CPADDOWN2);
		cpadleft=GetDlgItem(IDC_CPADLEFT2);
		cpadright=GetDlgItem(IDC_CPADRIGHT2);
		load_settings();
		return 0;
	}

	void load_settings()
	{
		wstring str_w;
		const char* str;
		wstring config_mode[3] = {L"Manual",L"Auto with named SDL device", L"Fully automatic"};
		wstring pak_type[3] = {L"None",L"Mem Pak", L"Rumble Pak"};
		for (int i=0;i<3;i++)config.AddString(config_mode[i].c_str());
		for (int i=0;i<3;i++)paktype.AddString(pak_type[i].c_str());
		CMupen64Plus *emulator_instance = CMupen64Plus::GetSingleton();
		m64p_error rval = emulator_instance->ConfigOpenSection("Input-SDL-Control2", &l_ConfigInput1);

		int configmode = emulator_instance->ConfigGetParamInt(l_ConfigInput1,"mode");
		config.SetCurSel(configmode);
		int pluggedin = emulator_instance->ConfigGetParamBool(l_ConfigInput1,"plugged");
		plugged_in.SetCheck(pluggedin);

		int pak = emulator_instance->ConfigGetParamInt(l_ConfigInput1,"plugin");
		switch (pak)
		{
		case 1:
			paktype.SetCurSel(0);
			break;
		case 2:
			paktype.SetCurSel(1);
			break;
		case 5:
			paktype.SetCurSel(2);
			break;
		}

		int error = SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
		SDL_GetError();
		const char* controller_str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"name");
		wstring controller_pick = utf16_from_utf8(controller_str);
		for (int i=0; i<SDL_NumJoysticks(); i++) 
		{
			wstring str = utf16_from_utf8( SDL_JoystickNameForIndex(i));
			controller.AddString(str.c_str());
		}
		controller.AddString(L"Keyboard");
		int sel =controller.FindString(0,controller_pick.c_str());
		controller.SetCurSel(sel);
		SDL_Quit();

		int usemouse = emulator_instance->ConfigGetParamInt(l_ConfigInput1,"mouse");
		use_mousebuttons.SetCheck(usemouse);

		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"MouseSensitivity");
		str_w = utf16_from_utf8(str);
		mouse_scalefactors.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"AnalogDeadzone");
		str_w = utf16_from_utf8(str);
		analogdeadzone.SetWindowTextW(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"AnalogPeak");
		str_w = utf16_from_utf8(str);
		analogpeak.SetWindowTextW(str_w.c_str());

		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad R");
		str_w = utf16_from_utf8(str);
		dpadright.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad L");
		str_w = utf16_from_utf8(str);
		dpadleft.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad D");
		str_w = utf16_from_utf8(str);
		dpaddown.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad U");
		str_w = utf16_from_utf8(str);
		dpadup.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Start");
		str_w = utf16_from_utf8(str);
		start.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Z Trig");
		str_w = utf16_from_utf8(str);
		z.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"B Button");
		str_w = utf16_from_utf8(str);
		b.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"A Button");
		str_w = utf16_from_utf8(str);
		a.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button R");
		str_w = utf16_from_utf8(str);
		cpadright.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button L");
		str_w = utf16_from_utf8(str);
		cpadleft.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button D");
		str_w = utf16_from_utf8(str);
		cpaddown.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button U");
		str_w = utf16_from_utf8(str);
		cpadup.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"R Trig");
		str_w = utf16_from_utf8(str);
		r.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"L Trig");
		str_w = utf16_from_utf8(str);
		l.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Mempak switch");
		str_w = utf16_from_utf8(str);
		mempaktoggle.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Rumblepak switch");
		str_w = utf16_from_utf8(str);
		rumblepaktoggle.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"X Axis");
		str_w = utf16_from_utf8(str);
		xaxis.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Y Axis");
		str_w = utf16_from_utf8(str);
		yaxis.SetWindowText(str_w.c_str());
	}

	void OnDPadDown(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpaddown.SetWindowText(input.str);
	}

	void OnDPadUp(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpadup.SetWindowText(input.str);
	}

	void OnDPadLeft(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpadleft.SetWindowText(input.str);
	}

	void OnDPadRight(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpadright.SetWindowText(input.str);
	}

	void OnStart(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		start.SetWindowText(input.str);
	}

	void OnZ(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		z.SetWindowText(input.str);
	}

	void OnA(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		a.SetWindowText(input.str);
	}

	void OnB(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		b.SetWindowText(input.str);
	}

	void OnL(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		l.SetWindowText(input.str);
	}

	void OnR(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		r.SetWindowText(input.str);
	}

	void OnMempak(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		mempaktoggle.SetWindowText(input.str);
	}

	void OnRumblePak(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		rumblepaktoggle.SetWindowText(input.str);
	}

	void OnCPadDown(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpaddown.SetWindowText(input.str);
	}

	void OnCPadUp(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpadup.SetWindowText(input.str);
	}

	void OnCPadLeft(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpadleft.SetWindowText(input.str);
	}

	void OnCPadRight(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpadright.SetWindowText(input.str);
	}

	void OnXAxis(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		TCHAR str[50]={0};
		TCHAR str2[50]={0};
		TCHAR str3[50]={0};
		MessageBox(L"The following will set the button for Left on the Control Stick");
		input.poll(0);
		lstrcpy(str,input.str);
		MessageBox(L"The following will set the button for Right on the Control Stick");
		input.poll(0);
		lstrcpy(str2,input.str);
		wsprintf(str3,L"key(%s,%s)",str,str2);
		xaxis.SetWindowText(str3);
	}

	void OnYAxis(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		TCHAR str[50]={0};
		TCHAR str2[50]={0};
		TCHAR str3[50]={0};
		MessageBox(L"The following will set the button for Up on the Control Stick");
		input.poll(0);
		lstrcpy(str,input.str);
		MessageBox(L"The following will set the button for Down on the Control Stick");
		input.poll(0);
		lstrcpy(str2,input.str);
		wsprintf(str3,L"key(%s,%s)",str,str2);
		yaxis.SetWindowText(str3);
	}


	void save_settings()
	{
		CMupen64Plus *emulator_instance = CMupen64Plus::GetSingleton();
		TCHAR strings[64]={0};
		int configmode =  config.GetCurSel();
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"mode",M64TYPE_INT,&configmode);
		int plugged = plugged_in.GetCheck();
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"plugged",M64TYPE_BOOL, &plugged);

		int controller_sel = controller.GetCurSel();
		int paknum= controller_sel;
		controller.GetLBText(controller_sel,strings);
		if (!lstrcmp(strings,L"Keyboard")) paknum = -1;
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"device",M64TYPE_INT,&paknum);
		string string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "name",M64TYPE_STRING, string_utf8.c_str());

		int paktypeser = paktype.GetCurSel();
		int pak = 0;
		if(paktypeser == 0)pak =1;
		if(paktypeser == 1)pak =2;
		if(paktypeser == 2)pak =5;
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"plugin",M64TYPE_INT,&pak);

		int mouse = use_mousebuttons.GetCheck();
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"mouse",M64TYPE_BOOL, &mouse);

		mouse_scalefactors.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "MouseSensitivity",M64TYPE_STRING, string_utf8.c_str());
		analogdeadzone.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "AnalogDeadzone",M64TYPE_STRING, string_utf8.c_str());
		analogpeak.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "AnalogPeak",M64TYPE_STRING, string_utf8.c_str());

		dpadright.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad R",M64TYPE_STRING, string_utf8.c_str());
		dpadleft.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad L",M64TYPE_STRING, string_utf8.c_str());
		dpadup.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad U",M64TYPE_STRING, string_utf8.c_str());
		dpaddown.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad D",M64TYPE_STRING, string_utf8.c_str());
		start.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Start",M64TYPE_STRING, string_utf8.c_str());
		z.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Z Trig",M64TYPE_STRING, string_utf8.c_str());
		b.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "B Button",M64TYPE_STRING, string_utf8.c_str());
		a.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "A Button",M64TYPE_STRING, string_utf8.c_str());
		cpadright.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button R",M64TYPE_STRING, string_utf8.c_str());
		cpadleft.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button L",M64TYPE_STRING, string_utf8.c_str());
		cpadup.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button U",M64TYPE_STRING, string_utf8.c_str());
		cpaddown.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button D",M64TYPE_STRING, string_utf8.c_str());
		r.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "R Trig",M64TYPE_STRING, string_utf8.c_str());
		l.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "L Trig",M64TYPE_STRING, string_utf8.c_str());
		mempaktoggle.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Mempak switch",M64TYPE_STRING, string_utf8.c_str());
		rumblepaktoggle.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Rumblepak switch",M64TYPE_STRING, string_utf8.c_str());
		xaxis.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "X Axis",M64TYPE_STRING, string_utf8.c_str());
		yaxis.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Y Axis",M64TYPE_STRING, string_utf8.c_str());
		m64p_error retval =emulator_instance->ConfigSaveFile();
	}

};

class CPluginInputSDL3 : public CDialogImpl<CPluginInputSDL3>
{
public:
	enum { IDD = IDD_SDLINPUT3};
	BEGIN_MSG_MAP(CPluginInputSDL3)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialogView)
		COMMAND_HANDLER_EX(IDC_DPADDOWN3, EN_SETFOCUS, OnDPadDown)
		COMMAND_HANDLER_EX(IDC_DPADUP3, EN_SETFOCUS, OnDPadUp)
		COMMAND_HANDLER_EX(IDC_DPADLEFT3, EN_SETFOCUS, OnDPadLeft)
		COMMAND_HANDLER_EX(IDC_DPADRIGHT3, EN_SETFOCUS, OnDPadRight)
		COMMAND_HANDLER_EX(IDC_STARTINPUT3, EN_SETFOCUS, OnStart)
		COMMAND_HANDLER_EX(IDC_ZINPUT3, EN_SETFOCUS, OnZ)
		COMMAND_HANDLER_EX(IDC_AINPUT3, EN_SETFOCUS, OnA)
		COMMAND_HANDLER_EX(IDC_BINPUT3, EN_SETFOCUS, OnB)
		COMMAND_HANDLER_EX(IDC_LINPUT3, EN_SETFOCUS, OnL)
		COMMAND_HANDLER_EX(IDC_RINPUT3, EN_SETFOCUS, OnR)
		COMMAND_HANDLER_EX(IDC_MEMPAK3, EN_SETFOCUS, OnMempak)
		COMMAND_HANDLER_EX(IDC_RUMBLEINPUT3, EN_SETFOCUS, OnRumblePak)
		COMMAND_HANDLER_EX(IDC_CPADDOWN3,EN_SETFOCUS,OnCPadDown)
		COMMAND_HANDLER_EX(IDC_CPADUP3,EN_SETFOCUS,OnCPadUp)
		COMMAND_HANDLER_EX(IDC_CPADLEFT3,EN_SETFOCUS,OnCPadLeft)
		COMMAND_HANDLER_EX(IDC_CPADRIGHT3,EN_SETFOCUS,OnCPadRight)
		COMMAND_HANDLER_EX(IDC_XINPUT3,EN_SETFOCUS,OnXAxis)
		COMMAND_HANDLER_EX(IDC_RINPUT3,EN_SETFOCUS,OnYAxis)
	END_MSG_MAP()

	CButton plugged_in, use_mousebuttons;
	CComboBox config,paktype,controller;
	CEdit mouse_scalefactors, analogdeadzone,analogpeak;
	CEdit dpadup,dpaddown,dpadleft,dpadright;
	CEdit start, a,b,z,l,r,mempaktoggle,rumblepaktoggle;
	CEdit xaxis,yaxis,cpadup,cpaddown,cpadleft,cpadright;

	m64p_handle l_ConfigInput1;

	LRESULT OnInitDialogView(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		plugged_in = GetDlgItem(IDC_SDLINPUTPLUGGED3);
		use_mousebuttons = GetDlgItem(IDC_MOUSEINPUT3);
		config = GetDlgItem(IDC_SDLINPUTCONFIG3);
		paktype = GetDlgItem(IDC_INPUTPAKTYPE3);
		controller = GetDlgItem(IDC_SDLINPUTCONTROLLER3);
		mouse_scalefactors = GetDlgItem(IDC_INPUTMOUSESCALE3);
		analogdeadzone = GetDlgItem(IDC_INPUTANADEADZONE3);
		analogpeak = GetDlgItem(IDC_ANAPEAK3);
		dpadup =GetDlgItem(IDC_DPADUP3);
		dpaddown=GetDlgItem(IDC_DPADDOWN3);
		dpadleft=GetDlgItem(IDC_DPADLEFT3);
		dpadright=GetDlgItem(IDC_DPADRIGHT3);
		start=GetDlgItem(IDC_STARTINPUT3);
		z=GetDlgItem(IDC_ZINPUT3);
		a=GetDlgItem(IDC_AINPUT3);
		b=GetDlgItem(IDC_BINPUT3);
		l=GetDlgItem(IDC_LINPUT3);
		r=GetDlgItem(IDC_RINPUT3);
		mempaktoggle=GetDlgItem(IDC_MEMPAK3);
		rumblepaktoggle=GetDlgItem(IDC_RUMBLEINPUT3);
		xaxis=GetDlgItem(IDC_XINPUT3);
		yaxis=GetDlgItem(IDC_YINPUT3);
		cpadup=GetDlgItem(IDC_CPADUP3);
		cpaddown=GetDlgItem(IDC_CPADDOWN3);
		cpadleft=GetDlgItem(IDC_CPADLEFT3);
		cpadright=GetDlgItem(IDC_CPADRIGHT3);
		load_settings();
		return 0;
	}

	void load_settings()
	{
		wstring str_w;
		const char* str;
		wstring config_mode[3] = {L"Manual",L"Auto with named SDL device", L"Fully automatic"};
		wstring pak_type[3] = {L"None",L"Mem Pak", L"Rumble Pak"};
		for (int i=0;i<3;i++)config.AddString(config_mode[i].c_str());
		for (int i=0;i<3;i++)paktype.AddString(pak_type[i].c_str());
		CMupen64Plus *emulator_instance = CMupen64Plus::GetSingleton();
		m64p_error rval = emulator_instance->ConfigOpenSection("Input-SDL-Control3", &l_ConfigInput1);

		int configmode = emulator_instance->ConfigGetParamInt(l_ConfigInput1,"mode");
		config.SetCurSel(configmode);
		int pluggedin = emulator_instance->ConfigGetParamBool(l_ConfigInput1,"plugged");
		plugged_in.SetCheck(pluggedin);

		int pak = emulator_instance->ConfigGetParamInt(l_ConfigInput1,"plugin");
		switch (pak)
		{
		case 1:
			paktype.SetCurSel(0);
			break;
		case 2:
			paktype.SetCurSel(1);
			break;
		case 5:
			paktype.SetCurSel(2);
			break;
		}

		int error = SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
		SDL_GetError();
		const char* controller_str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"name");
		wstring controller_pick = utf16_from_utf8(controller_str);
		for (int i=0; i<SDL_NumJoysticks(); i++) 
		{
			wstring str = utf16_from_utf8( SDL_JoystickNameForIndex(i));
			controller.AddString(str.c_str());
		}
		controller.AddString(L"Keyboard");
		int sel =controller.FindString(0,controller_pick.c_str());
		controller.SetCurSel(sel);
		SDL_Quit();

		int usemouse = emulator_instance->ConfigGetParamInt(l_ConfigInput1,"mouse");
		use_mousebuttons.SetCheck(usemouse);

		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"MouseSensitivity");
		str_w = utf16_from_utf8(str);
		mouse_scalefactors.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"AnalogDeadzone");
		str_w = utf16_from_utf8(str);
		analogdeadzone.SetWindowTextW(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"AnalogPeak");
		str_w = utf16_from_utf8(str);
		analogpeak.SetWindowTextW(str_w.c_str());

		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad R");
		str_w = utf16_from_utf8(str);
		dpadright.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad L");
		str_w = utf16_from_utf8(str);
		dpadleft.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad D");
		str_w = utf16_from_utf8(str);
		dpaddown.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad U");
		str_w = utf16_from_utf8(str);
		dpadup.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Start");
		str_w = utf16_from_utf8(str);
		start.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Z Trig");
		str_w = utf16_from_utf8(str);
		z.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"B Button");
		str_w = utf16_from_utf8(str);
		b.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"A Button");
		str_w = utf16_from_utf8(str);
		a.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button R");
		str_w = utf16_from_utf8(str);
		cpadright.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button L");
		str_w = utf16_from_utf8(str);
		cpadleft.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button D");
		str_w = utf16_from_utf8(str);
		cpaddown.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button U");
		str_w = utf16_from_utf8(str);
		cpadup.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"R Trig");
		str_w = utf16_from_utf8(str);
		r.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"L Trig");
		str_w = utf16_from_utf8(str);
		l.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Mempak switch");
		str_w = utf16_from_utf8(str);
		mempaktoggle.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Rumblepak switch");
		str_w = utf16_from_utf8(str);
		rumblepaktoggle.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"X Axis");
		str_w = utf16_from_utf8(str);
		xaxis.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Y Axis");
		str_w = utf16_from_utf8(str);
		yaxis.SetWindowText(str_w.c_str());
	}

	void OnDPadDown(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpaddown.SetWindowText(input.str);
	}

	void OnDPadUp(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpadup.SetWindowText(input.str);
	}

	void OnDPadLeft(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpadleft.SetWindowText(input.str);
	}

	void OnDPadRight(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpadright.SetWindowText(input.str);
	}

	void OnStart(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		start.SetWindowText(input.str);
	}

	void OnZ(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		z.SetWindowText(input.str);
	}

	void OnA(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		a.SetWindowText(input.str);
	}

	void OnB(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		b.SetWindowText(input.str);
	}

	void OnL(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		l.SetWindowText(input.str);
	}

	void OnR(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		r.SetWindowText(input.str);
	}

	void OnMempak(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		mempaktoggle.SetWindowText(input.str);
	}

	void OnRumblePak(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		rumblepaktoggle.SetWindowText(input.str);
	}

	void OnCPadDown(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpaddown.SetWindowText(input.str);
	}

	void OnCPadUp(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpadup.SetWindowText(input.str);
	}

	void OnCPadLeft(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpadleft.SetWindowText(input.str);
	}

	void OnCPadRight(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpadright.SetWindowText(input.str);
	}

	void OnXAxis(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		TCHAR str[50]={0};
		TCHAR str2[50]={0};
		TCHAR str3[50]={0};
		MessageBox(L"The following will set the button for Left on the Control Stick");
		input.poll(0);
		lstrcpy(str,input.str);
		MessageBox(L"The following will set the button for Right on the Control Stick");
		input.poll(0);
		lstrcpy(str2,input.str);
		wsprintf(str3,L"key(%s,%s)",str,str2);
		xaxis.SetWindowText(str3);
	}

	void OnYAxis(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		TCHAR str[50]={0};
		TCHAR str2[50]={0};
		TCHAR str3[50]={0};
		MessageBox(L"The following will set the button for Up on the Control Stick");
		input.poll(0);
		lstrcpy(str,input.str);
		MessageBox(L"The following will set the button for Down on the Control Stick");
		input.poll(0);
		lstrcpy(str2,input.str);
		wsprintf(str3,L"key(%s,%s)",str,str2);
		yaxis.SetWindowText(str3);
	}


	void save_settings()
	{
		CMupen64Plus *emulator_instance = CMupen64Plus::GetSingleton();
		TCHAR strings[64]={0};
		int configmode =  config.GetCurSel();
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"mode",M64TYPE_INT,&configmode);
		int plugged = plugged_in.GetCheck();
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"plugged",M64TYPE_BOOL, &plugged);

		int controller_sel = controller.GetCurSel();
		int paknum= controller_sel;
		controller.GetLBText(controller_sel,strings);
		if (!lstrcmp(strings,L"Keyboard")) paknum = -1;
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"device",M64TYPE_INT,&paknum);
		string string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "name",M64TYPE_STRING, string_utf8.c_str());

		int paktypeser = paktype.GetCurSel();
		int pak = 0;
		if(paktypeser == 0)pak =1;
		if(paktypeser == 1)pak =2;
		if(paktypeser == 2)pak =5;
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"plugin",M64TYPE_INT,&pak);

		int mouse = use_mousebuttons.GetCheck();
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"mouse",M64TYPE_BOOL, &mouse);

		mouse_scalefactors.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "MouseSensitivity",M64TYPE_STRING, string_utf8.c_str());
		analogdeadzone.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "AnalogDeadzone",M64TYPE_STRING, string_utf8.c_str());
		analogpeak.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "AnalogPeak",M64TYPE_STRING, string_utf8.c_str());

		dpadright.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad R",M64TYPE_STRING, string_utf8.c_str());
		dpadleft.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad L",M64TYPE_STRING, string_utf8.c_str());
		dpadup.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad U",M64TYPE_STRING, string_utf8.c_str());
		dpaddown.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad D",M64TYPE_STRING, string_utf8.c_str());
		start.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Start",M64TYPE_STRING, string_utf8.c_str());
		z.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Z Trig",M64TYPE_STRING, string_utf8.c_str());
		b.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "B Button",M64TYPE_STRING, string_utf8.c_str());
		a.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "A Button",M64TYPE_STRING, string_utf8.c_str());
		cpadright.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button R",M64TYPE_STRING, string_utf8.c_str());
		cpadleft.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button L",M64TYPE_STRING, string_utf8.c_str());
		cpadup.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button U",M64TYPE_STRING, string_utf8.c_str());
		cpaddown.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button D",M64TYPE_STRING, string_utf8.c_str());
		r.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "R Trig",M64TYPE_STRING, string_utf8.c_str());
		l.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "L Trig",M64TYPE_STRING, string_utf8.c_str());
		mempaktoggle.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Mempak switch",M64TYPE_STRING, string_utf8.c_str());
		rumblepaktoggle.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Rumblepak switch",M64TYPE_STRING, string_utf8.c_str());
		xaxis.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "X Axis",M64TYPE_STRING, string_utf8.c_str());
		yaxis.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Y Axis",M64TYPE_STRING, string_utf8.c_str());
		m64p_error retval =emulator_instance->ConfigSaveFile();
	}

};

class CPluginInputSDL4 : public CDialogImpl<CPluginInputSDL4>
{
public:
	enum { IDD = IDD_SDLINPUT4};
	BEGIN_MSG_MAP(CPluginInputSDL4)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialogView)
		COMMAND_HANDLER_EX(IDC_DPADDOWN4, EN_SETFOCUS, OnDPadDown)
		COMMAND_HANDLER_EX(IDC_DPADUP4, EN_SETFOCUS, OnDPadUp)
		COMMAND_HANDLER_EX(IDC_DPADLEFT4, EN_SETFOCUS, OnDPadLeft)
		COMMAND_HANDLER_EX(IDC_DPADRIGHT4, EN_SETFOCUS, OnDPadRight)
		COMMAND_HANDLER_EX(IDC_STARTINPUT4, EN_SETFOCUS, OnStart)
		COMMAND_HANDLER_EX(IDC_ZINPUT4, EN_SETFOCUS, OnZ)
		COMMAND_HANDLER_EX(IDC_AINPUT4, EN_SETFOCUS, OnA)
		COMMAND_HANDLER_EX(IDC_BINPUT4, EN_SETFOCUS, OnB)
		COMMAND_HANDLER_EX(IDC_LINPUT4, EN_SETFOCUS, OnL)
		COMMAND_HANDLER_EX(IDC_RINPUT4, EN_SETFOCUS, OnR)
		COMMAND_HANDLER_EX(IDC_MEMPAK4, EN_SETFOCUS, OnMempak)
		COMMAND_HANDLER_EX(IDC_RUMBLEINPUT4, EN_SETFOCUS, OnRumblePak)
		COMMAND_HANDLER_EX(IDC_CPADDOWN4,EN_SETFOCUS,OnCPadDown)
		COMMAND_HANDLER_EX(IDC_CPADUP4,EN_SETFOCUS,OnCPadUp)
		COMMAND_HANDLER_EX(IDC_CPADLEFT4,EN_SETFOCUS,OnCPadLeft)
		COMMAND_HANDLER_EX(IDC_CPADRIGHT4,EN_SETFOCUS,OnCPadRight)
		COMMAND_HANDLER_EX(IDC_XINPUT4,EN_SETFOCUS,OnXAxis)
		COMMAND_HANDLER_EX(IDC_RINPUT4,EN_SETFOCUS,OnYAxis)
	END_MSG_MAP()

	CButton plugged_in, use_mousebuttons;
	CComboBox config,paktype,controller;
	CEdit mouse_scalefactors, analogdeadzone,analogpeak;
	CEdit dpadup,dpaddown,dpadleft,dpadright;
	CEdit start, a,b,z,l,r,mempaktoggle,rumblepaktoggle;
	CEdit xaxis,yaxis,cpadup,cpaddown,cpadleft,cpadright;

	m64p_handle l_ConfigInput1;

	LRESULT OnInitDialogView(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		plugged_in = GetDlgItem(IDC_SDLINPUTPLUGGED4);
		use_mousebuttons = GetDlgItem(IDC_MOUSEINPUT4);
		config = GetDlgItem(IDC_SDLINPUTCONFIG4);
		paktype = GetDlgItem(IDC_INPUTPAKTYPE4);
		controller = GetDlgItem(IDC_SDLINPUTCONTROLLER4);
		mouse_scalefactors = GetDlgItem(IDC_INPUTMOUSESCALE4);
		analogdeadzone = GetDlgItem(IDC_INPUTANADEADZONE4);
		analogpeak = GetDlgItem(IDC_ANAPEAK4);
		dpadup =GetDlgItem(IDC_DPADUP4);
		dpaddown=GetDlgItem(IDC_DPADDOWN4);
		dpadleft=GetDlgItem(IDC_DPADLEFT4);
		dpadright=GetDlgItem(IDC_DPADRIGHT4);
		start=GetDlgItem(IDC_STARTINPUT4);
		z=GetDlgItem(IDC_ZINPUT4);
		a=GetDlgItem(IDC_AINPUT4);
		b=GetDlgItem(IDC_BINPUT4);
		l=GetDlgItem(IDC_LINPUT4);
		r=GetDlgItem(IDC_RINPUT4);
		mempaktoggle=GetDlgItem(IDC_MEMPAK4);
		rumblepaktoggle=GetDlgItem(IDC_RUMBLEINPUT4);
		xaxis=GetDlgItem(IDC_XINPUT4);
		yaxis=GetDlgItem(IDC_YINPUT4);
		cpadup=GetDlgItem(IDC_CPADUP4);
		cpaddown=GetDlgItem(IDC_CPADDOWN4);
		cpadleft=GetDlgItem(IDC_CPADLEFT4);
		cpadright=GetDlgItem(IDC_CPADRIGHT4);
		load_settings();
		return 0;
	}

	void load_settings()
	{
		wstring str_w;
		const char* str;
		wstring config_mode[3] = {L"Manual",L"Auto with named SDL device", L"Fully automatic"};
		wstring pak_type[3] = {L"None",L"Mem Pak", L"Rumble Pak"};
		for (int i=0;i<3;i++)config.AddString(config_mode[i].c_str());
		for (int i=0;i<3;i++)paktype.AddString(pak_type[i].c_str());
		CMupen64Plus *emulator_instance = CMupen64Plus::GetSingleton();
		m64p_error rval = emulator_instance->ConfigOpenSection("Input-SDL-Control4", &l_ConfigInput1);

		int configmode = emulator_instance->ConfigGetParamInt(l_ConfigInput1,"mode");
		config.SetCurSel(configmode);
		int pluggedin = emulator_instance->ConfigGetParamBool(l_ConfigInput1,"plugged");
		plugged_in.SetCheck(pluggedin);

		int pak = emulator_instance->ConfigGetParamInt(l_ConfigInput1,"plugin");
		switch (pak)
		{
		case 1:
			paktype.SetCurSel(0);
			break;
		case 2:
			paktype.SetCurSel(1);
			break;
		case 5:
			paktype.SetCurSel(2);
			break;
		}

		int error = SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
		SDL_GetError();
		const char* controller_str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"name");
		wstring controller_pick = utf16_from_utf8(controller_str);
		for (int i=0; i<SDL_NumJoysticks(); i++) 
		{
			wstring str = utf16_from_utf8( SDL_JoystickNameForIndex(i));
			controller.AddString(str.c_str());
		}
		controller.AddString(L"Keyboard");
		int sel =controller.FindString(0,controller_pick.c_str());
		controller.SetCurSel(sel);
		SDL_Quit();

		int usemouse = emulator_instance->ConfigGetParamInt(l_ConfigInput1,"mouse");
		use_mousebuttons.SetCheck(usemouse);

		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"MouseSensitivity");
		str_w = utf16_from_utf8(str);
		mouse_scalefactors.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"AnalogDeadzone");
		str_w = utf16_from_utf8(str);
		analogdeadzone.SetWindowTextW(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"AnalogPeak");
		str_w = utf16_from_utf8(str);
		analogpeak.SetWindowTextW(str_w.c_str());

		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad R");
		str_w = utf16_from_utf8(str);
		dpadright.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad L");
		str_w = utf16_from_utf8(str);
		dpadleft.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad D");
		str_w = utf16_from_utf8(str);
		dpaddown.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"DPad U");
		str_w = utf16_from_utf8(str);
		dpadup.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Start");
		str_w = utf16_from_utf8(str);
		start.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Z Trig");
		str_w = utf16_from_utf8(str);
		z.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"B Button");
		str_w = utf16_from_utf8(str);
		b.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"A Button");
		str_w = utf16_from_utf8(str);
		a.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button R");
		str_w = utf16_from_utf8(str);
		cpadright.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button L");
		str_w = utf16_from_utf8(str);
		cpadleft.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button D");
		str_w = utf16_from_utf8(str);
		cpaddown.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"C Button U");
		str_w = utf16_from_utf8(str);
		cpadup.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"R Trig");
		str_w = utf16_from_utf8(str);
		r.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"L Trig");
		str_w = utf16_from_utf8(str);
		l.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Mempak switch");
		str_w = utf16_from_utf8(str);
		mempaktoggle.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Rumblepak switch");
		str_w = utf16_from_utf8(str);
		rumblepaktoggle.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"X Axis");
		str_w = utf16_from_utf8(str);
		xaxis.SetWindowText(str_w.c_str());
		str = emulator_instance->ConfigGetParamString(l_ConfigInput1,"Y Axis");
		str_w = utf16_from_utf8(str);
		yaxis.SetWindowText(str_w.c_str());
	}

	void OnDPadDown(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpaddown.SetWindowText(input.str);
	}

	void OnDPadUp(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpadup.SetWindowText(input.str);
	}

	void OnDPadLeft(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpadleft.SetWindowText(input.str);
	}

	void OnDPadRight(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		dpadright.SetWindowText(input.str);
	}

	void OnStart(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		start.SetWindowText(input.str);
	}

	void OnZ(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		z.SetWindowText(input.str);
	}

	void OnA(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		a.SetWindowText(input.str);
	}

	void OnB(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		b.SetWindowText(input.str);
	}

	void OnL(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		l.SetWindowText(input.str);
	}

	void OnR(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		r.SetWindowText(input.str);
	}

	void OnMempak(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		mempaktoggle.SetWindowText(input.str);
	}

	void OnRumblePak(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		rumblepaktoggle.SetWindowText(input.str);
	}

	void OnCPadDown(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpaddown.SetWindowText(input.str);
	}

	void OnCPadUp(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpadup.SetWindowText(input.str);
	}

	void OnCPadLeft(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpadleft.SetWindowText(input.str);
	}

	void OnCPadRight(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		input.poll();
		cpadright.SetWindowText(input.str);
	}

	void OnXAxis(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		TCHAR str[50]={0};
		TCHAR str2[50]={0};
		TCHAR str3[50]={0};
		MessageBox(L"The following will set the button for Left on the Control Stick");
		input.poll(0);
		lstrcpy(str,input.str);
		MessageBox(L"The following will set the button for Right on the Control Stick");
		input.poll(0);
		lstrcpy(str2,input.str);
		wsprintf(str3,L"key(%s,%s)",str,str2);
		xaxis.SetWindowText(str3);
	}

	void OnYAxis(UINT, int, CWindow w) {
		SetFocus();
		CPluginInputKeys input;
		TCHAR str[50]={0};
		TCHAR str2[50]={0};
		TCHAR str3[50]={0};
		MessageBox(L"The following will set the button for Up on the Control Stick");
		input.poll(0);
		lstrcpy(str,input.str);
		MessageBox(L"The following will set the button for Down on the Control Stick");
		input.poll(0);
		lstrcpy(str2,input.str);
		wsprintf(str3,L"key(%s,%s)",str,str2);
		yaxis.SetWindowText(str3);
	}


	void save_settings()
	{
		CMupen64Plus *emulator_instance = CMupen64Plus::GetSingleton();
		TCHAR strings[64]={0};
		int configmode =  config.GetCurSel();
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"mode",M64TYPE_INT,&configmode);
		int plugged = plugged_in.GetCheck();
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"plugged",M64TYPE_BOOL, &plugged);

		int controller_sel = controller.GetCurSel();
		int paknum= controller_sel;
		controller.GetLBText(controller_sel,strings);
		if (!lstrcmp(strings,L"Keyboard")) paknum = -1;
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"device",M64TYPE_INT,&paknum);
		string string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "name",M64TYPE_STRING, string_utf8.c_str());

		int paktypeser = paktype.GetCurSel();
		int pak = 0;
		if(paktypeser == 0)pak =1;
		if(paktypeser == 1)pak =2;
		if(paktypeser == 2)pak =5;
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"plugin",M64TYPE_INT,&pak);

		int mouse = use_mousebuttons.GetCheck();
		emulator_instance->ConfigSetParameter(l_ConfigInput1,"mouse",M64TYPE_BOOL, &mouse);

		mouse_scalefactors.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "MouseSensitivity",M64TYPE_STRING, string_utf8.c_str());
		analogdeadzone.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "AnalogDeadzone",M64TYPE_STRING, string_utf8.c_str());
		analogpeak.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "AnalogPeak",M64TYPE_STRING, string_utf8.c_str());

		dpadright.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad R",M64TYPE_STRING, string_utf8.c_str());
		dpadleft.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad L",M64TYPE_STRING, string_utf8.c_str());
		dpadup.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad U",M64TYPE_STRING, string_utf8.c_str());
		dpaddown.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "DPad D",M64TYPE_STRING, string_utf8.c_str());
		start.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Start",M64TYPE_STRING, string_utf8.c_str());
		z.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Z Trig",M64TYPE_STRING, string_utf8.c_str());
		b.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "B Button",M64TYPE_STRING, string_utf8.c_str());
		a.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "A Button",M64TYPE_STRING, string_utf8.c_str());
		cpadright.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button R",M64TYPE_STRING, string_utf8.c_str());
		cpadleft.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button L",M64TYPE_STRING, string_utf8.c_str());
		cpadup.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button U",M64TYPE_STRING, string_utf8.c_str());
		cpaddown.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "C Button D",M64TYPE_STRING, string_utf8.c_str());
		r.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "R Trig",M64TYPE_STRING, string_utf8.c_str());
		l.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "L Trig",M64TYPE_STRING, string_utf8.c_str());
		mempaktoggle.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Mempak switch",M64TYPE_STRING, string_utf8.c_str());
		rumblepaktoggle.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Rumblepak switch",M64TYPE_STRING, string_utf8.c_str());
		xaxis.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "X Axis",M64TYPE_STRING, string_utf8.c_str());
		yaxis.GetWindowText(strings,64);
		string_utf8 = utf8_from_utf16(strings);
		emulator_instance->ConfigSetParameter(l_ConfigInput1, "Y Axis",M64TYPE_STRING, string_utf8.c_str());
		m64p_error retval =emulator_instance->ConfigSaveFile();
	}
};

class CPluginInputSDL : public CDialogImpl<CPluginInputSDL>
{
public:
	CDialogTabCtrl m_ctrlTab;
	CPluginInputSDL1 view1;
	CPluginInputSDL2 view2;
	CPluginInputSDL3 view3;
	CPluginInputSDL4 view4;

	enum { IDD = IDD_INPUTPLUGIN};

	BEGIN_MSG_MAP( CPluginInputSDL)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL,OnButtonCancel)
		COMMAND_ID_HANDLER(IDOK,OnButtonOK)
		REFLECT_NOTIFICATIONS();
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();
		m_ctrlTab.SubclassWindow(GetDlgItem(IDC_INPUTTAB));
		view1.Create(m_hWnd);
		view2.Create(m_hWnd);
		view3.Create(m_hWnd);
		view4.Create(m_hWnd);
		TCITEM tci = { 0 };
		tci.mask = TCIF_TEXT;
		tci.pszText = _T("Controller 1");
		m_ctrlTab.InsertItem(0, &tci, view1);
		tci.pszText = _T("Controller 2");
		m_ctrlTab.InsertItem(1, &tci,view2);
		tci.pszText = _T("Controller 3");
		m_ctrlTab.InsertItem(2, &tci,view3);
		tci.pszText = _T("Controller 4");
		m_ctrlTab.InsertItem(3, &tci,view4);
		m_ctrlTab.SetCurSel(0);
		return TRUE;
	}

	LRESULT OnButtonCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnButtonOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		view1.save_settings();
		view2.save_settings();
		view3.save_settings();
		view4.save_settings();
		EndDialog(wID);
		return 0;
	}


};

#define numWindowedModes 9
struct
{
	WORD width, height;
	TCHAR *description;
} windowedModes[9] = {
	{ 640, 480, L"640 x 480" },
	{ 800, 600, L"800 x 600" },
	{ 960, 720, L"960 x 720" },
	{ 1024, 768, L"1024 x 768" },
	{ 1152, 864, L"1152 x 864" },
	{ 1280, 960, L"1280 x 960" },
	{ 1280, 1024, L"1280 x 1024" },
	{ 1440, 1080, L"1440 x 1080" },
	{ 1600, 1200, L"1600 x 1200" }
};

class CPluginVideoGeneral : public CDialogImpl<CPluginVideoGeneral>
{
public:
	enum { IDD = IDD_VIDEOSETTINGS};
	BEGIN_MSG_MAP(CPluginVideoGeneral)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   END_MSG_MAP()

   CButton fullscreen;
   CComboBox reslist;
   m64p_handle l_ConfigVideoGeneral;

   LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
	   TCHAR strings[64]={0};
	   fullscreen = GetDlgItem(IDC_VIDFULLSCREEN);
	   reslist = GetDlgItem(IDC_VIDRES);

	   for (int i=0; i<numWindowedModes; i++) 
	   {
		   reslist.AddString(windowedModes[i].description);
	   }
	   reslist.SetCurSel(0);

	   CMupen64Plus *emulator_instance = CMupen64Plus::GetSingleton();
	   m64p_error rval = emulator_instance->ConfigOpenSection("Video-General", &l_ConfigVideoGeneral);

	   int fs = emulator_instance->ConfigGetParamBool(l_ConfigVideoGeneral,"Fullscreen");
	   fullscreen.SetCheck(fs);
	   int x_res = emulator_instance->ConfigGetParamInt(l_ConfigVideoGeneral,"ScreenWidth");

		for (int j=0;j<numWindowedModes;j++)
		{
			if (x_res == windowedModes[j].width)
			{
				reslist.SetCurSel(j);
				break;
			}
		}
		

	   return 0;
   }

   void save_settings()
   {
	    TCHAR strings[64]={0};
	   	CMupen64Plus *emulator_instance = CMupen64Plus::GetSingleton();
		int fs = fullscreen.GetCheck();
		emulator_instance->ConfigSetParameter(l_ConfigVideoGeneral,"Fullscreen",M64TYPE_BOOL, &fs);
		int sel = reslist.GetCurSel();
		int x_res = windowedModes[sel].width;
		int y_res = windowedModes[sel].height;
		emulator_instance->ConfigSetParameter(l_ConfigVideoGeneral,"ScreenWidth",M64TYPE_INT, &x_res);
		emulator_instance->ConfigSetParameter(l_ConfigVideoGeneral,"ScreenHeight",M64TYPE_INT, &y_res);
		m64p_error retval =emulator_instance->ConfigSaveFile();
		int size = ((x_res << 16) + y_res);
		emulator_instance->CoreDoCommand( M64CMD_CORE_STATE_SET,M64CORE_VIDEO_SIZE,&size);
   }

};


class CPluginVideo : public CDialogImpl<CPluginVideo>
{
public:
	CDialogTabCtrl m_ctrlTab;
	CPluginVideoGeneral view1;

	enum { IDD = IDD_VIDEOPLUGIN};

	BEGIN_MSG_MAP( CPluginVideo)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL,OnButtonCancel)
		COMMAND_ID_HANDLER(IDOK,OnButtonOK)
		REFLECT_NOTIFICATIONS();
	END_MSG_MAP()

	

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();
		m_ctrlTab.SubclassWindow(GetDlgItem(IDC_VIDEOTAB));
		view1.Create(m_hWnd);
		TCITEM tci = { 0 };
		tci.mask = TCIF_TEXT;
		tci.pszText = _T("General");
		m_ctrlTab.InsertItem(0, &tci, view1);
		m_ctrlTab.SetCurSel(0);
		return TRUE;
	}

	LRESULT OnButtonCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnButtonOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		view1.save_settings();
		EndDialog(wID);
		return 0;
	}

};

#endif PLUGINVIEW_H_INCLUDED