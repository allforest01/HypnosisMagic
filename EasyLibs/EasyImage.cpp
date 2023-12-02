#define STB_IMAGE_IMPLEMENTATION
#include "EasyImage.h"

// Texture2D MatToTexture(const cv::Mat& rgbImage) {
//     Image raylibImage;
//     raylibImage.data    = (void*)rgbImage.data;
//     raylibImage.width   = rgbImage.cols;
//     raylibImage.height  = rgbImage.rows;
//     raylibImage.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
//     raylibImage.mipmaps = 1;
//     Texture2D texture = LoadTextureFromImage(raylibImage);
//     return texture;
// }

void MatToTexture(const cv::Mat& bgrImage, GLuint& textureID) {
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bgrImage.cols, bgrImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, bgrImage.data);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void BufToTexture(std::vector<uchar>& buf, GLuint& textureID, int& width, int& height, int& channels) {
    unsigned char* imageData = stbi_load_from_memory(buf.data(), buf.size(), &width, &height, &channels, 3);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

    stbi_image_free(imageData);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void compressImage(const cv::Mat& mat, std::vector<uchar>& buffer, int quality) {
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(quality);
    cv::imencode(".jpg", mat, buffer, compression_params);
}

void decompressImage(const std::vector<uchar>& buffer, cv::Mat& mat) {
    mat = cv::imdecode(buffer, cv::IMREAD_COLOR);
}