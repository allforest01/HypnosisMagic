#include "frame_wrapper.h"

bool FrameWrapper::isTextureGenerated() {
    return this->textureGenerated;
}

bool FrameWrapper::isTexturePushed() {
    return this->texturePushed;
}

void FrameWrapper::generateTexture() {
    glGenTextures(1, &this->image_texture);
    this->textureGenerated = true;
}

void FrameWrapper::cleanTexture() {
    glDeleteTextures(1, &this->image_texture);
    this->textureGenerated = false;
}

void FrameWrapper::pushToTexture() {
    if (!this->isTextureGenerated()) this->generateTexture();

    auto frame = this->frame_queue.front(); this->frame_queue.pop();

    unsigned char* imageData = stbi_load_from_memory(frame.data(), frame.size(), &this->width, &this->height, &this->channels, 3);

    ImageToTexture(imageData, this->image_texture, this->width, this->height, this->channels);

    stbi_image_free(imageData);

    texturePushed = true;
}