#pragma once
#include <windows.h>

extern HWND hwndCapture;
extern bool captureVisible;

void ToggleCaptureWindow(HWND anchor);
LRESULT CALLBACK CaptureWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
