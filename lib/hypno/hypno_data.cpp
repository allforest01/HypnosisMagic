#include "hypno_data.h"

void BufToPacketBox(std::vector<uchar> &buf, PacketBox &box, int id, char type, int packet_size) {
    packet_size -= 7;
    int num = (int) buf.size() / packet_size + (((int) buf.size() % packet_size) != 0);
    box.data.clear();
    box.id = id;
    box.type = type;
    box.size = num;
    box.data.resize(num);
    for (short i = 0; i < num; i++) {
        auto seg_begin = buf.begin() + i * packet_size;
        auto seg_end = std::min(buf.begin() + (i + 1) * packet_size, buf.end());
        box.data[i].assign((char*)&id, (char*)&id + 2);
        box.data[i].emplace_back(type);
        box.data[i].insert(box.data[i].end(), (char*)&num, (char*)&num + 2);
        box.data[i].insert(box.data[i].end(), (char*)&i, (char*)&i + 2);
        box.data[i].insert(box.data[i].end(), seg_begin, seg_end);
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
    // printf("data.size() = %d\n", data.size());
    // for (int i = 0; i < (int) data.size(); i++) {
    //     printf("%d, ", data[i].size());
    // }
    // printf("\n");
    std::sort(data.begin(), data.end(), [](std::vector<uchar> a, std::vector<uchar> b){
        if (a.size() < 6 || b.size() < 6) {
            printf("a.size() = %d\n", a.size());
            printf("b.size() = %d\n", b.size());
            assert(false);
        }
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