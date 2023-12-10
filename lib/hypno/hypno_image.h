#pragma once

// #include "raylib.h"
#include <GL/glew.h>
#include <opencv2/opencv.hpp>
#include "../stb/stb_image.h"

// Texture2D MatToTexture(const cv::Mat&);
void MatToTexture(const cv::Mat&, GLuint&);
void ImageToTexture(unsigned char*, GLuint&, int& width, int& height, int& channels);
void compressImage(const cv::Mat&, std::vector<uchar>&, int);
void decompressImage(const std::vector<uchar>&, cv::Mat&);
