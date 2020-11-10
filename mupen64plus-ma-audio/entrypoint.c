#include <windows.h>
#include "3rdparty/m64p_config.h"
#include "audio.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#define M64P_PLUGIN_PROTOTYPES 1
#include "3rdparty/m64p_common.h"
#include "3rdparty/m64p_config.h"
#include "3rdparty/m64p_plugin.h"
#include "3rdparty/m64p_types.h"

/* version info */
#define AUDIO_PLUGIN_VERSION 0x020509
#define AUDIO_PLUGIN_API_VERSION 0x020000
#define CONFIG_API_VERSION       0x020100
#define CONFIG_PARAM_VERSION     1.00
#define VERSION_PRINTF_SPLIT(x) (((x) >> 16) & 0xffff), (((x) >> 8) & 0xff), ((x) & 0xff)

static void (*l_DebugCallback)(void*, int, const char*) = NULL;
static void* l_DebugCallContext = NULL;
static int l_PluginInit = 0;
static m64p_handle l_ConfigAudio;
static AUDIO_INFO AudioInfo;

ptr_ConfigOpenSection      ConfigOpenSection = NULL;
ptr_ConfigDeleteSection    ConfigDeleteSection = NULL;
ptr_ConfigSetParameter     ConfigSetParameter = NULL;
ptr_ConfigGetParameter     ConfigGetParameter = NULL;
ptr_ConfigGetParameterHelp ConfigGetParameterHelp = NULL;
ptr_ConfigSetDefaultInt    ConfigSetDefaultInt = NULL;
ptr_ConfigSetDefaultFloat  ConfigSetDefaultFloat = NULL;
ptr_ConfigSetDefaultBool   ConfigSetDefaultBool = NULL;
ptr_ConfigSetDefaultString ConfigSetDefaultString = NULL;
ptr_ConfigGetParamInt      ConfigGetParamInt = NULL;
ptr_ConfigGetParamFloat    ConfigGetParamFloat = NULL;
ptr_ConfigGetParamBool     ConfigGetParamBool = NULL;
ptr_ConfigGetParamString   ConfigGetParamString = NULL;

void DebugMessage(int level, const char* message, ...)
{
	char msgbuf[1024];
	va_list args;

	if (l_DebugCallback == NULL)
		return;

	va_start(args, message);
	vsprintf(msgbuf, message, args);

	(*l_DebugCallback)(l_DebugCallContext, level, msgbuf);

	va_end(args);
}

/* Mupen64Plus plugin functions */
EXPORT m64p_error CALL PluginStartup(m64p_dynlib_handle CoreLibHandle, void* Context,
	void (*DebugCallback)(void*, int, const char*))
{
	ptr_CoreGetAPIVersions CoreAPIVersionFunc;

	int ConfigAPIVersion, DebugAPIVersion, VidextAPIVersion;
	float fConfigParamsVersion = 0.0f;

	if (l_PluginInit)
		return M64ERR_ALREADY_INIT;

	/* first thing is to set the callback function for debug info */
	l_DebugCallback = DebugCallback;
	l_DebugCallContext = Context;

	/* attach and call the CoreGetAPIVersions function, check Config API version for compatibility */
	CoreAPIVersionFunc = (ptr_CoreGetAPIVersions)osal_dynlib_getproc(CoreLibHandle, "CoreGetAPIVersions");
	if (CoreAPIVersionFunc == NULL)
	{
		DebugMessage(M64MSG_ERROR, "Core emulator broken; no CoreAPIVersionFunc() function found.");
		return M64ERR_INCOMPATIBLE;
	}

	(*CoreAPIVersionFunc)(&ConfigAPIVersion, &DebugAPIVersion, &VidextAPIVersion, NULL);
	if ((ConfigAPIVersion & 0xffff0000) != (CONFIG_API_VERSION & 0xffff0000))
	{
		DebugMessage(M64MSG_ERROR, "Emulator core Config API (v%i.%i.%i) incompatible with plugin (v%i.%i.%i)",
			VERSION_PRINTF_SPLIT(ConfigAPIVersion), VERSION_PRINTF_SPLIT(CONFIG_API_VERSION));
		return M64ERR_INCOMPATIBLE;
	}

	/* Get the core config function pointers from the library handle */
	ConfigOpenSection = (ptr_ConfigOpenSection)osal_dynlib_getproc(CoreLibHandle, "ConfigOpenSection");
	ConfigDeleteSection = (ptr_ConfigDeleteSection)osal_dynlib_getproc(CoreLibHandle, "ConfigDeleteSection");
	ConfigSetParameter = (ptr_ConfigSetParameter)osal_dynlib_getproc(CoreLibHandle, "ConfigSetParameter");
	ConfigGetParameter = (ptr_ConfigGetParameter)osal_dynlib_getproc(CoreLibHandle, "ConfigGetParameter");
	ConfigSetDefaultInt = (ptr_ConfigSetDefaultInt)osal_dynlib_getproc(CoreLibHandle, "ConfigSetDefaultInt");
	ConfigSetDefaultFloat = (ptr_ConfigSetDefaultFloat)osal_dynlib_getproc(CoreLibHandle, "ConfigSetDefaultFloat");
	ConfigSetDefaultBool = (ptr_ConfigSetDefaultBool)osal_dynlib_getproc(CoreLibHandle, "ConfigSetDefaultBool");
	ConfigSetDefaultString = (ptr_ConfigSetDefaultString)osal_dynlib_getproc(CoreLibHandle, "ConfigSetDefaultString");
	ConfigGetParamInt = (ptr_ConfigGetParamInt)osal_dynlib_getproc(CoreLibHandle, "ConfigGetParamInt");
	ConfigGetParamFloat = (ptr_ConfigGetParamFloat)osal_dynlib_getproc(CoreLibHandle, "ConfigGetParamFloat");
	ConfigGetParamBool = (ptr_ConfigGetParamBool)osal_dynlib_getproc(CoreLibHandle, "ConfigGetParamBool");
	ConfigGetParamString = (ptr_ConfigGetParamString)osal_dynlib_getproc(CoreLibHandle, "ConfigGetParamString");

	if (!ConfigOpenSection || !ConfigDeleteSection || !ConfigSetParameter || !ConfigGetParameter ||
		!ConfigSetDefaultInt || !ConfigSetDefaultFloat || !ConfigSetDefaultBool || !ConfigSetDefaultString ||
		!ConfigGetParamInt || !ConfigGetParamFloat || !ConfigGetParamBool || !ConfigGetParamString)
		return M64ERR_INCOMPATIBLE;

	/* get a configuration section handle */
	if (ConfigOpenSection("Audio-SDL", &l_ConfigAudio) != M64ERR_SUCCESS)
	{
		DebugMessage(M64MSG_ERROR, "Couldn't open config section 'Audio-SDL'");
		return M64ERR_INPUT_NOT_FOUND;
	}

	/* check the section version number */
	if (ConfigGetParameter(l_ConfigAudio, "Version", M64TYPE_FLOAT, &fConfigParamsVersion, sizeof(float)) != M64ERR_SUCCESS)
	{
		DebugMessage(M64MSG_WARNING, "No version number in 'audio-miniaudio' config section. Setting defaults.");
		ConfigDeleteSection("Audio-SDL");
		ConfigOpenSection("Audio-SDL", &l_ConfigAudio);
	}
	else if (((int)fConfigParamsVersion) != ((int)CONFIG_PARAM_VERSION))
	{
		DebugMessage(M64MSG_WARNING, "Incompatible version %.2f in 'audio-miniaudio' config section: current is %.2f. Setting defaults.", fConfigParamsVersion, (float)CONFIG_PARAM_VERSION);
		ConfigDeleteSection("Audio-SDL");
		ConfigOpenSection("Audio-SDL", &l_ConfigAudio);
	}
	else if ((CONFIG_PARAM_VERSION - fConfigParamsVersion) >= 0.0001f)
	{
		/* handle upgrades */
		float fVersion = CONFIG_PARAM_VERSION;
		ConfigSetParameter(l_ConfigAudio, "Version", M64TYPE_FLOAT, &fVersion);
		DebugMessage(M64MSG_INFO, "Updating parameter set version in 'Audio-SDL' config section to %.2f", fVersion);
	}

	/* set the default values for this plugin */

	l_PluginInit = 1;
	return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginShutdown(void)
{
	if (!l_PluginInit)
		return M64ERR_NOT_INIT;

	/* reset some local variables */
	l_DebugCallback = NULL;
	l_DebugCallContext = NULL;

	l_PluginInit = 0;
	return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginGetVersion(m64p_plugin_type* PluginType, int* PluginVersion, int* APIVersion, const char** PluginNamePtr, int* Capabilities)
{
	/* set version info */
	if (PluginType != NULL)
		*PluginType = M64PLUGIN_AUDIO;

	if (PluginVersion != NULL)
		*PluginVersion = AUDIO_PLUGIN_VERSION;

	if (APIVersion != NULL)
		*APIVersion = AUDIO_PLUGIN_API_VERSION;

	if (PluginNamePtr != NULL)
		*PluginNamePtr = "Mupen64Plus Miniaudio Audio Plugin";

	if (Capabilities != NULL)
	{
		*Capabilities = 0;
	}

	return M64ERR_SUCCESS;
}

static unsigned int vi_clock_from_system_type(int system_type)
{
	switch (system_type)
	{
	default:
		DebugMessage(M64MSG_WARNING, "Invalid system_type %d. Assuming NTSC", system_type);
		/* fallback */
	case SYSTEM_NTSC: return 48681812;
	case SYSTEM_PAL:  return 49656530;
	case SYSTEM_MPAL: return 48628316;
	}
}

static unsigned int dacrate2freq(unsigned int vi_clock, uint32_t dacrate)
{
	return vi_clock / (dacrate + 1);
}
static init = false;
EXPORT void CALL AiDacrateChanged(int SystemType)
{
	if (!l_PluginInit)
		return;

	unsigned int frequency = dacrate2freq(vi_clock_from_system_type(SystemType), *AudioInfo.AI_DACRATE_REG);
	unsigned int bits = 1 + (*AudioInfo.AI_BITRATE_REG);
	
	if (!init)
	{
		audio_init(60, frequency, SystemType == SYSTEM_NTSC ? 60 : 50);
		init = true;
	}
	else
	audio_setsrate(frequency);
}

EXPORT void CALL AiLenChanged(void)
{
	if (!l_PluginInit)
		return;
	if(init)
	audio_mix(AudioInfo.RDRAM + (*AudioInfo.AI_DRAM_ADDR_REG & 0xffffff), *AudioInfo.AI_LEN_REG);
}

EXPORT int CALL InitiateAudio(AUDIO_INFO Audio_Info)
{
	if (!l_PluginInit)
		return 0;

	AudioInfo = Audio_Info;
	return 1;
}

EXPORT int CALL RomOpen(void)
{
	if (!l_PluginInit)
		return 0;

	return 1;
}

EXPORT void CALL RomClosed(void)
{
	if (!l_PluginInit)
		return;
	audio_destroy();
}

EXPORT void CALL ProcessAList(void)
{
}

EXPORT void CALL SetSpeedFactor(int percentage)
{
	if (!l_PluginInit)
		return;
}

EXPORT void CALL VolumeMute(void)
{
	if (!l_PluginInit)
		return;
}

EXPORT void CALL VolumeUp(void)
{
	if (!l_PluginInit)
		return;
}

EXPORT void CALL VolumeDown(void)
{
	if (!l_PluginInit)
		return;
}

EXPORT int CALL VolumeGetLevel(void)
{
	return 100;
}

EXPORT void CALL VolumeSetLevel(int level)
{
	if (!l_PluginInit)
		return;
}

EXPORT const char* CALL VolumeGetString(void)
{
	static char VolumeString[32];
	sprintf(VolumeString, "%i%%", 100);
	return VolumeString;
}