#pragma once
#include "KeyMapping.h"

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

    void (*KeyDownCallback)(int);
    void (*KeyUpCallback)(int);
    void (*LDownCallback)(int, int);
    void (*LUpCallback)(int, int);
    void (*RDownCallback)(int, int);
    void (*RUpCallback)(int, int);
    void (*MoveCallback)(int, int);

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

    void setKeyDownCallback(void (*)(int));
    void setKeyUpCallback(void (*)(int));
    void setLDownCallback(void (*)(int, int));
    void setLUpCallback(void (*)(int, int));
    void setRDownCallback(void (*)(int, int));
    void setRUpCallback(void (*)(int, int));
    void setMoveCallback(void (*)(int, int));
};
