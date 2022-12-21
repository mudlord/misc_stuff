#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) void DLL_Play(HWND hwnd,void *data, int len, int subsong);
__declspec(dllexport) void DLL_Stop();

#ifdef __cplusplus
}
#endif