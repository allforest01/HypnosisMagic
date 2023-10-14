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
    HHOOK keyboardHook;
    MSG msg;
    KeyMapping keymap;
    // void (*KeydownCallback)(int);
    // void (*KeyupCallback)(int);
public:
    cv::Mat CaptureScreen();
    void SendKeyPress(int os, int keyCode);
    void SendKeyRelease(int os, int keyCode);
    void StartHook();
    void MsgLoop();
    void Unhook();
};
