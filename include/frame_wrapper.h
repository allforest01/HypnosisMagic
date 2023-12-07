#pragma once

#include <queue>
#include <GL/glew.h>

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
    int start_x, start_y;

    void initTexture();
    void cleanTexture();
};