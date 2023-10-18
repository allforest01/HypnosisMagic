#include "EasyToolkit.h"

GLuint EasyToolkit::MatToTexture(const cv::Mat& bgrImage) {
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

void EasyToolkit::MatToEImage(const cv::Mat& mat, EImage& img) {
    img.size = mat.total() * mat.elemSize();
    delete[] img.data;
    img.data = new char[img.size];
    img.width = mat.cols;
    img.height = mat.rows;
    memcpy(img.data, mat.data, img.size);
}

void EasyToolkit::EImageToMat(const EImage& img, cv::Mat& mat) {
    mat = cv::Mat(img.height, img.width, CV_8UC3);
    memcpy(mat.data, img.data, img.size);
}

int EasyToolkit::EImageToStr(const EImage& img, char*& str) {
    delete[] str;
    str = new char[img.size + 16];
    memcpy(str, &img.size, 8);
    memcpy(str + 8, &img.height, 4);
    memcpy(str + 12, &img.width, 4);
    memcpy(str + 16, img.data, img.size);
    return img.size + 16;
}

void EasyToolkit::StrToEImage(const char* str, EImage& img) {
    memcpy(&img.size, str, 8);
    memcpy(&img.height, str + 8, 4);
    memcpy(&img.width, str + 12, 4);
    delete[] img.data;
    img.data = new char[img.size];
    memcpy(img.data, str + 16, img.size);
}
