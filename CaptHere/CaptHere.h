#pragma once

#include "resource.h"

#define BORDER_WIDTH 6

// window border color for active
#define BORDERCOLOR_ACTIVE      RGB(0,220,0)

// window border color for inactive
#define BORDERCOLOR_INACTIVE    RGB(255,200,75)

// alpha value for main window 
#define ALPHA_VALUE     50

#define REGVALUE_SAVEFOLDER     "SaveFolder"

#define WM_CAPTHEREAPP  (WM_APP + 1)

typedef enum tagSUBCOMMAND {
    SUBCOMMAND_BASE = 0,
    SUBCOMMAND_CAPTURE = SUBCOMMAND_BASE,
    SUBCOMMAND_ACTIVATE,
    SUBCOMMAND_ADJUST,
} SUBCOMMAND;

typedef struct tagSETTINGS {
    UINT snap;
    int x;
    int y;
    RECT    wndRect;
    TCHAR   szSaveFolder[MAX_PATH];
} SETTINGS;

INT_PTR ShowSettingsDialog(HINSTANCE hInstance, HWND hWndParent, SETTINGS* psettings);

ATOM RegisterHandleWindowClass(HINSTANCE hInstance);
HWND CreateHandleWindow(HINSTANCE hInstance, HWND hWndParent, int x, int y, int width, int height, int nCmdShow);

BOOL GetRegistryString(LPCTSTR pszValueName, LPTSTR pszValue, size_t nValueLen);
BOOL PutRegistryString(LPCTSTR pszValueName, LPCTSTR pszValue);
void CheckLastError(BOOL succeeded);

