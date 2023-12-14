#pragma once

#include <functional>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define WINVER 0x0605
    #include <Windows.h>
    #include <WinUser.h>
    #include <opencv2/opencv.hpp>
#else
    #include <unistd.h>
    #include <stdio.h>
    #include <ApplicationServices/ApplicationServices.h>
    #include <CoreGraphics/CoreGraphics.h>
    #include <opencv2/opencv.hpp>
#endif

enum keytype { KeyDown, KeyUp };
enum mousetype { LDown, LUp, RDown, RUp, MouseMove, MouseWheel };

#pragma pack(push, 1)
struct KeyboardEvent {
    keytype type; int keyCode;
    KeyboardEvent(keytype type, int keyCode):
        type(type), keyCode(keyCode) {}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MouseEvent {
    mousetype type; float x, y;
    MouseEvent(mousetype type, float x, float y):
        type(type), x(x), y(y) {}
};
#pragma pack(pop)

class EventsManager {
public:
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    int screen_x, screen_y;
    void toScreenCoord(int&, int&);
#endif

    int width, height;

    static EventsManager& getInstance() {
        static EventsManager instance;
        return instance;
    }

    EventsManager();

    cv::Mat captureScreen();

    void emitKeyDown(int);
    void emitKeyUp(int);
    void emitLDown(int, int);
    void emitLUp(int, int);
    void emitRDown(int, int);
    void emitRUp(int, int);
    void emitMove(int, int);
    void emitWheel(int, int);
};
