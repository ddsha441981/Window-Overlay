#pragma once
#include <windows.h>

extern HWND hwndTranscript;
extern bool transcriptVisible;

void ToggleTranscriptWindow(HWND anchor);
LRESULT CALLBACK TranscriptWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND GetTranscriptWindowHandle();
