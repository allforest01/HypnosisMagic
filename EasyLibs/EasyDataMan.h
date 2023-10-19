#pragma once

#include <vector>
#include <algorithm>
#include <map>

class PacketBox {
public:
    int id; char type; bool isComplete;
    std::vector<std::vector<char>> packets;
    PacketBox(): id(-1), type(-1), isComplete(false) {}
    void addPacket(std::vector<char>&);
};

class BoxManager {
public:
    std::map<int, int> memId;
    std::vector<PacketBox> boxs;
    void addPacketToBox(std::vector<char>&);
};

void BufToPacketBox(std::vector<char>&, PacketBox&, int, char, int);
void PacketBoxToBuf(PacketBox&, std::vector<char>&);