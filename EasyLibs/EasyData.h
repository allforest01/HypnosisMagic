#pragma once

#include <vector>
#include <algorithm>
#include <map>
#include <opencv2/opencv.hpp>

typedef unsigned char uchar;

class PacketBox {
public:
    short id; char type; short size;
    std::vector<std::vector<uchar>> packets;
    PacketBox(): id(-1), type(-1), size(-1) {}
    ~PacketBox();
    void addPacket(std::vector<uchar>&);
    void sort();
};

class BoxManager {
public:
    std::map<int, PacketBox> boxs;
    std::function<void(PacketBox&)> onComplete;
    void addPacketToBox(std::vector<uchar>&);
    void setCompleteCallback(std::function<void(PacketBox&)>);
};

void BufToPacketBox(std::vector<uchar>&, PacketBox&, int, char, int);
void PacketBoxToBuf(PacketBox&, std::vector<uchar>&);