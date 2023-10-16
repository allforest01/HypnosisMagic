#include "EasyToolkit.h"

GLuint EasyToolkit::matToTexture(const cv::Mat& bgrImage) {
    // 1. Flip the image vertically (if needed)
    // cv::Mat flippedImage;
    // cv::flip(bgrImage, flippedImage, 0);

    // 2. Generate an OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // 3. Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // 4. Set texture parameters (you can adjust these as needed)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 5. Upload the image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bgrImage.cols, bgrImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, bgrImage.data);

    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}