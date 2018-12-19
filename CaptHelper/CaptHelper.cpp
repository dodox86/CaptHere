// CaptHelper.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"

typedef struct tagAPPHOOKINFO {
    HHOOK   hHook;
    HWND    hWnd;

    UINT    message;
    WORD    forForegroundWindow;
    WORD    forScreen;
    bool    postedForegroundWindow;
    bool    postedScreen;

    union {
        UINT    keys;
        struct {
            UINT snapshot : 1;
            UINT ctrl : 1;
            UINT alt : 1;
        } key;
    } keyman;

} APPHOOKINFO;

static APPHOOKINFO g_info;

void InitAppHookInfoEx(APPHOOKINFO* pinfo)
{
    pinfo->hHook = NULL;
    pinfo->hWnd = NULL;
    pinfo->message = 0;
    pinfo->postedForegroundWindow = false;
    pinfo->postedScreen = false;;
    pinfo->keyman.keys = 0;
}

void InitAppHookInfo(void)
{
    InitAppHookInfoEx(&g_info);
}

static bool IsAttached(void)
{
    return (g_info.hHook != NULL);
}

static LRESULT CALLBACK KeyProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    bool handle = !(g_info.hWnd == NULL || g_info.message == 0);
    if (handle && nCode == HC_ACTION) {
        LPKBDLLHOOKSTRUCT ps = (LPKBDLLHOOKSTRUCT)lParam;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            switch (ps->vkCode) {
            case VK_SNAPSHOT:
                g_info.keyman.key.snapshot = 1;
                break;

            case VK_CONTROL:
            case VK_LCONTROL:
            case VK_RCONTROL:
                g_info.keyman.key.ctrl = 1;
                break;

            case VK_MENU:
            case VK_LMENU:
            case VK_RMENU:
                g_info.keyman.key.alt = 1;
                break;

            default:
                break;
            }
        }
        else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            switch (ps->vkCode) {
            case VK_SNAPSHOT:
                g_info.keyman.key.snapshot = 0;
                g_info.postedForegroundWindow = false;
                g_info.postedScreen = false;
                break;

                // CTRL
            case VK_CONTROL:
            case VK_LCONTROL:
            case VK_RCONTROL:
                g_info.keyman.key.ctrl = 0;
                g_info.postedScreen = false;
                break;


                // ALT
            case VK_MENU:
            case VK_LMENU:
            case VK_RMENU:
                g_info.keyman.key.alt = 0;
                g_info.postedForegroundWindow = false;
                break;
            default:
                break;
            }
        }

        if (g_info.keyman.key.snapshot) {
            if (g_info.keyman.key.ctrl && !g_info.postedScreen) {
                PostMessage(g_info.hWnd, g_info.message, g_info.forScreen, 0);
                g_info.postedScreen = true;
            }
            else if (g_info.keyman.key.alt && !g_info.postedForegroundWindow) {
                PostMessage(g_info.hWnd, g_info.message, g_info.forForegroundWindow, 0);
                g_info.postedForegroundWindow = true;
            }
        }
    }

    return CallNextHookEx(g_info.hHook, nCode, wParam, lParam);
}

static void Detach(void)
{
    if (g_info.hHook != NULL) {
        UnhookWindowsHookEx(g_info.hHook);
        g_info.hHook = NULL;
        g_info.hWnd = NULL;
    }
}

static BOOL Attach(HWND hWnd)
{
    Detach();

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
    g_info.hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyProc, hInst, 0);
    if (g_info.hHook != NULL) {
        g_info.hWnd = hWnd;
    }
    return (g_info.hHook != NULL);
}

static BOOL RegisterNotification(UINT message, WORD forForegroundWindow, WORD forScreen)
{
    BOOL attached = IsAttached();
    if (attached) {
        g_info.message = message;
        g_info.forForegroundWindow = forForegroundWindow;
        g_info.forScreen = forScreen;
        g_info.keyman.keys = 0;
    }
    return attached;
}

BOOL __stdcall CaptHelper_AttachWindow(HWND hWnd)
{
    return Attach(hWnd);
}

void __stdcall CaptHelper_DetatchWindow(void)
{
    return Detach();
}

BOOL __stdcall CaptHelper_RegisterNotification(UINT message, WORD forForegroundWindow, WORD forScreen)
{
    return RegisterNotification(message, forForegroundWindow, forScreen);
}

BOOL __stdcall CaptHelper_UnregisterNotification(void)
{
    return RegisterNotification(0, 0, 0);
}


