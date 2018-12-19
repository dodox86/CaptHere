// CaptHere.cpp
//

#include "stdafx.h"
#include <shlwapi.h>
#include "CaptHere.h"

#pragma comment(lib, "Shlwapi.lib")

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
static LRESULT OnNcHittest(HWND hWnd, LPARAM lParam);
static LRESULT OnMoving(HWND hWnd, LPRECT prect);
static LRESULT OnMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnNcRButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnActivate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnApp(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
void Capture(HWND hWnd);
void CaptureEx(LPCTSTR pszFileName, LPCRECT prect);
static void MakeFileName(LPTSTR pszFileName);
static void AdjustWindowSize(HWND hWnd, LPCRECT prect);

static HINSTANCE g_hInstance = NULL;
static WCHAR g_szTitle[128];
static HWND g_hHandleWnd = NULL;

static SETTINGS g_settings;

static const TCHAR g_szCaptHereWindowClass[] = _T("CaptHereWindowClass");

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadString(hInstance, IDS_APP_TITLE, g_szTitle, _countof(g_szTitle));
    MyRegisterClass(hInstance);
    RegisterHandleWindowClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CAPTHERE));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CAPTHERE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CAPTHERE);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = g_szCaptHereWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int /*nCmdShow*/)
{
    g_hInstance = hInstance;
    g_settings.snap = 20;

    GetRegistryString(_T(REGVALUE_SAVEFOLDER), g_settings.szSaveFolder, _countof(g_settings.szSaveFolder));

    // 現在のマウスカーソル位置から表示
    int x = 100;
    int y = 100;
    CURSORINFO  info;
    info.cbSize = sizeof(info);
    BOOL succeeded = GetCursorInfo(&info);
    CheckLastError(succeeded);
    if (succeeded) {
        x = info.ptScreenPos.x;
        y = info.ptScreenPos.y;
    }

    DWORD dwExStyle = WS_EX_LAYERED;
    DWORD dwStyle = WS_POPUP | WS_VISIBLE;
    HWND hWnd = CreateWindowEx(dwExStyle, g_szCaptHereWindowClass, g_szTitle, dwStyle,
        x, y, 400, 200, nullptr, nullptr, hInstance, nullptr);

    succeeded = (hWnd != NULL);
    if (succeeded) {
        ShowWindow(hWnd, SW_SHOW);
        UpdateWindow(hWnd);
    }

    return succeeded;
}

LRESULT OnCreate(HWND hWnd, LPCREATESTRUCT /*pcs*/)
{
    SetLayeredWindowAttributes(hWnd, 0, ALPHA_VALUE, LWA_ALPHA);

    RECT rect;
    GetWindowRect(hWnd, &rect);

    // 全画面
    g_hHandleWnd = CreateHandleWindow(g_hInstance, hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SW_SHOW);
    return 0L;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0L;
    switch (message)
    {
    case WM_CREATE:
        lResult = OnCreate(hWnd, (LPCREATESTRUCT)lParam);
        break;

    case WM_NCHITTEST:
        lResult = OnNcHittest(hWnd, lParam);
        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_CAPTHEREAPP:
        lResult = OnApp(hWnd, wParam, lParam);
        break;

    case WM_NCRBUTTONDOWN:
        lResult = OnNcRButtonDown(hWnd, wParam, lParam);
        break;

    case WM_MOVE:
        lResult = OnMove(hWnd, wParam, lParam);
        break;

    case WM_SIZE:
        lResult = OnSize(hWnd, wParam, lParam);
        break;

    case WM_ACTIVATE:
        lResult = OnActivate(hWnd, wParam, lParam);
        break;

    case WM_KEYDOWN:
        lResult = OnKeyDown(hWnd, wParam, lParam);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        lResult = DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return lResult;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

static LRESULT OnNcHittestEx(HWND hWnd, LPARAM lParam)
{
    LRESULT defValue = DefWindowProc(hWnd, WM_NCHITTEST, 0, lParam);
    if (defValue != HTCLIENT) {
        return defValue;
    }

    POINT pt = { /*x*/LOWORD(lParam), /*y*/HIWORD(lParam) };
    ScreenToClient(hWnd, &pt);

    RECT rect;
    GetClientRect(hWnd, &rect);

    if (pt.x <= BORDER_WIDTH) {
        if (pt.y <= BORDER_WIDTH) {
            return HTTOPLEFT;
        }
        if (pt.y >= (rect.bottom - BORDER_WIDTH)) {
            return HTBOTTOMLEFT;
        }
        return HTLEFT;
    }
    else if (pt.x >= (rect.right - BORDER_WIDTH)) {
        if (pt.y <= BORDER_WIDTH) {
            return HTTOPRIGHT;
        }
        if (pt.y >= (rect.bottom - BORDER_WIDTH)) {
            return HTBOTTOMRIGHT;
        }
        return HTRIGHT;
    }
    else {
        if (pt.y <= BORDER_WIDTH) {
            return HTTOP;
        }
        if (pt.y >= rect.bottom - BORDER_WIDTH) {
            return HTBOTTOM;
        }
    }

    return HTCAPTION;
}

LRESULT OnNcHittest(HWND hWnd, LPARAM lParam)
{
    LRESULT lResult = OnNcHittestEx(hWnd, lParam);
    return lResult;
}

static LRESULT OnMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = DefWindowProc(hWnd, WM_MOVING, wParam, lParam);
    RECT rect;
    GetWindowRect(hWnd, &rect);
    ATLTRACE(_T("%s(): x,y=(%d,%d)\n"), _T(__FUNCTION__), rect.left, rect.top);
    SetWindowPos(g_hHandleWnd, NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    return lResult;
}

static LRESULT OnApp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (wParam == SUBCOMMAND_ADJUST) {
        LPCRECT prect = (LPCRECT)lParam;
        AdjustWindowSize(hWnd, prect);
    }
    else if (wParam == SUBCOMMAND_CAPTURE) {
        Capture(hWnd);
    }

    return 0;
}

static void AdjustWindowSize(HWND hWnd, LPCRECT prect)
{
    int cx = prect->right - prect->left;
    int cy = prect->bottom - prect->top;
    SetWindowPos(hWnd, NULL, prect->left, prect->top, cx, cy, SWP_NOZORDER);
    SetWindowPos(g_hHandleWnd, NULL, prect->left, prect->top, cx, cy, SWP_NOZORDER);
}

static LRESULT OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    if (wParam == SIZE_RESTORED) {
        RECT rect;
        GetWindowRect(hWnd, &rect);
        SetWindowPos(g_hHandleWnd, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
    }
    else {
        lResult = DefWindowProc(hWnd, WM_SIZE, wParam, lParam);
    }

    return lResult;
}

static LRESULT OnNcRButtonDown(HWND hWnd, WPARAM /*wParam*/, LPARAM lParam)
{
    POINTS& pt = (POINTS&)lParam;
    ATLTRACE(_T("x,y=(%d, %d)\n"), pt.x, pt.y);

    SETTINGS settings = g_settings;
    settings.x = pt.x;
    settings.y = pt.y;
    GetWindowRect(hWnd, &settings.wndRect);

    int ret = (int)ShowSettingsDialog(g_hInstance, hWnd, &settings);
    if (ret == IDOK) {
        g_settings = settings;
        PutRegistryString(_T(REGVALUE_SAVEFOLDER), g_settings.szSaveFolder);
    }

    return 0;
}

static LRESULT OnActivate(HWND /*hWnd*/, WPARAM wParam, LPARAM /*lParam*/)
{
    ATLTRACE2(_T("OnActivate: wParam=%d\n"), wParam);
    SendMessage(g_hHandleWnd, WM_CAPTHEREAPP, SUBCOMMAND_ACTIVATE, wParam);

    return 0L;
}

static LRESULT OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    if (wParam == VK_ESCAPE) {
        PostMessage(hWnd, WM_CLOSE, 0, 0);
    }
    else if (wParam == VK_CONTROL) {
        PostMessage(hWnd, WM_CAPTHEREAPP, SUBCOMMAND_CAPTURE, 0);
    }
    else {
        lResult = DefWindowProc(hWnd, WM_KEYDOWN, wParam, lParam);
    }

    return lResult;
}

void Capture(HWND hWnd)
{
    RECT rect;
    GetWindowRect(hWnd, &rect);
    TCHAR szFileName[MAX_PATH];
    MakeFileName(szFileName);

    CaptureEx(szFileName, &rect);
}

void CaptureEx(LPCTSTR pszFileName, LPCRECT prect)
{
    const int bitCount = 16;

    HANDLE hFile = CreateFile(pszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD dwError = GetLastError();
        ATLTRACE2(_T("#%d: error=%ld\n"), __LINE__, dwError);
        return;
    }

    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    int bmpWidth = prect->right - prect->left;
    int bmpHeight = prect->bottom - prect->top;

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    fileHeader.bfType = 'B' | ('M' << 8);   // 0x4d42
    fileHeader.bfSize = 0;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    infoHeader.biSize = sizeof(infoHeader);
    infoHeader.biWidth = bmpWidth;
    infoHeader.biHeight = -bmpHeight;   // top-down DIB
    infoHeader.biPlanes = 1;

    // 16bit per pixel
    infoHeader.biBitCount = bitCount;
    infoHeader.biCompression = BI_RGB;
    infoHeader.biSizeImage = 0;
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    DWORD   dwWritten;
    WriteFile(hFile, &fileHeader, sizeof(fileHeader), &dwWritten, NULL);
    WriteFile(hFile, &infoHeader, sizeof(infoHeader), &dwWritten, NULL);

    BITMAPINFO info;
    info.bmiHeader = infoHeader;

    BOOL succeeded;
    BYTE* pmemory = NULL;
    HBITMAP hBitmap = CreateDIBSection(hScreenDC, &info, DIB_RGB_COLORS, (void**)&pmemory, 0, 0);
    SelectObject(hMemoryDC, hBitmap);

    succeeded = BitBlt(hMemoryDC, 0, 0, bmpWidth, bmpHeight, hScreenDC, prect->left, prect->top, SRCCOPY);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    DWORD dwBytesToWrite = (((bitCount * bmpWidth + 31) & (~31)) / 8) * bmpHeight;
    WriteFile(hFile, pmemory, dwBytesToWrite, &dwWritten, NULL);

    DeleteObject(hBitmap);
    CloseHandle(hFile);
}

static void MakeFileName(LPTSTR pszFileName)
{
    SYSTEMTIME  st;
    GetLocalTime(&st);

    _tcscpy_s(pszFileName, MAX_PATH,
        (g_settings.szSaveFolder[0] == _T('\0')) ? _T(".") : g_settings.szSaveFolder);

    TCHAR   szTemp[MAX_PATH];
    _sntprintf_s(szTemp, _countof(szTemp), _TRUNCATE, _T("capt_%04d%02d%02d_%02d%02d%02d.bmp"),
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    PathAppend(pszFileName, szTemp);
}

static BOOL OpenRegistryKey(PHKEY phKey) {

    LSTATUS status = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\CaptHere"), 0, KEY_ALL_ACCESS, phKey);
    return (status == ERROR_SUCCESS);
}

static void CloseRegistryKey(HKEY hKey)
{
    RegCloseKey(hKey);
}

BOOL GetRegistryString(LPCTSTR pszValueName, LPTSTR pszValue, size_t nValueLen)
{
    BOOL succeeded = FALSE;
    HKEY hKey;
    succeeded = OpenRegistryKey(&hKey);
    if (succeeded) {
        DWORD   cbData = (DWORD)(nValueLen / sizeof(TCHAR));
        DWORD   dwType;
        LSTATUS status = RegQueryValueEx(hKey, pszValueName, NULL, &dwType, (LPBYTE)pszValue, &cbData);
        if (status == ERROR_SUCCESS) {
            succeeded = TRUE;
        }

        CloseRegistryKey(hKey);
    }

    if (!succeeded) {
        *pszValue = _T('\0');
    }

    return succeeded;
}

BOOL PutRegistryString(LPCTSTR pszValueName, LPCTSTR pszValue)
{
    BOOL succeeded = FALSE;
    HKEY hKey;
    succeeded = OpenRegistryKey(&hKey);
    if (succeeded) {
        DWORD cbData = (DWORD)((_tcslen(pszValue) + 1) * sizeof(TCHAR));
        LSTATUS status = RegSetValueEx(hKey, pszValueName, NULL, REG_SZ, (CONST BYTE*)pszValue, cbData);
        if (status == ERROR_SUCCESS) {
            succeeded = TRUE;
        }

        CloseRegistryKey(hKey);
    }
    return succeeded;
}

void CheckLastError(BOOL succeeded)
{
    if (!succeeded) {
        DWORD dwError = GetLastError();
        ATLTRACE(_T("GetLastError(): error=(%lu, 0x%X)\n"), dwError, dwError);
    }
}
