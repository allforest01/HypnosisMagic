#pragma once

#include "EasyKeyCode.h"
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

enum keytype { KeyDown, KeyUp };
enum mousetype { LDown, LUp, RDown, RUp, MouseMove };

struct KeyboardEvent {
    keytype type; int keyCode;
    KeyboardEvent(keytype type, int keyCode):
        type(type), keyCode(keyCode) {}
};

struct MouseEvent {
    mousetype type; double x, y;
    MouseEvent(mousetype type, double x, double y):
        type(type), x(x), y(y) {}
};

class EasyEvent {

private:
#ifdef WINDOWS
    // HHOOK keyboardHook, mouseHook; MSG msg;
    // LRESULT CALLBACK KeyboardHookCallback(int, WPARAM, LPARAM);
    // static LRESULT CALLBACK GlobalKeyboardHookCallback(int, WPARAM, LPARAM);
    // LRESULT CALLBACK MouseHookCallback(int, WPARAM, LPARAM);
    // static LRESULT CALLBACK GlobalMouseHookCallback(int, WPARAM, LPARAM);
    void toScreenCoord(int&, int&);
#else
    // static CGEventRef MyCGEventCallback(CGEventTapProxy, CGEventType, CGEventRef, void*);
    // CFMachPortRef eventTap;
    // CFRunLoopSourceRef runLoopSource;
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

    void sendKeyDown(int);
    void sendKeyUp(int);
    void sendLDown(int, int);
    void sendLUp(int, int);
    void sendRDown(int, int);
    void sendRUp(int, int);
    void sendMove(int, int);

    // std::function<void(int)> onKeyDown;
    // std::function<void(int)> onKeyUp;
    // std::function<void(int, int)> onLDown;
    // std::function<void(int, int)> onLUp;
    // std::function<void(int, int)> onRDown;
    // std::function<void(int, int)> onRUp;
    // std::function<void(int, int)> onMove;

    // void setKeyDownCallback(std::function<void(int)>);
    // void setKeyUpCallback(std::function<void(int)>);
    // void setLDownCallback(std::function<void(int, int)>);
    // void setLUpCallback(std::function<void(int, int)>);
    // void setRDownCallback(std::function<void(int, int)>);
    // void setRUpCallback(std::function<void(int, int)>);
    // void setMoveCallback(std::function<void(int, int)>);

    // void startHook();
    // void msgLoop();
    // void stopHook();
};
