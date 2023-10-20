#pragma once

#include <vector>
#include <algorithm>
#include <map>
#include <opencv2/opencv.hpp>

typedef unsigned char uchar;

class PacketBox {
public:
    int id; char type; bool isComplete;
    std::vector<std::vector<uchar>> packets;
    PacketBox(): id(-1), type(-1), isComplete(false) {}
    void addPacket(std::vector<uchar>&);
};

class BoxManager {
public:
    std::map<int, int> memId;
    std::vector<PacketBox> boxs;
    // to-do: change vector to map and clear when complete
    void addPacketToBox(std::vector<uchar>&);
};

void BufToPacketBox(std::vector<uchar>&, PacketBox&, int, char, int);
void PacketBoxToBuf(PacketBox&, std::vector<uchar>&);

void compressImage(const cv::Mat&, std::vector<uchar>&, int quality = 90);
void decompressImage(const std::vector<uchar>&, cv::Mat&);