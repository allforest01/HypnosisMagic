#pragma once

// #include "raylib.h"
#include <GL/glew.h>
#include <opencv2/opencv.hpp>

// Texture2D MatToTexture(const cv::Mat&);
GLuint MatToTexture(const cv::Mat&);
void compressImage(const cv::Mat&, std::vector<uchar>&, int);
void decompressImage(const std::vector<uchar>&, cv::Mat&);
