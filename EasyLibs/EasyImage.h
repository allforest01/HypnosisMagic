#pragma once

#include <GL/glew.h>
#include <opencv2/opencv.hpp>

GLuint MatToTexture(const cv::Mat&);
void compressImage(const cv::Mat&, std::vector<uchar>&, int);
void decompressImage(const std::vector<uchar>&, cv::Mat&);
