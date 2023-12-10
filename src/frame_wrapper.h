#pragma once

#include <queue>
#include <GL/glew.h>
#include <imgui.h>
#include "image_manager.h"

typedef unsigned char uchar;

struct FrameWrapper {
    GLuint image_texture;
    std::queue<std::vector<uchar>> frame_queue;
    int width, height, channels, scaled_width, scaled_height, start_x, start_y;
    bool scale_calculated, is_hovered, is_focused;

    FrameWrapper():
        width(0), height(0), channels(0), scaled_width(0), scaled_height(0), start_x(0), start_y(0),
        scale_calculated(false), is_hovered(false), is_focused(false) {}

    void initTexture();
    void cleanTexture();
    void pushToTexture();
};
