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
    HHOOK keyboardHook; MSG msg;
    void (*KeydownCallback)(int);
    void (*KeyupCallback)(int);
public:
    static EasyEvent& getInstance() {
        static EasyEvent instance;
        return instance;
    }
    cv::Mat CaptureScreen();
    void SendKeyPress(int, int);
    void SendKeyRelease(int, int);
    void StartHook();
    void MsgLoop();
    void Unhook();
    void setKeydownCallback(void (*)(int));
    void setKeyupCallback(void (*)(int));
    LRESULT CALLBACK KeyboardHookCallback(int, WPARAM, LPARAM);
    static LRESULT CALLBACK GlobalKeyboardHookCallback(int, WPARAM, LPARAM);
};
