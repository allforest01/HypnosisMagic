#pragma once

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
public:
    cv::Mat CaptureScreen();
private:

};
