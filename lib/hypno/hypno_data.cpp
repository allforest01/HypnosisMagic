#include "hypno_data.h"

void BufToPacketBox(std::vector<uchar> &buf, PacketBox &box, int id, char type, int packet_size) {
    packet_size -= 7;
    int num = (int) buf.size() / packet_size + (((int) buf.size() % packet_size) != 0);
    box.packets.clear();
    box.id = id;
    box.type = type;
    box.size = num;
    box.packets.resize(num);
    for (short i = 0; i < num; i++) {
        auto seg_begin = buf.begin() + i * packet_size;
        auto seg_end = std::min(buf.begin() + (i + 1) * packet_size, buf.end());
        box.packets[i].assign((char*)&id, (char*)&id + 2);
        box.packets[i].push_back(type);
        box.packets[i].insert(box.packets[i].end(), (char*)&num, (char*)&num + 2);
        box.packets[i].insert(box.packets[i].end(), (char*)&i, (char*)&i + 2);
        box.packets[i].insert(box.packets[i].end(), seg_begin, seg_end);
    }
}

void PacketBoxToBuf(PacketBox &box, std::vector<uchar> &buf) {
    buf.clear();
    for (int i = 0; i < (int) box.packets.size(); i++) {
        buf.insert(buf.end(), box.packets[i].begin() + 7, box.packets[i].end());
    }
}

PacketBox::~PacketBox() {
   this->packets.clear(); 
}

void PacketBox::addPacket(std::vector<uchar> &packet) {
    this->packets.push_back(packet);
    this->type = *(packet.data() + 2);
    this->size = *(short*)(packet.data() + 3);
}

void PacketBox::sort() {
    std::sort(packets.begin(), packets.end(), [](std::vector<uchar> a, std::vector<uchar> b){
        int a_order = *(short*)(a.data() + 5);
        int b_order = *(short*)(b.data() + 5);
        return a_order < b_order;
    });
}

void BoxManager::addPacketToBox(std::vector<uchar> &packet) {
    short id = *(packet.data());
    short num = *(short*)(packet.data() + 3);
    if (!this->boxs.count(id)) {
        this->boxs[id] = PacketBox();
    }
    // printf("Hehe!");
    this->boxs[id].addPacket(packet);
    if (num == this->boxs[id].packets.size()) {
        // printf("num = %d\n", num);
        // printf("size = %d\n", this->boxs[id].packets.size());
        this->boxs[id].sort();
        this->onComplete(this->boxs[id]);
        this->boxs.erase(id);
    }
}

void BoxManager::setCompleteCallback(std::function<void(PacketBox&)> onComplete) {
    this->onComplete = onComplete;
}