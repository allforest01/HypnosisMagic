#include "EasyData.h"

void BufToPacketBox(std::vector<uchar> &buf, PacketBox &box, int id, char type, int num) {
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

void PacketBoxToBuf(PacketBox &box, std::vector<uchar> &buf) {
    buf.clear();
    for (int i = 0; i < box.packets.size(); i++) {
        buf.insert(buf.end(), box.packets[i].begin() + 6, box.packets[i].end());
    }
}

void PacketBox::addPacket(std::vector<uchar> &packet) {
    this->packets.push_back(packet);
    this->type = *((char*)packet.data() + 4);
    this->isComplete = *((char*)packet.data() + 5);
}

void BoxManager::addPacketToBox(std::vector<uchar> &packet) {
    int id = *((int*)packet.data());
    if (!this->boxs.count(id)) {
        this->boxs[id] = PacketBox();
    }
    this->boxs[id].addPacket(packet);
    if (this->boxs[id].isComplete) {
        this->completeCallback(this->boxs[id]);
        this->boxs.erase(id);
    }
}

void BoxManager::setCompleteCallback(std::function<void(PacketBox&)> completeCallback) {
    this->completeCallback = completeCallback;
}