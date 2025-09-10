# TransparentOverlay - AI Interview Assistant Frontend

A Windows-based transparent overlay application that serves as the **frontend client** for the [FloatingNavbar AI Interview Assistant](https://github.com/ddsha441981/FloatingNavbar). This application provides an intuitive floating interface for screen capture, AI-powered help assistance, and real-time transcription capabilities.

## 🏗️ Architecture Overview

**TransparentOverlay** (this repository) acts as the **frontend client** that communicates with the **backend server**:

- **Frontend**: TransparentOverlay (C++ Win32 Application) - *This Repository*
- **Backend**: [FloatingNavbar](https://github.com/ddsha441981/FloatingNavbar) (Qt6 + C++20 Application)
- **Communication**: Named Pipes for real-time data exchange

```
┌─────────────────────┐    Named Pipes    ┌──────────────────────┐
│  TransparentOverlay │ ◄─────────────► │   FloatingNavbar     │
│    (Frontend UI)    │                  │  (Backend + AI Core) │
│                     │                  │                      │
│ • User Interface    │                  │ • Groq API           │
│ • Screen Capture    │                  │ • Audio Processing   │
│ • Window Management │                  │ • AI Integration     │
│ • User Interaction  │                  │ • Question Analysis  │
└─────────────────────┘                  └──────────────────────┘
```

## ✨ Features

### 🎯 **Frontend UI Components**
- **Transparent Overlay**: Always-on-top floating toolbar that doesn't interfere with other applications
- **Screen Capture Exclusion**: Hidden from screen sharing and recording applications
- **Draggable Interface**: Move the overlay anywhere on your screen
- **Auto-fade**: Automatically reduces opacity after inactivity

### 🪟 **Multiple Window Interface**
- **Help Window**: Displays AI responses with syntax highlighting and scrolling
- **Capture Window**: Shows captured screen content and analysis
- **Transcript Window**: Real-time transcription display with editing capabilities
- **More Menu**: Additional controls and settings popup

### 🔧 **Advanced Frontend Features**
- **Named Pipe Communication**: Real-time communication with FloatingNavbar backend
- **Multi-threaded Streaming**: Separate threads for help, capture, and transcript data
- **Scrollable Content**: All text windows support vertical scrolling and mouse wheel
- **Memory Management**: Efficient handling of large text content with proper cleanup

## 📋 Prerequisites

### System Requirements
- **Windows 10/11** (required for Windows API features)
- **CMake 3.15+**
- **Visual Studio 2019+** or compatible C++ compiler with C++17 support

### Backend Dependency
- **[FloatingNavbar Backend](https://github.com/ddsha441981/FloatingNavbar)** - Must be running for full functionality

## 🚀 Installation

### 1. Setup Backend First
Before running TransparentOverlay, ensure the backend is properly set up:

```bash
# Clone and setup the backend
git clone https://github.com/ddsha441981/FloatingNavbar.git
cd FloatingNavbar
# Follow the backend setup instructions in that repository
```

### 2. Build Frontend (TransparentOverlay)

```bash
# Clone this repository
git clone <this-repository-url>
cd TransparentOverlay

# Create build directory
mkdir build
cd build

# Generate build files
cmake ..

# Build the application
cmake --build . --config Release

# Run the executable
./Release/TransparentOverlay.exe
```

### Dependencies
The application automatically links with required Windows libraries:
- `Dwmapi.lib` - Desktop Window Manager API
- `Shcore.lib` - Shell Core API  
- `Uxtheme.lib` - Visual Styles API

## 🎮 Usage

### Startup Sequence
1. **Start Backend First**: Launch the FloatingNavbar application
2. **Start Frontend**: Run TransparentOverlay.exe
3. **Verify Connection**: UI buttons should respond when clicked

### Main Toolbar Buttons

| Button | Function | Backend Pipe | Description |
|--------|----------|--------------|-------------|
| **Capture** | Screen Analysis | `FloatingNavbarPipeCapture` | Captures and analyzes screen content |
| **Ask/Help** | AI Assistant | `FloatingNavbarPipeHelp` | Opens help window with AI responses |
| **Start/Stop** | Recording Control | `FloatingNavbarPipeCommand` | Controls recording functionality |
| **Show/Hide** | Visibility Toggle | `FloatingNavbarPipeCommand` | Shows/hides interface elements |
| **More** | Additional Options | - | Opens popup with extra controls |

### Additional Controls (More Menu)

| Button | Function | Description |
|--------|----------|-------------|
| **Prompt** | Custom Prompts | Send custom prompts to AI backend |
| **Transcript** | Real-time Text | View live transcription from backend |
| **Models** | AI Model Selection | Choose different AI models (via backend) |
| **Speakers** | Audio Settings | Configure speaker options (via backend) |
| **Reset** | Reset Session | Clear all data and restart backend session |
| **Contact** | Support | Contact information |

### Keyboard & Mouse Controls

- **Left Click + Drag**: Move the overlay window
- **Mouse Wheel**: Scroll in help/capture/transcript windows
- **Auto-fade**: Interface becomes transparent after 30 seconds of inactivity

## 🏗️ Architecture Details

### Core Components

```
TransparentOverlay/
├── main.cpp              # Main application entry point
├── ButtonIDs.h           # Button identifier constants
├── CaptureWindow.cpp/.h  # Screen capture display window
├── HelpWindow.cpp/.h     # AI help response window
├── MoreWindow.cpp/.h     # Additional controls popup
├── TranscriptWindow.cpp/.h # Real-time transcription window
├── CommandDispatcher.cpp/.h # Named pipe communication manager
└── CMakeLists.txt        # Build configuration
```

### Communication Flow

```
TransparentOverlay (Frontend) ←→ Named Pipes ←→ FloatingNavbar (Backend)
                               │
                               ├── FloatingNavbarPipeCommand
                               ├── FloatingNavbarPipeHelp
                               ├── FloatingNavbarPipeCapture
                               └── FloatingNavbarPipeTranscript
```

### Threading Model

- **Main Thread**: UI rendering and user interaction
- **Help Stream Thread**: Real-time AI response streaming from backend
- **Capture Stream Thread**: Screen capture data processing from backend
- **Transcript Stream Thread**: Live transcription handling from backend

## ⚙️ Configuration

### Window Properties
- **Opacity**: Configurable fade levels (255 normal, 80 faded)
- **Position**: Centered horizontally, top of screen by default
- **Size**: 800x80 pixels main window
- **Exclusion**: Automatically excluded from screen capture

### Communication Settings
All named pipe communication uses the following pattern:
```cpp
\\.\pipe\FloatingNavbarPipe[Command|Help|Capture|Transcript]
```

**Note**: These pipes are created and managed by the FloatingNavbar backend application.

## 🛠️ Development

### Key Classes and Functions

**Main Window Management**
```cpp
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
void CreateOverlayButtons(HWND parent)
void UpdateFade()
```

**Named Pipe Communication**
```cpp
void SendCommandToQtApp(const char* message)
void StartHelpStreamThread(HWND hwndHelpWindow) 
void StartCaptureStreamThread(HWND hwndCaptureWindow)
```

**Window Controls**
```cpp
void ToggleHelpWindow(HWND anchor)
void ToggleCaptureWindow(HWND anchor) 
void ShowMoreMenuWindow(HWND hwndMain)
```

### Adding New Features

1. **New Buttons**: Add identifier to `ButtonIDs.h` and handler in `main.cpp`
2. **New Windows**: Create separate `.cpp/.h` files following existing patterns
3. **New Communication**: Extend `CommandDispatcher` with additional pipe handling
4. **Backend Integration**: Coordinate with [FloatingNavbar backend](https://github.com/ddsha441981/FloatingNavbar) for new features

## 🐛 Troubleshooting

### Common Issues

**Application won't start**
- Ensure FloatingNavbar backend is running first
- Check that all Windows API dependencies are available
- Verify named pipes are not blocked by antivirus

**Communication failures**
- Confirm FloatingNavbar backend is running and listening on named pipes
- Check Windows firewall settings
- Verify pipe names match between frontend and backend applications
- Try restarting both applications in sequence (backend first, then frontend)

**Display issues**
- Update graphics drivers
- Check Windows DPI scaling settings
- Ensure Desktop Window Manager is running

**UI not responding**
- Check backend console for pipe connection errors
- Verify backend API configuration (Groq API key, etc.)
- Restart backend application first, then frontend

### Debug Mode
Build with debug configuration to enable console output:
```cpp
OutputDebugStringA("[DEBUG] Message here\n");
```

## 📊 Performance

- **Memory**: ~2-5 MB typical usage (frontend only)
- **CPU**: <1% when idle, 2-5% during active streaming
- **Startup**: <500ms typical launch time
- **Responsiveness**: <50ms UI interaction response time
- **Backend Dependency**: Performance also depends on FloatingNavbar backend resource usage

## 🔗 Related Projects

- **Backend**: [FloatingNavbar](https://github.com/ddsha441981/FloatingNavbar) - Main AI processing and interview assistance logic
- **Frontend**: TransparentOverlay (this repository) - Windows UI client

## 📄 License

[Add your license information here]

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Ensure compatibility with [FloatingNavbar backend](https://github.com/ddsha441981/FloatingNavbar)
4. Test frontend-backend communication
5. Commit your changes (`git commit -m 'Add amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

## 📞 Support

For technical support or questions:
- **Frontend Issues**: Create an issue in this repository
- **Backend Issues**: Create an issue in [FloatingNavbar repository](https://github.com/ddsha441981/FloatingNavbar)
- **Integration Issues**: Mention both repositories in your issue

***

**⚠️ Important Note**: This frontend application requires the [FloatingNavbar backend](https://github.com/ddsha441981/FloatingNavbar) to be running for full functionality. The backend handles AI processing, screen capture analysis, audio processing, and transcription services. Make sure to set up and start the backend application before using TransparentOverlay.
