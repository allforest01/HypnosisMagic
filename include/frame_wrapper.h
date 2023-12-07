#pragma once

#include <queue>
#include <GL/glew.h>
#include <imgui.h>
#include "../lib/hypno/hypno_image.h"

typedef unsigned char uchar;

// struct ImageWrapper {
//     GLuint image_texture;
//     std::queue<std::vector<uchar>> queue_image_data;
//     int image_width, image_height, image_channels;
//     int image_scaled_width, image_scaled_height;
//     int image_start_x, image_start_y;
// };

struct FrameWrapper {
    GLuint image_texture;
    std::queue<std::vector<uchar>> frame_queue;
    int width, height, channels;
    int scaled_width, scaled_height;
    bool scale_calculated;
    int start_x, start_y;

    FrameWrapper():
        width(0), height(0), channels(0),
        scaled_width(0), scaled_height(0),
        scale_calculated(false),
        start_x(0), start_y(0) {}

    void initTexture();
    void cleanTexture();
    void pushToTexture();
};