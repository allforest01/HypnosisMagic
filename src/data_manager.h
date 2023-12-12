#pragma once

#include <vector>
#include <algorithm>
#include <map>
#include <opencv2/opencv.hpp>

typedef unsigned char uchar;

class PacketBox {
public:
    short id; char type; short size;
    std::vector<std::vector<uchar>> data;
    PacketBox(): id(-1), type(-1), size(-1) {}
    ~PacketBox();
    void sort();
};

void BufToPacketBox(std::vector<uchar>&, PacketBox&, int, char, int);
void PacketBoxToBuf(PacketBox&, std::vector<uchar>&);