#include "EasyData.h"

void BufToPacketBox(std::vector<uchar> &buf, PacketBox &box, int id, char type, int packetSize) {
    packetSize -= 6;
    int num =(int) buf.size() / packetSize + (((int) buf.size() % packetSize) != 0);
    box.packets.clear();
    box.id = id;
    box.type = type;
    box.isComplete = true;
    box.packets.resize(num);
    for (int i = 0; i < num; i++) {
        auto segbeg = buf.begin() + i * packetSize;
        auto segend = std::min(buf.begin() + (i + 1) * packetSize, buf.end());
        box.packets[i].assign((char*)&id, (char*)&id + 4);
        box.packets[i].push_back(type);
        box.packets[i].push_back(i + 1 == num);
        box.packets[i].insert(box.packets[i].end(), segbeg, segend);
    }
}

void PacketBoxToBuf(PacketBox &box, std::vector<uchar> &buf) {
    buf.clear();
    for (int i = 0; i < (int) box.packets.size(); i++) {
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
        this->onComplete(this->boxs[id]);
        this->boxs.erase(id);
    }
}

void BoxManager::setCompleteCallback(std::function<void(PacketBox&)> onComplete) {
    this->onComplete = onComplete;
}