#include "EasyImage.h"

// GLuint MatToTexture(const cv::Mat& bgrImage) {
//     GLuint textureID;
//     glGenTextures(1, &textureID);

//     glBindTexture(GL_TEXTURE_2D, textureID);

//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bgrImage.cols, bgrImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, bgrImage.data);

//     glBindTexture(GL_TEXTURE_2D, 0);

//     return textureID;
// }

void compressImage(const cv::Mat& mat, std::vector<uchar>& buffer, int quality) {
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(quality);
    cv::imencode(".jpg", mat, buffer, compression_params);
}

void decompressImage(const std::vector<uchar>& buffer, cv::Mat& mat) {
    mat = cv::imdecode(buffer, cv::IMREAD_COLOR);
}