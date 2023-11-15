#include "EasyData.h"

void BufToPacketBox(std::vector<uchar> &buf, PacketBox &box, int id, char type, int packetSize) {
    packetSize -= 6;
    int num = (int) buf.size() / packetSize + (((int) buf.size() % packetSize) != 0);
    box.packets.clear();
    box.id = id;
    box.type = type;
    box.isComplete = true;
    box.packets.resize(num);
    for (int i = 0; i < num; i++) {
        auto segBeg = buf.begin() + i * packetSize;
        auto segEnd = std::min(buf.begin() + (i + 1) * packetSize, buf.end());
        box.packets[i].assign((char*)&id, (char*)&id + 4);
        box.packets[i].push_back(type);
        box.packets[i].push_back(i + 1 == num);
        box.packets[i].insert(box.packets[i].end(), segBeg, segEnd);
    }
}

void PacketBoxToBuf(PacketBox &box, std::vector<uchar> &buf) {
    buf.clear();
    for (int i = 0; i < (int) box.packets.size(); i++) {
        buf.insert(buf.end(), box.packets[i].begin() + 6, box.packets[i].end());
    }
}

PacketBox::~PacketBox() {
   this->packets.clear(); 
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
    // printf("Hehe!");
    this->boxs[id].addPacket(packet);
    if (this->boxs[id].isComplete) {
        // printf("size = %d\n", this->boxs[id].packets.size());
        this->onComplete(this->boxs[id]);
        if (this->boxs.count(id)) {
            // printf("count = %d\n", this->boxs.count(id));
            // this->boxs[id].packets.clear();
            this->boxs.erase(id);
            printf("cleared %d\n", id);
            printf("this->boxs.size() = %lu\n", this->boxs.size());
        }
    }
}

void BoxManager::setCompleteCallback(std::function<void(PacketBox&)> onComplete) {
    this->onComplete = onComplete;
}