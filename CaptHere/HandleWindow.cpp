// HandleWindow.cpp
//

#include "stdafx.h"
#include "CaptHere.h"

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT OnApp(HWND hWnd, WPARAM wParam, LPARAM lParam);

static const TCHAR g_szWindowClass[] = _T("CaptHere_HandleWindowClass");

#define TRANSPARENT_COLORREF    RGB(255,0,0)

static HBRUSH g_hbrBackground = NULL;

ATOM RegisterHandleWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    g_hbrBackground = CreateSolidBrush(TRANSPARENT_COLORREF);

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = NULL;
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    //wcex.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
    wcex.hbrBackground = g_hbrBackground;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = g_szWindowClass;
    wcex.hIconSm = NULL;

    return RegisterClassExW(&wcex);
}

HWND CreateHandleWindow(HINSTANCE hInstance, HWND hWndParent, int x, int y, int width, int height, int nCmdShow)
{
    // WS_EX_TRANSPARENT で親を透過する
    DWORD dwExStyle = WS_EX_LAYERED | WS_EX_TRANSPARENT;
    //DWORD dwStyle = WS_POPUP | WS_VISIBLE;
    // WS_CHILDだとCreateできないみたい
    // DWORD dwStyle = WS_CHILD | WS_VISIBLE;
    DWORD dwStyle = WS_POPUP | WS_VISIBLE;
    HWND hWnd = CreateWindowEx(dwExStyle, g_szWindowClass, _T(""), dwStyle,
        x, y, width, height, hWndParent, nullptr, hInstance, nullptr);

    BOOL bResult = FALSE;
    if (hWnd != NULL) {
        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);
        bResult = TRUE;
    }

    return hWnd;
}

void DrawBorder(HWND hWnd, bool bActive)
{
    COLORREF col = bActive ? BORDERCOLOR_ACTIVE : BORDERCOLOR_INACTIVE;

    HDC hDC = GetWindowDC(hWnd);
    HPEN hNewPen = (HPEN)CreatePen(PS_INSIDEFRAME, BORDER_WIDTH, col);
    HPEN hOldPen = (HPEN)SelectObject(hDC, hNewPen);
    HBRUSH hNewBrush = (HBRUSH)CreateSolidBrush(TRANSPARENT_COLORREF);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hNewBrush);

    RECT rect;
    GetWindowRect(hWnd, &rect);
    Rectangle(hDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top);

    DeleteObject(SelectObject(hDC, hOldPen));
    DeleteObject(SelectObject(hDC, hOldBrush));

    GetClientRect(hWnd, &rect);

    int x = rect.right / 2;
    int y = rect.bottom / 2;

    hNewPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
    hOldPen = (HPEN)SelectObject(hDC, hNewPen);

    MoveToEx(hDC, x, 0 + BORDER_WIDTH, NULL);
    LineTo(hDC, x, rect.bottom - BORDER_WIDTH);
    MoveToEx(hDC, 0 + BORDER_WIDTH, y, NULL);
    LineTo(hDC, rect.right - BORDER_WIDTH, y);

    DeleteObject(SelectObject(hDC, hOldPen));
}

static LRESULT OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    BeginPaint(hWnd, &ps);

    DrawBorder(hWnd, true);

    EndPaint(hWnd, &ps);
    return 0;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    switch (message)
    {
    case WM_CREATE:
        SetLayeredWindowAttributes(hWnd, TRANSPARENT_COLORREF, 0, LWA_COLORKEY);
        break;

    case WM_SIZE:
        ATLTRACE2(_T("HnadleWindow:WM_SIZE\n"));
        lResult = DefWindowProc(hWnd, message, wParam, lParam);
        break;

    case WM_PAINT:
        lResult = OnPaint(hWnd);
        break;

    case WM_CAPTHEREAPP:
        lResult = OnApp(hWnd, wParam, lParam);
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

static LRESULT OnApp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (wParam) {
    case SUBCOMMAND_ACTIVATE:
        DrawBorder(hWnd, lParam != WA_INACTIVE);
        break;
    }

    return 0;
}

