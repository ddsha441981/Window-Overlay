cmake_minimum_required(VERSION 3.15)
project(TransparentOverlay)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_WIN32_EXECUTABLE TRUE)

set(SOURCES
        main.cpp
        MoreWindow.cpp
        TranscriptWindow.cpp
        HelpWindow.cpp
        CommandDispatcher.cpp
        CaptureWindow.cpp
)

set(HEADERS
        MoreWindow.h
        ButtonIDs.h
        TranscriptWindow.h
        HelpWindow.h
        CommandDispatcher.h
        CaptureWindow.h
)

add_executable(TransparentOverlay ${SOURCES} ${HEADERS})

#  Win32 system libraries needed for DPI/DWM/Themes
target_link_libraries(TransparentOverlay Dwmapi Shcore Uxtheme)
