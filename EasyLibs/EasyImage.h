#pragma once

// #include "raylib.h"
#include <GL/glew.h>
#include <opencv2/opencv.hpp>

// Texture2D MatToTexture(const cv::Mat&);
void MatToTexture(const cv::Mat&, GLuint &textureID);
void compressImage(const cv::Mat&, std::vector<uchar>&, int);
void decompressImage(const std::vector<uchar>&, cv::Mat&);
