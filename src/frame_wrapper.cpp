#include "../include/frame_wrapper.h"

void FrameWrapper::initTexture() {
    glGenTextures(1, &this->image_texture);
}

void FrameWrapper::cleanTexture() {
    glDeleteTextures(1, &this->image_texture);
}