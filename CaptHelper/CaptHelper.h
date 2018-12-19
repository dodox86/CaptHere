#pragma once

#ifdef CAPTHELPER_EXPORTS
#define CAPTHELPER_API __declspec(dllexport)
#else
#define CAPTHELPER_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

    CAPTHELPER_API BOOL __stdcall CaptHelper_AttachWindow(HWND hWnd);
    CAPTHELPER_API void __stdcall CaptHelper_DetatchWindow(void);
    CAPTHELPER_API BOOL __stdcall CaptHelper_RegisterNotification(UINT message, WORD forForegroundWindow, WORD forScreen);
    CAPTHELPER_API BOOL __stdcall CaptHelper_UnregisterNotification(void);

#ifdef __cplusplus
}
#endif
