// SettingsDlg.cpp

#include "stdafx.h"
#include <shellapi.h>
#include <ShlObj.h>

#include "resource.h"
#include "CaptHere.h"

#pragma comment(lib, "Shell32.lib")

typedef struct SETTINGSDIALOGDATA {
    SETTINGS*   psourceSettings;
    SETTINGS    settings;
    HINSTANCE   hInstance;
    HWND    hWndMain;
    TCHAR   szName[256];
} SETTINGSDIALOGDATA;

static BOOL OnInitDialog(HWND hDlg, LPARAM lParam);
static BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
static void OnDestroy(void);
static BOOL ApplyWindowRect(HWND hDlg);
static BOOL ChooseFolder(HWND hWnd, LPTSTR pszName, LPTSTR pszFolder);
UINT ShowMessageBox(HWND hWnd, UINT id, UINT utype);

static SETTINGSDIALOGDATA *g_pdata = NULL;

static INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL proceeded = FALSE;
    switch (message)
    {
    case WM_INITDIALOG:
        proceeded = OnInitDialog(hDlg, lParam);
        break;

    case WM_COMMAND:
        proceeded = OnCommand(hDlg, wParam, lParam);
        break;

    case WM_DESTROY:
        OnDestroy();
        break;

    default:
        break;
    }

    return (INT_PTR)proceeded;
}

INT_PTR ShowSettingsDialog(HINSTANCE hInstance, HWND hWndParent, SETTINGS* psettings)
{
    return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SETTINGS), hWndParent, SettingsDlgProc, (LPARAM)psettings);
}

static BOOL OnInitDialog(HWND hDlg, LPARAM lParam)
{
    ATLASSERT(lParam != 0L);

    g_pdata = (SETTINGSDIALOGDATA *)malloc(sizeof(*g_pdata));
    ATLASSERT(g_pdata != NULL);

    g_pdata->hInstance = (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE);
    g_pdata->hWndMain = GetParent(hDlg);
    g_pdata->psourceSettings = (SETTINGS*)lParam;
    g_pdata->settings = *(SETTINGS*)(lParam);
    SetWindowPos(hDlg, NULL, g_pdata->settings.x, g_pdata->settings.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    SetDlgItemInt(hDlg, IDC_EDIT_SNAP, g_pdata->settings.snap, FALSE);
    SetDlgItemText(hDlg, IDC_EDIT_FOLDER, g_pdata->settings.szSaveFolder);

    // デスクトップウィンドウのサイズを上限とする
    RECT rect;
    GetWindowRect(GetDesktopWindow(), &rect);
    SendDlgItemMessage(hDlg, IDC_SPIN_X, UDM_SETRANGE, 0, MAKELPARAM(0, rect.right));
    SendDlgItemMessage(hDlg, IDC_SPIN_Y, UDM_SETRANGE, 0, MAKELPARAM(0, rect.bottom));
    SendDlgItemMessage(hDlg, IDC_SPIN_WIDTH, UDM_SETRANGE, 0, MAKELPARAM(0, rect.right));
    SendDlgItemMessage(hDlg, IDC_SPIN_HEIGHT, UDM_SETRANGE, 0, MAKELPARAM(0, rect.bottom));

    SendDlgItemMessage(hDlg, IDC_SPIN_X, UDM_SETPOS, 0, g_pdata->settings.wndRect.left);
    SendDlgItemMessage(hDlg, IDC_SPIN_Y, UDM_SETPOS, 0, g_pdata->settings.wndRect.top);
    SendDlgItemMessage(hDlg, IDC_SPIN_WIDTH, UDM_SETPOS, 0, g_pdata->settings.wndRect.right - g_pdata->settings.wndRect.left);
    SendDlgItemMessage(hDlg, IDC_SPIN_HEIGHT, UDM_SETPOS, 0, g_pdata->settings.wndRect.bottom - g_pdata->settings.wndRect.top);

    return TRUE;
}

static void OnDestroy(void)
{
    if (g_pdata != NULL) {
        free(g_pdata);
        g_pdata = NULL;
    }
}

static BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM /*lParam*/)
{
    BOOL proceeded = FALSE;

    int id = LOWORD(wParam);
    switch (id) {
    case IDOK:
    case IDCANCEL:
        if (id == IDOK) {
            BOOL translated = FALSE;
            UINT ret = GetDlgItemInt(hDlg, IDC_EDIT_SNAP, &translated, FALSE);
            if (translated) {
                g_pdata->settings.snap = ret;
            }
            GetDlgItemText(hDlg, IDC_EDIT_FOLDER, g_pdata->settings.szSaveFolder, _countof(g_pdata->settings.szSaveFolder));
            ApplyWindowRect(hDlg);
            *g_pdata->psourceSettings = g_pdata->settings;
        }
        EndDialog(hDlg, id);
        proceeded = TRUE;
        break;

    case IDC_BUTTON_FOLDER:
    {
        BOOL succeeded = ChooseFolder(hDlg, g_pdata->szName, g_pdata->settings.szSaveFolder);
        if (succeeded) {
            SetDlgItemText(hDlg, IDC_EDIT_FOLDER, g_pdata->settings.szSaveFolder);
        }
    }
    break;

    case IDC_BUTTON_FOLDEROPEN:
        ShellExecute(NULL, NULL, g_pdata->settings.szSaveFolder, NULL, NULL, SW_SHOWNORMAL);
        break;

    case IDC_BUTTON_APPLY:
        ApplyWindowRect(hDlg);
        break;

    default:
        break;
    }

    return proceeded;
}

static BOOL ApplyWindowRectEx(HWND hDlg)
{
    int x = -1;
    int y = -1;
    int width = -1;
    int height = -1;

    int v;
    BOOL translated;
    v = GetDlgItemInt(hDlg, IDC_EDIT_X, &translated, FALSE);
    if (translated) {
        x = v;
    }

    v = GetDlgItemInt(hDlg, IDC_EDIT_Y, &translated, FALSE);
    if (translated) {
        y = v;
    }

    v = GetDlgItemInt(hDlg, IDC_EDIT_WIDTH, &translated, FALSE);
    if (translated) {
        width = v;
    }

    v = GetDlgItemInt(hDlg, IDC_EDIT_HEIGHT, &translated, FALSE);
    if (translated) {
        height = v;
    }

    BOOL succeeded = FALSE;
    if (x != -1 && y != -1 && width != -1 && height != -1) {
        MoveWindow(g_pdata->hWndMain, x, y, width, height, TRUE);
        RECT rect = { x, y, x + width, y + height };
        SendMessage(g_pdata->hWndMain, WM_CAPTHEREAPP, SUBCOMMAND_ADJUST, (LPARAM)&rect);
        succeeded = TRUE;
    }
    return succeeded;
}

static BOOL ApplyWindowRect(HWND hDlg)
{
    BOOL succeeded = ApplyWindowRectEx(hDlg);
    if (!succeeded) {
        ShowMessageBox(hDlg, IDS_INVALIDREGION, MB_OK);
    }
    return succeeded;
}

static BOOL ChooseFolder(HWND /*hWnd*/, LPTSTR pszName, LPTSTR pszFolder)
{
    BROWSEINFO  binfo;
    LPITEMIDLIST idlist;

    TCHAR   szTitle[256];
    LoadString(g_pdata->hInstance, IDS_CHOOSEFOLDER_TITLE, szTitle, _countof(szTitle));

    binfo.hwndOwner = NULL;
    binfo.pidlRoot = NULL;
    binfo.pszDisplayName = pszName;
    binfo.lpszTitle = szTitle;
    binfo.ulFlags = BIF_RETURNONLYFSDIRS;
    binfo.lpfn = NULL;
    binfo.lParam = 0;
    binfo.iImage = 0;

    BOOL succeeded = FALSE;
    idlist = SHBrowseForFolder(&binfo);
    if (idlist != NULL) {
        SHGetPathFromIDList(idlist, pszFolder);
        ATLTRACE2(_T("name[%s], dir[%s]\n"), pszName, pszFolder);
        CoTaskMemFree(idlist);
        succeeded = TRUE;

        ATLTRACE2(_T("name=[%s], dir=[%s]\n"), pszName, pszFolder);
    }

    return succeeded;
}

UINT ShowMessageBox(HWND hWnd, UINT id, UINT utype)
{
    TCHAR   message[256];
    LoadString(g_pdata->hInstance, id, message, _countof(message));

    TCHAR   caption[256];
    GetWindowText(hWnd, caption, _countof(caption));

    return MessageBox(hWnd, message, caption, utype);
}

