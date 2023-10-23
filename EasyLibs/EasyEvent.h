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

enum os { win, mac };

class EasyEvent {

private:
#ifdef WINDOWS
//     HHOOK keyboardHook, mouseHook; MSG msg;
//     LRESULT CALLBACK KeyboardHookCallback(int, WPARAM, LPARAM);
//     static LRESULT CALLBACK GlobalKeyboardHookCallback(int, WPARAM, LPARAM);
//     LRESULT CALLBACK MouseHookCallback(int, WPARAM, LPARAM);
//     static LRESULT CALLBACK GlobalMouseHookCallback(int, WPARAM, LPARAM);
    void toScreenCoord(int&, int&);
#else
//     static CGEventRef MyCGEventCallback(CGEventTapProxy, CGEventType, CGEventRef, void*);
//     CFMachPortRef eventTap;
//     CFRunLoopSourceRef runLoopSource;
#endif

public:
    static EasyEvent& getInstance() {
        static EasyEvent instance;
        return instance;
    }

    EasyEvent();

    int screenX, screenY;
    int width, height;

    cv::Mat captureScreen();

    std::function<void(int)> onKeyDown;
    std::function<void(int)> onKeyUp;
    std::function<void(int, int)> onLDown;
    std::function<void(int, int)> onLUp;
    std::function<void(int, int)> onRDown;
    std::function<void(int, int)> onRUp;
    std::function<void(int, int)> onMove;

    // void startHook();
    // void msgLoop();
    // void stopHook();

    void sendKeyDown(os, int);
    void sendKeyUp(os, int);
    void sendLDown(int, int);
    void sendLUp(int, int);
    void sendRDown(int, int);
    void sendRUp(int, int);
    void sendMove(int, int);

    // void setKeyDownCallback(std::function<void(int)>);
    // void setKeyUpCallback(std::function<void(int)>);
    // void setLDownCallback(std::function<void(int, int)>);
    // void setLUpCallback(std::function<void(int, int)>);
    // void setRDownCallback(std::function<void(int, int)>);
    // void setRUpCallback(std::function<void(int, int)>);
    // void setMoveCallback(std::function<void(int, int)>);
};
