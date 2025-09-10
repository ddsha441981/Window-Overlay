#pragma once
#include <string>

#include "HelpWindow.h"

extern std::string lastQtResponse;

void SendCommandToQtApp(const char* message);
void StartTranscriptThread(HWND hwndTranscript);
void StopTranscriptThread();
void startHelpResponse(const char* message);
void StartHelpStreamThread(HWND hwndHelpWindow);
void StopHelpStreamThread();
void SendToHelpWindow(const std::string& text);
void SendToTranscriptWindow(const std::string& text);
void StartCaptureStreamThread(HWND hwndCaptureWindow);
void StopCaptureStreamThread();
void SendToCaptureWindow(const std::string& text);




