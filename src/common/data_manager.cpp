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

void PacketBox::sort() {
    std::sort(data.begin(), data.end(), [](std::vector<uchar> a, std::vector<uchar> b){
        int a_order = *(short*)(a.data() + 5);
        int b_order = *(short*)(b.data() + 5);
        return a_order < b_order;
    });
}

PacketBox::~PacketBox() {
   this->data.clear(); 
}
