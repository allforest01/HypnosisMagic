#include "EasyDataMan.h"

void BufToPacketBox(std::vector<char> &buf, PacketBox &box, int id, char type, int num) {
    num = std::min(num, (int) buf.size());
    box.packets.resize(num);
    int packetSize = buf.size() / num + ((buf.size() % num) != 1);
    for (int i = 0; i < num; i++) {
        auto start = buf.begin() + i * packetSize;
        auto end = std::min(buf.begin() + (i + 1) * packetSize, buf.end());
        box.packets[i].clear();
        box.packets[i].assign((char*)&id, (char*)&id + 4);
        box.packets[i].insert(box.packets[i].end(), (char*)&type, (char*)&type + 1);
        box.packets[i].push_back(char(i + 1 == num));
        box.packets[i].insert(box.packets[i].end(), start, end);
    }
}

void PacketBoxToBuf(PacketBox &box, std::vector<char> &buf) {
    buf.clear();
    for (int i = 0; i < box.packets.size(); i++) {
        buf.insert(buf.end(), box.packets[i].begin() + 6, box.packets[i].end());
    }
}

void PacketBox::addPacket(std::vector<char> &packet) {
    this->packets.push_back(packet);
    char *type = (char*)&packet + 4;
    bool *isComplete = (bool*)&packet + 5;
    this->type = *type;
    this->isComplete = *isComplete;
}

void BoxManager::addPacketToBox(std::vector<char> &packet) {
    int *id = (int*)packet.data();
    if (!this->memId.count(*id)) {
        this->memId[*id] = this->boxs.size();
        this->boxs.push_back(PacketBox());
    }
    int sid = this->memId[*id];
    this->boxs[sid].addPacket(packet);
}
