#pragma once

#include <GL/glew.h>
#include <opencv2/opencv.hpp>

struct EImage {
    long long size;
    int width, height;
    char *data;
};

class EasyToolkit {
private:
public:
    static EasyToolkit& getInstance() {
        static EasyToolkit instance;
        return instance;
    }
    GLuint MatToTexture(const cv::Mat&);
    void MatToEImage(const cv::Mat&, EImage&);
    void EImageToMat(const EImage&, cv::Mat&);
    int EImageToStr(const EImage&, char*&);
    void StrToEImage(const char*, EImage&);
};
