#pragma once

#include <GL/glew.h>
#include <opencv2/opencv.hpp>

class EasyToolkit {
private:
public:
    static EasyToolkit& getInstance() {
        static EasyToolkit instance;
        return instance;
    }
    GLuint matToTexture(const cv::Mat&);
};