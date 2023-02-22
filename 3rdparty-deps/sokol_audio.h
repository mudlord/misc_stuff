#if defined(SOKOL_IMPL) && !defined(SOKOL_AUDIO_IMPL)
#define SOKOL_AUDIO_IMPL
#endif
#ifndef SOKOL_AUDIO_INCLUDED
/*
    sokol_audio.h -- cross-platform audio-streaming API

    Project URL: https://github.com/floooh/sokol

    Do this:
        #define SOKOL_IMPL or
        #define SOKOL_AUDIO_IMPL
    before you include this file in *one* C or C++ file to create the
    implementation.

    LICENSE
    =======

    zlib/libpng license

    Copyright (c) 2018 Andre Weissflog

    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.

        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.

        3. This notice may not be removed or altered from any source
        distribution.
*/
#define SOKOL_AUDIO_INCLUDED (1)
#include <stdint.h>
#include <stdbool.h>

#if defined(SOKOL_API_DECL) && !defined(SOKOL_AUDIO_API_DECL)
#define SOKOL_AUDIO_API_DECL SOKOL_API_DECL
#endif
#ifndef SOKOL_AUDIO_API_DECL
#if defined(_WIN32) && defined(SOKOL_DLL) && defined(SOKOL_AUDIO_IMPL)
#define SOKOL_AUDIO_API_DECL __declspec(dllexport)
#elif defined(_WIN32) && defined(SOKOL_DLL)
#define SOKOL_AUDIO_API_DECL __declspec(dllimport)
#else
#define SOKOL_AUDIO_API_DECL extern
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct saudio_desc {
    int sample_rate;        /* requested sample rate */
    int num_channels;       /* number of channels, default: 1 (mono) */
    int buffer_frames;      /* number of frames in streaming buffer */
    int packet_frames;      /* number of frames in a packet */
    int num_packets;        /* number of packets in packet queue */
    DWORD (*stream_cb)(float* buffer, int num_frames, int num_channels);  /* optional streaming callback (no user data) */
    DWORD (*stream_userdata_cb)(float* buffer, int num_frames, int num_channels, void* user_data); /*... and with user data */
    void* user_data;        /* optional user data argument for stream_userdata_cb */
} saudio_desc;

/* setup sokol-audio */
SOKOL_AUDIO_API_DECL void saudio_setup(const saudio_desc* desc);
/* shutdown sokol-audio */
SOKOL_AUDIO_API_DECL void saudio_shutdown(void);

#ifdef __cplusplus
} /* extern "C" */

/* reference-based equivalents for c++ */
inline void saudio_setup(const saudio_desc& desc) { return saudio_setup(&desc); }

#endif
#endif // SOKOL_AUDIO_INCLUDED

/*=== IMPLEMENTATION =========================================================*/
#ifdef SOKOL_AUDIO_IMPL
#define SOKOL_AUDIO_IMPL_INCLUDED (1)
#include <string.h> /* memset, memcpy */

#ifndef SOKOL_API_IMPL
    #define SOKOL_API_IMPL
#endif
#ifndef SOKOL_DEBUG
    #ifndef NDEBUG
        #define SOKOL_DEBUG (1)
    #endif
#endif
#ifndef SOKOL_ASSERT
    #include <assert.h>
    #define SOKOL_ASSERT(c) assert(c)
#endif
#ifndef SOKOL_MALLOC
    #include <stdlib.h>
    #define SOKOL_MALLOC(s) malloc(s)
    #define SOKOL_FREE(p) free(p)
#endif
#ifndef SOKOL_LOG
    #ifdef SOKOL_DEBUG
        #include <stdio.h>
        #define SOKOL_LOG(s) { SOKOL_ASSERT(s); puts(s); }
    #else
        #define SOKOL_LOG(s)
    #endif
#endif

#ifndef _SOKOL_PRIVATE
    #if defined(__GNUC__) || defined(__clang__)
        #define _SOKOL_PRIVATE __attribute__((unused)) static
    #else
        #define _SOKOL_PRIVATE static
    #endif
#endif

#ifndef _SOKOL_UNUSED
    #define _SOKOL_UNUSED(x) (void)(x)
#endif

#if defined(SOKOL_DUMMY_BACKEND)
    // No threads needed for SOKOL_DUMMY_BACKEND
#elif defined(_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <windows.h>
    #include <synchapi.h>
    #if (defined(WINAPI_FAMILY_PARTITION) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP))
        #define SOKOL_WIN32_NO_MMDEVICE
        #pragma comment (lib, "WindowsApp.lib")
    #else
        #pragma comment (lib, "kernel32.lib")
        #pragma comment (lib, "ole32.lib")
        #if defined(SOKOL_WIN32_NO_MMDEVICE)
            #pragma comment (lib, "mmdevapi.lib")
        #endif
    #endif
#endif

#if defined(SOKOL_DUMMY_BACKEND)
    // No audio API needed for SOKOL_DUMMY_BACKEND
#elif defined(_WIN32)
    #ifndef CINTERFACE
    #define CINTERFACE
    #endif
    #ifndef COBJMACROS
    #define COBJMACROS
    #endif
    #ifndef CONST_VTABLE
    #define CONST_VTABLE
    #endif
    #include <mmdeviceapi.h>
    #include <audioclient.h>
    static const IID _saudio_IID_IAudioClient = { 0x1cb9ad4c, 0xdbfa, 0x4c32, { 0xb1, 0x78, 0xc2, 0xf5, 0x68, 0xa7, 0x03, 0xb2 } };
    static const IID _saudio_IID_IMMDeviceEnumerator = { 0xa95664d2, 0x9614, 0x4f35, { 0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6 } };
    static const CLSID _saudio_CLSID_IMMDeviceEnumerator = { 0xbcde0395, 0xe52f, 0x467c, { 0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2e } };
    static const IID _saudio_IID_IAudioRenderClient = { 0xf294acfc, 0x3146, 0x4483,{ 0xa7, 0xbf, 0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2 } };
    static const IID _saudio_IID_Devinterface_Audio_Render = { 0xe6327cad, 0xdcec, 0x4949, {0xae, 0x8a, 0x99, 0x1e, 0x97, 0x6a, 0x79, 0xd2 } };
    static const IID _saudio_IID_IActivateAudioInterface_Completion_Handler = { 0x94ea2b94, 0xe9cc, 0x49e0, {0xc0, 0xff, 0xee, 0x64, 0xca, 0x8f, 0x5b, 0x90} };
    #if defined(__cplusplus)
    #define _SOKOL_AUDIO_WIN32COM_ID(x) (x)
    #else
    #define _SOKOL_AUDIO_WIN32COM_ID(x) (&x)
    #endif
    /* fix for Visual Studio 2015 SDKs */
    #ifndef AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
    #define AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM 0x80000000
    #endif
    #ifndef AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY
    #define AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY 0x08000000
    #endif
#endif
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable:4505)   /* unreferenced local function has been removed */
#endif

#define _saudio_def(val, def) (((val) == 0) ? (def) : (val))
#define _saudio_def_flt(val, def) (((val) == 0.0f) ? (def) : (val))

#define _SAUDIO_DEFAULT_SAMPLE_RATE (44100)
#define _SAUDIO_DEFAULT_BUFFER_FRAMES (2048)
#define _SAUDIO_DEFAULT_PACKET_FRAMES (128)
#define _SAUDIO_DEFAULT_NUM_PACKETS ((_SAUDIO_DEFAULT_BUFFER_FRAMES/_SAUDIO_DEFAULT_PACKET_FRAMES)*4)

#ifndef SAUDIO_RING_MAX_SLOTS
#define SAUDIO_RING_MAX_SLOTS (1024)
#endif

typedef struct {
    CRITICAL_SECTION critsec;
} _saudio_mutex_t;


typedef struct {
    HANDLE thread_handle;
    HANDLE buffer_end_event;
    bool stop;
    UINT32 dst_buffer_frames;
} _saudio_wasapi_thread_data_t;

typedef struct {

    IMMDeviceEnumerator* device_enumerator;
    IMMDevice* device;
    IAudioClient* audio_client;
    IAudioRenderClient* render_client;
    _saudio_wasapi_thread_data_t thread;
} _saudio_backend_t;

/*=== GENERAL DECLARATIONS ===================================================*/

/* sokol-audio state */
typedef struct {
    bool valid;
    DWORD (*stream_cb)(float* buffer, int num_frames, int num_channels);
    DWORD (*stream_userdata_cb)(float* buffer, int num_frames, int num_channels, void* user_data);
    void* user_data;
    int sample_rate;            /* sample rate */
    int buffer_frames;          /* number of frames in streaming buffer */
    int bytes_per_frame;        /* filled by backend */
    int packet_frames;          /* number of frames in a packet */
    int num_packets;            /* number of packets in packet queue */
    int num_channels;           /* actual number of channels */
    saudio_desc desc;
    _saudio_backend_t backend;
} _saudio_state_t;

static _saudio_state_t _saudio;

_SOKOL_PRIVATE bool _saudio_has_callback(void) {
    return (_saudio.stream_cb || _saudio.stream_userdata_cb);
}

_SOKOL_PRIVATE DWORD _saudio_stream_callback(float* buffer, int num_frames, int num_channels) {
    if (_saudio.stream_cb) {
       return _saudio.stream_cb(buffer, num_frames, num_channels);
    }
    else if (_saudio.stream_userdata_cb) {
       return _saudio.stream_userdata_cb(buffer, num_frames, num_channels, _saudio.user_data);
    }
}

/*=== MUTEX IMPLEMENTATION ===================================================*/
_SOKOL_PRIVATE void _saudio_mutex_init(_saudio_mutex_t* m) {
    InitializeCriticalSection(&m->critsec);
}

_SOKOL_PRIVATE void _saudio_mutex_destroy(_saudio_mutex_t* m) {
    DeleteCriticalSection(&m->critsec);
}

_SOKOL_PRIVATE void _saudio_mutex_lock(_saudio_mutex_t* m) {
    EnterCriticalSection(&m->critsec);
}

_SOKOL_PRIVATE void _saudio_mutex_unlock(_saudio_mutex_t* m) {
    LeaveCriticalSection(&m->critsec);
}

_SOKOL_PRIVATE void _saudio_wasapi_submit_buffer(UINT32 num_frames) {
    BYTE* wasapi_buffer = 0;
    if (FAILED(IAudioRenderClient_GetBuffer(_saudio.backend.render_client, num_frames, &wasapi_buffer))) {
        return;
    }
    SOKOL_ASSERT(wasapi_buffer);
    DWORD consumedBytes = _saudio_stream_callback((float*)wasapi_buffer, num_frames, _saudio.num_channels);
    DWORD samplesRead = ((num_frames * _saudio.num_channels) * sizeof(float));
    memset((BYTE*)wasapi_buffer + consumedBytes, 0, samplesRead-consumedBytes);
    IAudioRenderClient_ReleaseBuffer(_saudio.backend.render_client, num_frames, 0);
}

_SOKOL_PRIVATE DWORD WINAPI _saudio_wasapi_thread_fn(LPVOID param) {
    (void)param;
    _saudio_wasapi_submit_buffer(_saudio.buffer_frames);
    IAudioClient_Start(_saudio.backend.audio_client);
    while (!_saudio.backend.thread.stop) {
        WaitForSingleObject(_saudio.backend.thread.buffer_end_event, INFINITE);
        UINT32 padding = 0;
        if (FAILED(IAudioClient_GetCurrentPadding(_saudio.backend.audio_client, &padding))) {
            continue;
        }
        SOKOL_ASSERT(_saudio.backend.thread.dst_buffer_frames >= padding);
        UINT32 num_frames = _saudio.backend.thread.dst_buffer_frames - padding;
        if (num_frames > 0) {
            _saudio_wasapi_submit_buffer(num_frames);
        }
    }
    return 0;
}

_SOKOL_PRIVATE void _saudio_wasapi_release(void) {
    if (_saudio.backend.render_client) {
        IAudioRenderClient_Release(_saudio.backend.render_client);
        _saudio.backend.render_client = 0;
    }
    if (_saudio.backend.audio_client) {
        IAudioClient_Release(_saudio.backend.audio_client);
        _saudio.backend.audio_client = 0;
    }

    if (_saudio.backend.device) {
        IMMDevice_Release(_saudio.backend.device);
        _saudio.backend.device = 0;
    }
    if (_saudio.backend.device_enumerator) {
        IMMDeviceEnumerator_Release(_saudio.backend.device_enumerator);
        _saudio.backend.device_enumerator = 0;
    }

    if (0 != _saudio.backend.thread.buffer_end_event) {
        CloseHandle(_saudio.backend.thread.buffer_end_event);
        _saudio.backend.thread.buffer_end_event = 0;
    }
}

_SOKOL_PRIVATE bool _saudio_backend_init(void) {
    REFERENCE_TIME dur;
    /* UWP Threads are CoInitialized by default with a different threading model, and this call fails
    See https://github.com/Microsoft/cppwinrt/issues/6#issuecomment-253930637 */
#if (defined(WINAPI_FAMILY_PARTITION) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP))
    /* CoInitializeEx could have been called elsewhere already, in which
        case the function returns with S_FALSE (thus it doesn't make much
        sense to check the result)
    */
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    _SOKOL_UNUSED(hr);
#endif
    _saudio.backend.thread.buffer_end_event = CreateEvent(0, FALSE, FALSE, 0);
    if (0 == _saudio.backend.thread.buffer_end_event) {
        SOKOL_LOG("sokol_audio wasapi: failed to create buffer_end_event");
        goto error;
    }

    if (FAILED(CoCreateInstance(_SOKOL_AUDIO_WIN32COM_ID(_saudio_CLSID_IMMDeviceEnumerator),
        0, CLSCTX_ALL,
        _SOKOL_AUDIO_WIN32COM_ID(_saudio_IID_IMMDeviceEnumerator),
        (void**)&_saudio.backend.device_enumerator)))
    {
        SOKOL_LOG("sokol_audio wasapi: failed to create device enumerator");
        goto error;
    }
    if (FAILED(IMMDeviceEnumerator_GetDefaultAudioEndpoint(_saudio.backend.device_enumerator,
        eRender, eConsole,
        &_saudio.backend.device)))
    {
        SOKOL_LOG("sokol_audio wasapi: GetDefaultAudioEndPoint failed");
        goto error;
    }
    if (FAILED(IMMDevice_Activate(_saudio.backend.device,
        _SOKOL_AUDIO_WIN32COM_ID(_saudio_IID_IAudioClient),
        CLSCTX_ALL, 0,
        (void**)&_saudio.backend.audio_client)))
    {
        SOKOL_LOG("sokol_audio wasapi: device activate failed");
        goto error;
    }

    WAVEFORMATEX fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.nChannels = (WORD) _saudio.num_channels;
    fmt.nSamplesPerSec = _saudio.sample_rate;
    fmt.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    fmt.wBitsPerSample = 32;
    fmt.nBlockAlign = (fmt.nChannels * fmt.wBitsPerSample) / 8;
    fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;
    dur = (REFERENCE_TIME)
        (((double)_saudio.buffer_frames) / (((double)_saudio.sample_rate) * (1.0/10000000.0)));
    if (FAILED(IAudioClient_Initialize(_saudio.backend.audio_client,
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK|AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM|AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
        dur, 0, &fmt, 0)))
    {
        SOKOL_LOG("sokol_audio wasapi: audio client initialize failed");
        goto error;
    }
    if (FAILED(IAudioClient_GetBufferSize(_saudio.backend.audio_client, &_saudio.backend.thread.dst_buffer_frames))) {
        SOKOL_LOG("sokol_audio wasapi: audio client get buffer size failed");
        goto error;
    }
    if (FAILED(IAudioClient_GetService(_saudio.backend.audio_client,
        _SOKOL_AUDIO_WIN32COM_ID(_saudio_IID_IAudioRenderClient),
        (void**)&_saudio.backend.render_client)))
    {
        SOKOL_LOG("sokol_audio wasapi: audio client GetService failed");
        goto error;
    }
    if (FAILED(IAudioClient_SetEventHandle(_saudio.backend.audio_client, _saudio.backend.thread.buffer_end_event))) {
        SOKOL_LOG("sokol_audio wasapi: audio client SetEventHandle failed");
        goto error;
    }

    /* create streaming thread */
    _saudio.backend.thread.thread_handle = CreateThread(NULL, 0, _saudio_wasapi_thread_fn, 0, 0, 0);
    if (0 == _saudio.backend.thread.thread_handle) {
        SOKOL_LOG("sokol_audio wasapi: CreateThread failed");
        goto error;
    }
    return true;
error:
    _saudio_wasapi_release();
    return false;
}

_SOKOL_PRIVATE void _saudio_backend_shutdown(void) {
    if (_saudio.backend.thread.thread_handle) {
        _saudio.backend.thread.stop = true;
        SetEvent(_saudio.backend.thread.buffer_end_event);
        WaitForSingleObject(_saudio.backend.thread.thread_handle, INFINITE);
        CloseHandle(_saudio.backend.thread.thread_handle);
        _saudio.backend.thread.thread_handle = 0;
    }
    if (_saudio.backend.audio_client) {
        IAudioClient_Stop(_saudio.backend.audio_client);
    }
    _saudio_wasapi_release();

#if (defined(WINAPI_FAMILY_PARTITION) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP))
    CoUninitialize();
#endif
}

/*=== PUBLIC API FUNCTIONS ===================================================*/
SOKOL_API_IMPL void saudio_setup(const saudio_desc* desc) {
    SOKOL_ASSERT(!_saudio.valid);
    SOKOL_ASSERT(desc);
    memset(&_saudio, 0, sizeof(_saudio));
    _saudio.desc = *desc;
    _saudio.stream_cb = desc->stream_cb;
    _saudio.stream_userdata_cb = desc->stream_userdata_cb;
    _saudio.user_data = desc->user_data;
    _saudio.sample_rate = _saudio_def(_saudio.desc.sample_rate, _SAUDIO_DEFAULT_SAMPLE_RATE);
    _saudio.buffer_frames = _saudio_def(_saudio.desc.buffer_frames, _SAUDIO_DEFAULT_BUFFER_FRAMES);
    _saudio.packet_frames = _saudio_def(_saudio.desc.packet_frames, _SAUDIO_DEFAULT_PACKET_FRAMES);
    _saudio.num_packets = _saudio_def(_saudio.desc.num_packets, _SAUDIO_DEFAULT_NUM_PACKETS);
    _saudio.num_channels = _saudio_def(_saudio.desc.num_channels, 1);
    if (_saudio_backend_init()) {
        /* the backend might not support the requested exact buffer size,
           make sure the actual buffer size is still a multiple of
           the requested packet size
        */
        if (0 != (_saudio.buffer_frames % _saudio.packet_frames)) {
            SOKOL_LOG("sokol_audio.h: actual backend buffer size isn't multiple of requested packet size");
            _saudio_backend_shutdown();
            return;
        }
        _saudio.valid = true;
    }
}

SOKOL_API_IMPL void saudio_shutdown(void) {
    if (_saudio.valid) {
        _saudio_backend_shutdown();
        _saudio.valid = false;
    }
}

#undef _saudio_def
#undef _saudio_def_flt

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
