#include "../include/connection_manager.h"
#include "../lib/hypno/hypno_data.h"

int main() {
    ClientConnectionManager client;
    client.connect("127.0.0.1", 4401, 3, "TCP");
    PacketBox box;
    box.data.push_back(std::vector<uchar>({'m', 'e', 's', 's', 'a', 'g', 'e', '_', '0', '\0'}));
    box.data.push_back(std::vector<uchar>({'m', 'e', 's', 's', 'a', 'g', 'e', '_', '1', '\0'}));
    box.data.push_back(std::vector<uchar>({'m', 'e', 's', 's', 'a', 'g', 'e', '_', '2', '\0'}));
    box.data.push_back(std::vector<uchar>({'m', 'e', 's', 's', 'a', 'g', 'e', '_', '3', '\0'}));
    box.data.push_back(std::vector<uchar>({'m', 'e', 's', 's', 'a', 'g', 'e', '_', '4', '\0'}));
    box.data.push_back(std::vector<uchar>({'m', 'e', 's', 's', 'a', 'g', 'e', '_', '5', '\0'}));
    box.data.push_back(std::vector<uchar>({'m', 'e', 's', 's', 'a', 'g', 'e', '_', '6', '\0'}));
    box.data.push_back(std::vector<uchar>({'m', 'e', 's', 's', 'a', 'g', 'e', '_', '7', '\0'}));
    box.data.push_back(std::vector<uchar>({'m', 'e', 's', 's', 'a', 'g', 'e', '_', '8', '\0'}));
    box.data.push_back(std::vector<uchar>({'m', 'e', 's', 's', 'a', 'g', 'e', '_', '9', '\0'}));
    client.send(box);
    client.clean();
}