#include "CommandDispatcher.h"
#include "HelpWindow.h"
#include <windows.h>
#include <thread>
#include <atomic>

#include "CaptureWindow.h"
#include "TranscriptWindow.h"

extern HWND hwndMain;

std::string lastQtResponse = "No response";

std::atomic<bool> transcriptActive = false;
std::thread transcriptThread;
std::atomic<bool> helpStreamActive = false;
std::thread helpStreamThread;
std::atomic<bool> captureStreamActive = false;
std::thread captureStreamThread;


void StartTranscriptThread(HWND hwndTranscript) {
    if (transcriptActive) return;

    transcriptActive = true;
    transcriptThread = std::thread([hwndTranscript]() {
        HANDLE hPipe = CreateFileA(
            R"(\\.\pipe\FloatingNavbarPipeTranscript)",
            GENERIC_READ | GENERIC_WRITE,
            0, NULL, OPEN_EXISTING, 0, NULL);

        if (hPipe == INVALID_HANDLE_VALUE) {
            OutputDebugStringA("❌ Failed to open pipe\n");
            return;
        }

       
        const char* cmd = "Transcript";
        DWORD written;
        WriteFile(hPipe, cmd, strlen(cmd), &written, NULL);
        FlushFileBuffers(hPipe);

        char buffer[512];
        DWORD bytesRead;

        std::string pendingText;

        while (transcriptActive && ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
            if (bytesRead == 0) break;

            pendingText.append(buffer, bytesRead);

            size_t newlinePos;
            while ((newlinePos = pendingText.find('\n')) != std::string::npos) {
                std::string line = pendingText.substr(0, newlinePos + 1);
                pendingText.erase(0, newlinePos + 1);

                SendToTranscriptWindow(line);

            }
        }

        CloseHandle(hPipe);
        transcriptActive = false;
    });
}

void StopTranscriptThread() {
    transcriptActive = false;
    if (transcriptThread.joinable())
        transcriptThread.join();
}

void StartHelpStreamThread(HWND hwndHelpWindow) {
    if (helpStreamActive) return;

    helpStreamActive = true;
    helpStreamThread = std::thread([hwndHelpWindow]() {
        HANDLE hPipe = CreateFileA(
            R"(\\.\pipe\FloatingNavbarPipeHelp)",
            GENERIC_READ | GENERIC_WRITE,
            0, NULL, OPEN_EXISTING, 0, NULL);

        if (hPipe == INVALID_HANDLE_VALUE) {
            OutputDebugStringA("❌ Failed to open HELP pipe\n");
            return;
        }

        const char* cmd = "Ask/Help";
        DWORD written;
        WriteFile(hPipe, cmd, strlen(cmd), &written, NULL);
        FlushFileBuffers(hPipe);

        char buffer[512];
        DWORD bytesRead;
        std::string pendingText;

        while (helpStreamActive && ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
            if (bytesRead == 0) break;

            pendingText.append(buffer, bytesRead);

            size_t newlinePos;
            while ((newlinePos = pendingText.find('\n')) != std::string::npos) {
                std::string line = pendingText.substr(0, newlinePos + 1);
                pendingText.erase(0, newlinePos + 1);

                SendToHelpWindow(line);
            }
        }

        CloseHandle(hPipe);
        helpStreamActive = false;
    });
}

void StopHelpStreamThread() {
    helpStreamActive = false;
    if (helpStreamThread.joinable())
        helpStreamThread.join();
}

void SendToHelpWindow(const std::string& text) {
    if (!hwndHelp || !IsWindow(hwndHelp)) return;

    COPYDATASTRUCT cds;
    cds.dwData = 1;
    cds.cbData = text.size() + 1;
    cds.lpData = (void*)text.c_str();

    SendMessage(hwndHelp, WM_COPYDATA, 0, (LPARAM)&cds);
}

void SendToTranscriptWindow(const std::string& text) {
    if (!hwndTranscript || !IsWindow(hwndTranscript)) return;

    COPYDATASTRUCT cds;
    cds.dwData = 2;
    cds.cbData = text.size() + 1;
    cds.lpData = (void*)text.c_str();

    SendMessage(hwndTranscript, WM_COPYDATA, 0, (LPARAM)&cds);
}

void SendCommandToQtApp(const char* message) {
    HANDLE hPipe = CreateFileA(
        R"(\\.\pipe\FloatingNavbarPipeCommand)",
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL);

    if (hPipe != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hPipe, message, strlen(message), &written, NULL);

        std::string response;
        char buffer[512];
        DWORD read;

        while (ReadFile(hPipe, buffer, sizeof(buffer), &read, NULL) && read > 0) {
            response.append(buffer, read);
            if (read < sizeof(buffer)) break;
        }

        lastQtResponse = response.empty() ? "Failed to read response." : response;
        CloseHandle(hPipe);
    } else {
        lastQtResponse = "Failed to connect to Qt.";
    }

    if (!hwndHelp || !IsWindowVisible(hwndHelp)) {
        //ToggleHelpWindow(hwndMain); 
        ShowHelpWindow(hwndMain);

    } else {
        SendMessage(hwndHelp, WM_USER + 1, 0, 0);
    }
}

void StartCaptureStreamThread(HWND hwndCaptureWindow) {
    if (captureStreamActive) return;

    captureStreamActive = true;
    captureStreamThread = std::thread([hwndCaptureWindow]() {
        HANDLE hPipe = CreateFileA(
            R"(\\.\pipe\FloatingNavbarPipeCapture)",
            GENERIC_READ | GENERIC_WRITE,
            0, NULL, OPEN_EXISTING, 0, NULL);

        if (hPipe == INVALID_HANDLE_VALUE) {
            OutputDebugStringA("❌ Failed to open CAPTURE pipe\n");
            captureStreamActive = false;
            return;
        }

        const char* cmd = "Capture";
        DWORD written;
        WriteFile(hPipe, cmd, strlen(cmd), &written, NULL);
        FlushFileBuffers(hPipe);

        char buffer[512];
        DWORD bytesRead;
        std::string pending;

        while (captureStreamActive && ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
            if (bytesRead == 0) break;

            pending.append(buffer, bytesRead);

            size_t newline;
            while ((newline = pending.find('\n')) != std::string::npos) {
                std::string line = pending.substr(0, newline + 1);
                pending.erase(0, newline + 1);
                SendToCaptureWindow(line);
            }
        }

        CloseHandle(hPipe);
        captureStreamActive = false;
    });
}

void StopCaptureStreamThread() {
    captureStreamActive = false;
    if (captureStreamThread.joinable()) captureStreamThread.join();
}

void SendToCaptureWindow(const std::string& text) {
    if (!hwndCapture || !IsWindow(hwndCapture)) return;

    COPYDATASTRUCT cds;
    cds.dwData = 3;
    cds.cbData = text.size() + 1;
    cds.lpData = (void*)text.c_str();
    SendMessage(hwndCapture, WM_COPYDATA, 0, (LPARAM)&cds);
}

