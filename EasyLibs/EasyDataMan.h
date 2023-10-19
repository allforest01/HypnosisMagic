#pragma once

#include <vector>
#include <algorithm>

class PacketBox {
public:
    int id; char type; bool isComplete;
    std::vector<std::vector<char>> packets;
    PacketBox(): isComplete(false) {}
    void addPacket(std::vector<char>&);
};

class ClientDataMan {
    std::vector<PacketBox> boxs;
};

class ServerDataMan {
    std::vector<PacketBox> boxs;
    // addPacketToBox(std::vector<char>&);
};

void BufToPacketBox(std::vector<char>&, PacketBox&, int, char, int);
void PacketBoxToBuf(PacketBox&, std::vector<char>&);