#include "data_manager.h"

void BufToPacketBox(std::vector<uchar> &buf, PacketBox &box, int id, char type, int dataize) {
    dataize -= 7;
    int num = (int) buf.size() / dataize + (((int) buf.size() % dataize) != 0);
    box.data.clear();
    box.id = id;
    box.type = type;
    box.size = num;
    box.data.resize(num);
    for (short i = 0; i < num; i++) {
        auto segBeg = buf.begin() + i * dataize;
        auto segEnd = std::min(buf.begin() + (i + 1) * dataize, buf.end());
        box.data[i].assign((char*)&id, (char*)&id + 2);
        box.data[i].push_back(type);
        box.data[i].insert(box.data[i].end(), (char*)&num, (char*)&num + 2);
        box.data[i].insert(box.data[i].end(), (char*)&i, (char*)&i + 2);
        box.data[i].insert(box.data[i].end(), segBeg, segEnd);
    }
}

void PacketBoxToBuf(PacketBox &box, std::vector<uchar> &buf) {
    buf.clear();
    for (int i = 0; i < (int) box.data.size(); i++) {
        buf.insert(buf.end(), box.data[i].begin() + 7, box.data[i].end());
    }
}

PacketBox::~PacketBox() {
   this->data.clear(); 
}

void PacketBox::addPacket(std::vector<uchar> &packet) {
    this->data.push_back(packet);
    this->type = *(packet.data() + 2);
    this->size = *(short*)(packet.data() + 3);
}

void PacketBox::sort() {
    std::sort(data.begin(), data.end(), [](std::vector<uchar> a, std::vector<uchar> b){
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
    if (num == this->boxs[id].data.size()) {
        // printf("num = %d\n", num);
        // printf("size = %d\n", this->boxs[id].data.size());
        this->boxs[id].sort();
        this->onComplete(this->boxs[id]);
        this->boxs.erase(id);
    }
}

void BoxManager::setCompleteCallback(std::function<void(PacketBox&)> onComplete) {
    this->onComplete = onComplete;
}