#define INITGUID
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <Objbase.h>
#include <Shobjidl.h>
#include <MMDeviceAPI.h>
#include <Mmreg.h>
#include <locale.h>
#include <Functiondiscoverykeys_devpkey.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "quartz.lib")
#pragma comment(lib, "uuid.lib")

#define NUM_FREQ 192000
#define NUM_BITS 32
#define NUM_CHANNELS 2

#include "../3rdparty-deps/cmdline.h"


DEFINE_PROPERTYKEY(PKEY_InternalAudioFormat, 0xe4870e26, 0x3cc5, 0x4cd2, 0xba, 0x46, 0xca, 0xa, 0x9a, 0x70, 0xed, 0x4, 0);
DEFINE_PROPERTYKEY(PKEY_InternalAudioFormat2, 0x3d6e1656, 0x2e50, 0x4c4c, 0x8d, 0x85, 0xd0, 0xac, 0xae, 0x3c, 0x6c, 0x68, 2);
DEFINE_PROPERTYKEY(PKEY_InternalAudioFormat3, 0x3d6e1656, 0x2e50, 0x4c4c, 0x8d, 0x85, 0xd0, 0xac, 0xae, 0x3c, 0x6c, 0x68, 3);
DEFINE_PROPERTYKEY(PKEY_InternalAudioFormat4, 0x624f56de, 0xfd24, 0x473e, 0x81, 0x4a, 0xde, 0x40, 0xaa, 0xca, 0xed, 0x16, 3);

bool SetFormatProperty(IPropertyStore* propertyStore, PROPERTYKEY pkey, int freq, int channels, int bitsPerSample) {

	HRESULT hr;
	PROPVARIANT format;
	PropVariantInit(&format);
	WAVEFORMATEXTENSIBLE wext;

	hr = propertyStore->GetValue(pkey, &format);
	if (FAILED(hr)) { printf("Can't get property: HRESULT 0x%x\n", hr); return false; }

	if (format.blob.cbSize != sizeof(WAVEFORMATEXTENSIBLE)) {
		return false;
	}

	memcpy_s(&wext, sizeof(wext), format.blob.pBlobData, sizeof(wext));

	wext.Format.nChannels = channels;
	wext.Format.nSamplesPerSec = freq;

	int new_bits = wext.Format.wBitsPerSample;
	int new_valid_bits = wext.Samples.wValidBitsPerSample;

	if (bitsPerSample != 0) {
		new_bits = bitsPerSample;
		new_valid_bits = bitsPerSample;
	}

	PROPVARIANT varName = { 0 };
	PropVariantInit(&varName);
	varName.vt = VT_BLOB;
	varName.blob.cbSize = sizeof(WAVEFORMATEXTENSIBLE);

	int alignment = wext.Format.nChannels * new_bits / 8;

	WAVEFORMATEXTENSIBLE fmt = {
		{
				WAVE_FORMAT_EXTENSIBLE,
				wext.Format.nChannels,
				wext.Format.nSamplesPerSec,
				wext.Format.nSamplesPerSec * alignment,
				alignment,
				new_bits,
				(sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))
		},
		{new_valid_bits},
		wext.dwChannelMask,
		wext.SubFormat
	};
	varName.blob.pBlobData = (BYTE*)&fmt;

	hr = propertyStore->SetValue(pkey, varName);
	if (hr != S_OK)printf("SetValue failed. HRESULT: 0x%x\n", (int)hr);

	PropVariantClear(&format);
	return (SUCCEEDED(hr));

}

int _tmain(int argc, char* argv[])
{
	DWORD dwMode = STGM_READWRITE;

	cmdline::parser a;
	a.add<int>("sample_rate", 's', "sample rate", true, NUM_FREQ, cmdline::range(800, 192000));
	a.add<int>("bits", 'b', "bits per sample", true, NUM_BITS, cmdline::range(8, 32));
	a.add<int>("channels", 'c', "number of channels", true, NUM_CHANNELS, cmdline::range(1, 8));
	bool ret = a.parse(argc, argv);
	if (!ret) {
		printf("%s\n", a.usage());
		return 0;
	}

	CoInitialize(NULL);

	TCHAR* pstr = NULL;
	DWORD state;

	IMMDeviceEnumerator* deviceEnumerator = NULL;
	IMMDeviceCollection* deviceCollection = NULL;
	IMMDevice* dev = NULL;
	IPropertyStore* propertyStore = NULL;

	WAVEFORMATEXTENSIBLE wext;
	bool res;

	//Create Device Enumerator
	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator));
	if (FAILED(hr)) { printf("CoCreateInstance fail: HRESULT 0x%x", hr); goto End; }

	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &dev);
	if (FAILED(hr)) { printf("EnumAudioEndpoints fail: HRESULT 0x%x\n", hr); goto End; }
	hr = dev->OpenPropertyStore(dwMode, &propertyStore);
	if (FAILED(hr)) { printf("Can't open property store: HRESULT 0x%x\n", hr); goto End; }
	res = SetFormatProperty(propertyStore, PKEY_AudioEngine_DeviceFormat, a.get<int>("sample_rate"), a.get<int>("channels"), a.get<int>("bits"));
	if (!res) { printf("Can't set PKEY_AudioEngine_DeviceFormat!\n"); goto End; }
	SetFormatProperty(propertyStore, PKEY_InternalAudioFormat, a.get<int>("sample_rate"), a.get<int>("channels"), 0);
	SetFormatProperty(propertyStore, PKEY_InternalAudioFormat2, a.get<int>("sample_rate"), a.get<int>("channels"), 0);
	SetFormatProperty(propertyStore, PKEY_InternalAudioFormat3, a.get<int>("sample_rate"), a.get<int>("channels"), 0);
	SetFormatProperty(propertyStore, PKEY_InternalAudioFormat4, a.get<int>("sample_rate"), a.get<int>("channels"), 0);
End:if (dev != NULL) { dev->Release(); dev = NULL; }
	if (propertyStore != NULL) { propertyStore->Release(); propertyStore = NULL; }
	if (deviceEnumerator != NULL) { deviceEnumerator->Release(); deviceEnumerator = NULL; }
	if (deviceCollection != NULL) { deviceCollection->Release(); deviceCollection = NULL; }
	CoUninitialize();
	system("PAUSE");
	return 0;
}