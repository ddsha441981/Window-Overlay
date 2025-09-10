#include "MoreWindow.h"
#include "ButtonIDs.h"

extern HWND hwndMain;

void RegisterMoreWindowClass(HINSTANCE hInstance) {
    WNDCLASSA moreClass = {};
    moreClass.lpfnWndProc = MoreWndProc;
    moreClass.hInstance = hInstance;
    moreClass.lpszClassName = "MorePopupWindowClass";
    moreClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    UnregisterClassA("MorePopupWindowClass", GetModuleHandle(NULL));
    RegisterClassA(&moreClass);
}

void ShowMoreMenuWindow(HWND hwndMain) {
    if (!hwndMorePopup) {
        RECT navRect;
        GetWindowRect(hwndMain, &navRect);

        int btnWidth = 140;
        int btnHeight = 40;
        int spacing = 10;

        int x1 = spacing;
        int x2 = x1 + btnWidth + spacing;
        int y1 = spacing;
        int y2 = y1 + btnHeight + spacing;
        int y3 = y2 + btnHeight + spacing;

        int popupWidth = 2 * btnWidth + 3 * spacing;
        int popupHeight = y3 + btnHeight + spacing;

        hwndMorePopup = CreateWindowExA(
            WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            "MorePopupWindowClass", NULL,
            WS_POPUP,
            navRect.right + 5, navRect.top,
            popupWidth, popupHeight,
            NULL, NULL, GetModuleHandle(NULL), NULL
        );

        SetLayeredWindowAttributes(hwndMorePopup, 0, 255, LWA_ALPHA);
        SetWindowDisplayAffinity(hwndMorePopup, WDA_EXCLUDEFROMCAPTURE);

        CreateWindowA("BUTTON", "Prompt",     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                      x1, y1, btnWidth, btnHeight, hwndMorePopup, (HMENU)BTN_PROMPT, GetModuleHandle(NULL), NULL);
        CreateWindowA("BUTTON", "Transcript", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                      x2, y1, btnWidth, btnHeight, hwndMorePopup, (HMENU)BTN_TRANSCRIPT, GetModuleHandle(NULL), NULL);
        CreateWindowA("BUTTON", "Models",     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                      x1, y2, btnWidth, btnHeight, hwndMorePopup, (HMENU)BTN_MODELS, GetModuleHandle(NULL), NULL);
        CreateWindowA("BUTTON", "Speakers",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                      x2, y2, btnWidth, btnHeight, hwndMorePopup, (HMENU)BTN_SPEAKER, GetModuleHandle(NULL), NULL);
        CreateWindowA("BUTTON", "Reset",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                      x1, y3, btnWidth, btnHeight, hwndMorePopup, (HMENU)BTN_RESET, GetModuleHandle(NULL), NULL);
        CreateWindowA("BUTTON", "Contact",    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                      x2, y3, btnWidth, btnHeight, hwndMorePopup, (HMENU)BTN_CONTACT, GetModuleHandle(NULL), NULL);
    }

    ShowWindow(hwndMorePopup, SW_SHOW);
    SetWindowPos(hwndMorePopup, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

LRESULT CALLBACK MoreWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        HBRUSH background = CreateSolidBrush(RGB(20, 20, 20));
        FillRect(hdc, &ps.rcPaint, background);
        DeleteObject(background);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_KILLFOCUS: {
        HWND newFocus = (HWND)wParam;
        if (!IsChild(hwnd, newFocus)) {
            ShowWindow(hwnd, SW_HIDE);
        }
        break;
    }

    case WM_COMMAND:

        SendMessage(hwndMain, WM_COMMAND, wParam, lParam);
        break;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}
