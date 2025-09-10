#include <windows.h>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include "MoreWindow.h"
#include "ButtonIDs.h"
#include "CaptureWindow.h"
#include "TranscriptWindow.h"
#include "HelpWindow.h"
#include "CommandDispatcher.h"



LPCSTR CLASS_NAME = "HiddenOverlayClass";
LPCSTR HELP_CLASS_NAME = "HelpOverlayClass";
#define MAX_VISIBLE_LINES 15
HWND hwndMorePopup = nullptr;


void ToggleHelpWindow();
HWND hwndMain;
std::vector<HWND> buttons;
bool dragging = false;
POINT offset;
std::chrono::steady_clock::time_point lastInteraction;
bool faded = false;
// bool helpVisible = false;

// const int FADE_TIMEOUT_MS = 3000;
// const int FADE_ALPHA = 80;
// const int NORMAL_ALPHA = 255;

const int FADE_TIMEOUT_MS = 30000; 
const int FADE_ALPHA = 255;       
const int NORMAL_ALPHA = 255;


void CreateOverlayButtons(HWND parent) {
    const int btnWidth = 100, btnHeight = 40;
    const int spacing = 10;

    struct ButtonInfo {
        const char* label;
        int id;
    };

    ButtonInfo buttonsToCreate[] = {
        { "Capture",     BTN_CAPTURE },
        { "Ask/Help",    BTN_ASK_HELP },
        { "Start/Stop",  BTN_START_STOP },
        { "Show/Hide",   BTN_SHOW_HIDE },
        { "More",        ID_BTN_MORE }
    };

    int count = sizeof(buttonsToCreate) / sizeof(ButtonInfo);
    int totalWidth = count * btnWidth + (count - 1) * spacing;
    int x = 0;

    for (int i = 0; i < count; ++i) {
        HWND btn = CreateWindowA("BUTTON", buttonsToCreate[i].label,
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            x, 0, btnWidth, btnHeight,
            parent, (HMENU)buttonsToCreate[i].id, GetModuleHandle(NULL), NULL);
        buttons.push_back(btn);
        x += btnWidth + spacing;
    }

    SetWindowPos(parent, NULL, 0, 0, totalWidth, btnHeight, SWP_NOMOVE | SWP_NOZORDER);
}

void UpdateFade() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInteraction).count();

    if (elapsed > FADE_TIMEOUT_MS && !faded) {
        SetLayeredWindowAttributes(hwndMain, 0, FADE_ALPHA, LWA_ALPHA);
        if (hwndHelp && helpVisible) {
            SetLayeredWindowAttributes(hwndHelp, 0, FADE_ALPHA, LWA_ALPHA);
        }
        faded = true;
    } else if (elapsed <= FADE_TIMEOUT_MS && faded) {
        SetLayeredWindowAttributes(hwndMain, 0, NORMAL_ALPHA, LWA_ALPHA);
        if (hwndHelp && helpVisible) {
            SetLayeredWindowAttributes(hwndHelp, 0, NORMAL_ALPHA, LWA_ALPHA);
        }
        faded = false;
    }
}

void ResetFadeTimer() {
    lastInteraction = std::chrono::steady_clock::now();
}

void OnClickPrompt(){
    SendCommandToQtApp("Prompt");
}
void OnClickCapture(){
    if (!captureVisible) {
        ToggleCaptureWindow(hwndHelp);
        SendToCaptureWindow("CLEAR_CAPTURE");
        StartCaptureStreamThread(hwndCapture);
    } else {
        StopCaptureStreamThread();
        ToggleCaptureWindow(hwndHelp);
    }
}


void OnClickAskHelp(){
    OutputDebugStringA("[AskHelp] Button clicked.\n");
    StopHelpStreamThread();
    SendToHelpWindow("CLEAR_HELP");
    StartHelpStreamThread(hwndHelp);
    // ToggleHelpWindow(hwndMain);
    ShowHelpWindow(hwndMain);
}
void OnClickStartStop(){
    SendCommandToQtApp("Start/Stop");
}
void OnClickShowHide()     { SendCommandToQtApp("Show/Hide"); }
void ToggleTranscriptWindow();

void OnClickTranscript(){
    if (!transcriptVisible) {
        ToggleTranscriptWindow(hwndMorePopup);  
        StartTranscriptThread(GetTranscriptWindowHandle());
    } else {
        StopTranscriptThread();
        ToggleTranscriptWindow(hwndMorePopup);  
    }
}
void moreMenuWindow();

void OnClickMore()
{
    ShowMoreMenuWindow(hwndMain);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        CreateOverlayButtons(hwnd);
        SetTimer(hwnd, 1, 100, NULL);
        ResetFadeTimer();
        break;

    case WM_TIMER:
        UpdateFade();
        break;

    case WM_LBUTTONDOWN:
        dragging = true;
        SetCapture(hwnd);
        offset.x = LOWORD(lParam);
        offset.y = HIWORD(lParam);
        ResetFadeTimer();
        break;

    case WM_LBUTTONUP:
        dragging = false;
        ReleaseCapture();
        break;

    case WM_MOUSEMOVE:
        if (dragging) {
            POINT pt;
            GetCursorPos(&pt);
            SetWindowPos(hwnd, NULL, pt.x - offset.x, pt.y - offset.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            if (hwndHelp && helpVisible) {
                RECT rect;
                GetWindowRect(hwnd, &rect);
                SetWindowPos(hwndHelp, NULL, rect.left, rect.bottom + 10, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            }
        }
        ResetFadeTimer();
        break;

    case WM_COMMAND:
        switch (wParam) {
    case BTN_PROMPT:       OnClickPrompt(); break;
    case BTN_CAPTURE:      OnClickCapture(); break;
    case BTN_ASK_HELP:     OnClickAskHelp(); break;
    case BTN_START_STOP:   OnClickStartStop(); break;
    case BTN_SHOW_HIDE:    OnClickShowHide(); break;
    case BTN_TRANSCRIPT:   OnClickTranscript(); break;
    case ID_BTN_MORE:         OnClickMore(); break;
        }
        ResetFadeTimer();
        break;


    case WM_SETCURSOR:
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));

        RECT rect;
        GetClientRect(hwnd, &rect);
        rect.top += 45;
        //DrawTextA(hdc, "Hidden from screen share", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        if (hwndHelp) DestroyWindow(hwndHelp);
        PostQuitMessage(0);
        break;
    }

    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {

//screen on top don't delete
     WNDCLASSA wc = {};
     wc.lpfnWndProc = WndProc;
     wc.hInstance = hInstance;
     wc.lpszClassName = CLASS_NAME;
     wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
     wc.hCursor = LoadCursorA(NULL, IDC_ARROW);

     RegisterClassA(&wc);

     int screenWidth = GetSystemMetrics(SM_CXSCREEN);
     int x = (screenWidth - 800) / 2;
     int y = 0;
    // More Window register
        RegisterMoreWindowClass(hInstance);
    hwndMain = CreateWindowExA(
    WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 
    CLASS_NAME,
    NULL,
    WS_POPUP,
    x, y, 800, 80, 
    NULL, NULL, hInstance, NULL
);

    SetLayeredWindowAttributes(hwndMain, 0, NORMAL_ALPHA, LWA_ALPHA);
    SetWindowDisplayAffinity(hwndMain, WDA_EXCLUDEFROMCAPTURE);

    ShowWindow(hwndMain, SW_SHOW);
    UpdateWindow(hwndMain);

    MSG msg = {};
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}


