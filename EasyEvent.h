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
#else
    static CGEventRef MyCGEventCallback(CGEventTapProxy, CGEventType, CGEventRef, void*);
#endif
public:
    static EasyEvent& getInstance() {
        static EasyEvent instance;
        return instance;
    }
    void (*KeyDownCallback)(int);
    void (*KeyUpCallback)(int);
    void (*LDownCallback)();
    void (*LUpCallback)();
    void (*RDownCallback)();
    void (*RUpCallback)();
    void (*MoveCallback)(int, int);

    cv::Mat CaptureScreen();
    void SendKeyPress(int, int);
    void SendKeyRelease(int, int);
    void StartHook();
    void MsgLoop();
    void Unhook();
    void setKeyDownCallback(void (*)(int));
    void setKeyUpCallback(void (*)(int));
    void setLDownCallback(void (*)());
    void setLUpCallback(void (*)());
    void setRDownCallback(void (*)());
    void setRUpCallback(void (*)());
    void setMoveCallback(void (*)(int, int));
};
