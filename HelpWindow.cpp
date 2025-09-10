#include "HelpWindow.h"
#include <string>
#include <windowsx.h> 

HWND hwndHelp = NULL;
bool helpVisible = false;

LRESULT CALLBACK HelpWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static std::string fullResponseText;
    static int scrollPos = 0;
    static int totalContentHeight = 0;

    switch (uMsg) {
    case WM_CREATE:
        SetScrollRange(hwnd, SB_VERT, 0, 0, FALSE);
        SetScrollPos(hwnd, SB_VERT, 0, TRUE);
        break;

    case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdcWindow = BeginPaint(hwnd, &ps);
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);

            HDC hdcMem = CreateCompatibleDC(hdcWindow);
            HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, clientRect.right, clientRect.bottom);
            HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
            FillRect(hdcMem, &clientRect, CreateSolidBrush(RGB(20, 20, 20)));  
            
            RECT drawRect = clientRect;
            drawRect.top -= scrollPos;

            SetBkMode(hdcMem, TRANSPARENT);
            SetTextColor(hdcMem, RGB(255, 255, 255));

            HFONT hFont = CreateFontA(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);

            DrawTextA(hdcMem, fullResponseText.c_str(), -1, &drawRect,
                      DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);

           
            BitBlt(hdcWindow, 0, 0, clientRect.right, clientRect.bottom, hdcMem, 0, 0, SRCCOPY);

            SelectObject(hdcMem, hOldFont);
            DeleteObject(hFont);
            SelectObject(hdcMem, hOldBitmap);
            DeleteObject(hBitmap);
            DeleteDC(hdcMem);
            EndPaint(hwnd, &ps);
            break;
    }


    case WM_COPYDATA: {
        COPYDATASTRUCT* cds = reinterpret_cast<COPYDATASTRUCT*>(lParam);
        if (cds && cds->lpData) {
            std::string chunk(static_cast<char*>(cds->lpData));
            if (chunk == "CLEAR_HELP") {
                fullResponseText.clear();
                scrollPos = 0;
                SetScrollPos(hwnd, SB_VERT, 0, TRUE);
            } else {
                fullResponseText += chunk;
            }

            HDC hdc = GetDC(hwnd);
            HFONT hFont = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            SelectObject(hdc, hFont);

            RECT measure = { 0, 0, 500, 0 };
            DrawTextA(hdc, fullResponseText.c_str(), -1, &measure,
                      DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
            totalContentHeight = measure.bottom;

            DeleteObject(hFont);
            ReleaseDC(hwnd, hdc);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int visibleHeight = clientRect.bottom;

            SCROLLINFO si = {};
            si.cbSize = sizeof(si);
            si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
            si.nMin = 0;
            si.nMax = totalContentHeight - 1;
            si.nPage = visibleHeight;
            si.nPos = scrollPos;

            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;
    }

    case WM_VSCROLL: {
        SCROLLINFO si = { sizeof(si), SIF_ALL };
        GetScrollInfo(hwnd, SB_VERT, &si);
        int oldPos = si.nPos;

        switch (LOWORD(wParam)) {
        case SB_LINEUP:     si.nPos -= 20; break;
        case SB_LINEDOWN:   si.nPos += 20; break;
        case SB_PAGEUP:     si.nPos -= si.nPage; break;
        case SB_PAGEDOWN:   si.nPos += si.nPage; break;
        case SB_THUMBTRACK: si.nPos = HIWORD(wParam); break;
        }

        si.fMask = SIF_POS;
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
        GetScrollInfo(hwnd, SB_VERT, &si);
        scrollPos = si.nPos;
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    }

    case WM_MOUSEWHEEL: {
        short delta = GET_WHEEL_DELTA_WPARAM(wParam);
        int lines = -delta / WHEEL_DELTA * 40;

        SCROLLINFO si = { sizeof(si), SIF_ALL };
        GetScrollInfo(hwnd, SB_VERT, &si);

        int newPos = si.nPos + lines;
        newPos = max(si.nMin, min(si.nMax - (int)si.nPage + 1, newPos));

        si.fMask = SIF_POS;
        si.nPos = newPos;
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
        scrollPos = newPos;
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    }

    case WM_USER + 2:
        fullResponseText.clear();
        scrollPos = 0;
        SetScrollPos(hwnd, SB_VERT, 0, TRUE);
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    }

    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void ShowHelpWindow(HWND anchor) {
    const int NORMAL_ALPHA = 255;
    if (!hwndHelp) {
        WNDCLASSA helpClass = {};
        helpClass.lpfnWndProc = HelpWndProc;
        helpClass.hInstance = GetModuleHandle(NULL);
        helpClass.lpszClassName = "HelpOverlayClass";
        helpClass.hbrBackground = CreateSolidBrush(RGB(20, 20, 20));
        UnregisterClassA("HelpOverlayClass", GetModuleHandle(NULL));
        RegisterClassA(&helpClass);

        RECT rect;
        GetWindowRect(anchor, &rect);

        int helpHeight = 400;
        int helpWidth  = rect.right - rect.left;

        hwndHelp = CreateWindowExA(
            WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            "HelpOverlayClass", NULL, WS_POPUP | WS_VSCROLL,
            rect.left, rect.bottom + 10,
            helpWidth, helpHeight,
            NULL, NULL, GetModuleHandle(NULL), NULL
        );

        SetLayeredWindowAttributes(hwndHelp, 0, 255, LWA_ALPHA);
        SetWindowDisplayAffinity(hwndHelp,WDA_EXCLUDEFROMCAPTURE);

    }

    if (!IsWindowVisible(hwndHelp)) {
        ShowWindow(hwndHelp, SW_SHOW);
        UpdateWindow(hwndHelp);
        helpVisible = true;
    }
}

