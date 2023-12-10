#include "frame_wrapper.h"

void FrameWrapper::initTexture() {
    glGenTextures(1, &this->image_texture);
}

void FrameWrapper::cleanTexture() {
    glDeleteTextures(1, &this->image_texture);
}

void FrameWrapper::pushToTexture() {
    auto frame = this->frame_queue.front(); this->frame_queue.pop();

    unsigned char* imageData = stbi_load_from_memory(frame.data(), frame.size(), &this->width, &this->height, &this->channels, 3);

    if (!this->scale_calculated) {
        this->scale_calculated = true;
        
        ImVec2 window_avail_size = ImGui::GetContentRegionAvail();
        window_avail_size.y -= 6;

        float window_aspect = window_avail_size.x / window_avail_size.y;
        float image_aspect = (float) this->width / this->height;

        float scale;
        if (window_aspect > image_aspect) {
            // imgui_wrapper.window is wider than the image
            scale = window_avail_size.y / this->height;
        } else {
            // imgui_wrapper.window is narrower than the image
            scale = window_avail_size.x / this->width;
        }

        this->scaled_width = this->width * scale;
        this->scaled_height = this->height * scale;
    }

    ImageToTexture(imageData, this->image_texture, this->width, this->height, this->channels);

    stbi_image_free(imageData);
}