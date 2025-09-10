#include "TranscriptWindow.h"
#include <string>

HWND hwndTranscript = NULL;
HWND hwndTranscriptEdit = NULL;
bool transcriptVisible = false;
// extern std::string lastQtResponse;
extern std::string transcriptResponse;


static HFONT hFont = NULL;
static HBRUSH hBackgroundBrush = NULL;

LRESULT CALLBACK TranscriptWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        if (!hFont) {
            hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        }

        if (!hBackgroundBrush) {
            hBackgroundBrush = CreateSolidBrush(RGB(20, 20, 20));
        }

        hwndTranscriptEdit = CreateWindowExA(
            0, "EDIT", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL |
            ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            0, 0, 400, 200,
            hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        SendMessage(hwndTranscriptEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        InvalidateRect(hwndTranscriptEdit, NULL, TRUE);
        break;
    }

    case WM_COPYDATA: {
            COPYDATASTRUCT* cds = reinterpret_cast<COPYDATASTRUCT*>(lParam);
            if (cds && cds->lpData && hwndTranscriptEdit) {
                std::string incoming(static_cast<char*>(cds->lpData));
                incoming += "\r\n";

                SendMessageA(hwndTranscriptEdit, EM_SETSEL, -1, -1);
                SendMessageA(hwndTranscriptEdit, EM_REPLACESEL, TRUE, (LPARAM)incoming.c_str());
                SendMessageA(hwndTranscriptEdit, EM_SCROLLCARET, 0, 0);
            }
            break;
    }


    case WM_SIZE:
        if (hwndTranscriptEdit) {
            MoveWindow(hwndTranscriptEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        }
        break;

    case WM_CTLCOLOREDIT: {
        HDC hdcEdit = (HDC)wParam;
        SetBkColor(hdcEdit, RGB(20, 20, 20));
        SetTextColor(hdcEdit, RGB(255, 255, 255));
        return (INT_PTR)hBackgroundBrush;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetBkColor(hdcStatic, RGB(20, 20, 20));
        SetTextColor(hdcStatic, RGB(255, 255, 255));
        return (INT_PTR)hBackgroundBrush;
    }

    case WM_DESTROY:
        hwndTranscriptEdit = NULL;
        break;
    }

    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void ToggleTranscriptWindow(HWND anchor) {
    const int NORMAL_ALPHA = 255;

    if (!hwndTranscript) {
        WNDCLASSA wc = {};
        wc.lpfnWndProc = TranscriptWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "TranscriptWindowClass";
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        UnregisterClassA("TranscriptWindowClass", GetModuleHandle(NULL));
        RegisterClassA(&wc);

        RECT rect;
        GetWindowRect(anchor, &rect);

        int width = 400;
        int height = 200;
        int x = rect.left;
        int y = rect.bottom + 5;

        hwndTranscript = CreateWindowExA(
            WS_EX_LAYERED | WS_EX_TOPMOST |WS_EX_TOOLWINDOW,
            "TranscriptWindowClass", NULL,
            WS_POPUP | WS_VISIBLE,
            x, y, width, height,
            NULL, NULL, GetModuleHandle(NULL), NULL
        );

        SetLayeredWindowAttributes(hwndTranscript, 0, 255, LWA_ALPHA);
        SetWindowDisplayAffinity(hwndTranscript, WDA_EXCLUDEFROMCAPTURE);

    }

    transcriptVisible = !transcriptVisible;

    ShowWindow(hwndTranscript, transcriptVisible ? SW_SHOW : SW_HIDE);
    UpdateWindow(hwndTranscript);
}

HWND GetTranscriptWindowHandle() {
    return hwndTranscript;
}
