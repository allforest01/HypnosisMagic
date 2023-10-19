#pragma once
#include "KeyMapping.h"
#include <functional>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define WINDOWS
    #include <windows.h>
    #include <opencv2/opencv.hpp>
#else
    #include <unistd.h>
    #include <stdio.h>
    #include <ApplicationServices/ApplicationServices.h>
    #include <CoreGraphics/CoreGraphics.h>
    #include <opencv2/opencv.hpp>
#endif

class EasyEvent {

private:
#ifdef WINDOWS
    HHOOK keyboardHook, mouseHook; MSG msg;
    LRESULT CALLBACK KeyboardHookCallback(int, WPARAM, LPARAM);
    static LRESULT CALLBACK GlobalKeyboardHookCallback(int, WPARAM, LPARAM);
    LRESULT CALLBACK MouseHookCallback(int, WPARAM, LPARAM);
    static LRESULT CALLBACK GlobalMouseHookCallback(int, WPARAM, LPARAM);
    void toScrCoor(int&, int&);
#else
    static CGEventRef MyCGEventCallback(CGEventTapProxy, CGEventType, CGEventRef, void*);
    CFMachPortRef eventTap;
    CFRunLoopSourceRef runLoopSource;
#endif

public:
    static EasyEvent& getInstance() {
        static EasyEvent instance;
        return instance;
    }

    EasyEvent();

    int screenx, screeny;
    int width, height;

    cv::Mat CaptureScreen();

    std::function<void(int)> KeyDownCallback;
    std::function<void(int)> KeyUpCallback;
    std::function<void(int, int)> LDownCallback;
    std::function<void(int, int)> LUpCallback;
    std::function<void(int, int)> RDownCallback;
    std::function<void(int, int)> RUpCallback;
    std::function<void(int, int)> MoveCallback;

    void StartHook();
    void MsgLoop();
    void Unhook();

    void SendKeyDown(int, int);
    void SendKeyUp(int, int);
    void SendLDown(int, int);
    void SendLUp(int, int);
    void SendRDown(int, int);
    void SendRUp(int, int);
    void SendMove(int, int);

    void setKeyDownCallback(std::function<void(int)>);
    void setKeyUpCallback(std::function<void(int)>);
    void setLDownCallback(std::function<void(int, int)>);
    void setLUpCallback(std::function<void(int, int)>);
    void setRDownCallback(std::function<void(int, int)>);
    void setRUpCallback(std::function<void(int, int)>);
    void setMoveCallback(std::function<void(int, int)>);
};
