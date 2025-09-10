#include "CaptureWindow.h"
#include <string>

HWND hwndCapture = NULL;
bool captureVisible = false;
static int captureScrollPos = 0;
static int captureTotalHeight = 0;
static const int MAX_VISIBLE_LINES = 15;
static const int LINE_HEIGHT = 20;  


LRESULT CALLBACK CaptureWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static std::string fullCaptureText;

    switch (uMsg) {
    case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);

            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
            HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
            FillRect(hdcMem, &clientRect, CreateSolidBrush(RGB(30, 30, 30)));
            RECT drawRect = clientRect;
            drawRect.top -= captureScrollPos;

            SetBkMode(hdcMem, TRANSPARENT);
            SetTextColor(hdcMem, RGB(255, 255, 255));

            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);

            DrawTextA(hdcMem, fullCaptureText.c_str(), -1, &drawRect,
                      DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);

            BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcMem, 0, 0, SRCCOPY);

            SelectObject(hdcMem, hOldFont);
            SelectObject(hdcMem, hOldBitmap);
            DeleteObject(hBitmap);
            DeleteDC(hdcMem);
            DeleteObject(hFont);

            EndPaint(hwnd, &ps);
            break;
    }


    case WM_COPYDATA: {
            COPYDATASTRUCT* cds = reinterpret_cast<COPYDATASTRUCT*>(lParam);
            if (cds && cds->lpData) {
                std::string incoming(static_cast<char*>(cds->lpData));
                if (incoming == "CLEAR_CAPTURE") {
                    fullCaptureText.clear();
                    captureScrollPos = 0;
                    SetScrollPos(hwnd, SB_VERT, 0, TRUE);
                } else {
                    fullCaptureText += incoming;
                }

                int lineCount = 0;
                for (char c : fullCaptureText) {
                    if (c == '\n') ++lineCount;
                }

                const int BOTTOM_PADDING = 10;
                captureTotalHeight = (lineCount + 1) * LINE_HEIGHT + BOTTOM_PADDING;

                RECT rcClient;
                GetClientRect(hwnd, &rcClient);
                int visibleHeight = rcClient.bottom;

                SCROLLINFO si = { sizeof(si), SIF_RANGE | SIF_PAGE | SIF_POS };
                si.nMin = 0;
                si.nMax = captureTotalHeight - 1;
                si.nPage = visibleHeight;
                si.nPos = captureScrollPos;
                SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

               
                int linesToShow = min(lineCount + 1, MAX_VISIBLE_LINES);
                int newHeight = linesToShow * LINE_HEIGHT + 10;

                RECT helpRect;
                GetWindowRect(GetParent(hwnd), &helpRect);  
                MoveWindow(hwnd, helpRect.left, helpRect.bottom + 10, helpRect.right - helpRect.left, newHeight, TRUE);

                captureScrollPos = max(0, captureTotalHeight - newHeight);
                SetScrollPos(hwnd, SB_VERT, captureScrollPos, TRUE);

                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
    }
    case WM_VSCROLL: {
            SCROLLINFO si = { sizeof(si), SIF_ALL };
            GetScrollInfo(hwnd, SB_VERT, &si);
            int pos = si.nPos;

            switch (LOWORD(wParam)) {
            case SB_LINEUP:     pos -= LINE_HEIGHT; break;
            case SB_LINEDOWN:   pos += LINE_HEIGHT; break;
            case SB_PAGEUP:     pos -= si.nPage; break;
            case SB_PAGEDOWN:   pos += si.nPage; break;
            case SB_THUMBTRACK: pos = HIWORD(wParam); break;
            }

            pos = max(si.nMin, min(si.nMax, pos));
            captureScrollPos = pos;

            si.fMask = SIF_POS;
            si.nPos = pos;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
            break;
    }



        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
}


void ToggleCaptureWindow(HWND helpAnchor) {
    const int NORMAL_ALPHA = 255;

    RECT helpRect;
    GetWindowRect(helpAnchor, &helpRect);

    int width  = helpRect.right - helpRect.left;
    int height = 200;
    int x      = helpRect.left;
    int y      = helpRect.bottom + 10;

    if (!hwndCapture) {
        WNDCLASSA wc = {};
        wc.lpfnWndProc = CaptureWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "CaptureOverlayClass";
        wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));
        UnregisterClassA("CaptureOverlayClass", GetModuleHandle(NULL));
        RegisterClassA(&wc);

        hwndCapture = CreateWindowExA(
    WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
    "CaptureOverlayClass", NULL, WS_POPUP | WS_VSCROLL,
    x, y, width, height,
    NULL, NULL, GetModuleHandle(NULL), NULL
);


        SetLayeredWindowAttributes(hwndCapture, 0, 255, LWA_ALPHA);
        SetWindowDisplayAffinity(hwndCapture, WDA_EXCLUDEFROMCAPTURE);



    } else {
        MoveWindow(hwndCapture, x, y, width, height, TRUE);
    }

    captureVisible = !captureVisible;
    ShowWindow(hwndCapture, captureVisible ? SW_SHOW : SW_HIDE);
    UpdateWindow(hwndCapture);
}


