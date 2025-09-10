#pragma once
#include <windows.h>

extern HWND hwndHelp;
extern bool helpVisible;

void ToggleHelpWindow(HWND anchor);
void ShowHelpWindow(HWND anchor);
LRESULT CALLBACK HelpWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND GetHelpWindowHandle();
