#pragma once
#include <windows.h>

extern HWND hwndMorePopup;

void RegisterMoreWindowClass(HINSTANCE hInstance);
void ShowMoreMenuWindow(HWND hwndMain);
LRESULT CALLBACK MoreWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
