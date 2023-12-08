#include "../include/connection_manager.h"

void ServerConnectionManager::clean() {
    ports.clear();
    for (int i = 0; i < this->size; i++) {
        servers[i].hypnoClose();
    }
    servers.clear();
}

void ServerConnectionManager::listen(int port, int size, const char* type) {
    this->size = size;

    for (int i = 0; i < size; i++) {
        ports.push_back(std::to_string(port + i));
    }

    servers.resize(size);
    threads.resize(size);

    for (int pos = 0; pos < size; pos++) {
        this->threads[pos] = std::thread([&](){
            int i = pos;
            servers[i].hypnoListen((char*) ports[i].c_str(), type);
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    for (int i = 0; i < size; i++) {
        this->threads[i].join();
    }
}

void ServerConnectionManager::receive(PacketBox& box, int segment_size) {
    int packets_size = 0;

    this->servers[0].setService(
        [&packets_size](SOCKET sock, char data[], int size, char host[]) {
            packets_size = *(int*)data;
        }
    );
    this->servers[0].receiveData(4);

    box.packets.resize(packets_size);

    printf("packets_size = %d\n", (int) box.packets.size());

    int d = packets_size / this->size + ((packets_size % this->size) != 0);

    for (int pos = 0; pos < this->size; pos++) {
        threads[pos] = std::thread([&](){
            int i = pos;
            int seg_start = i * d;
            int seg_end = std::min((i + 1) * d, packets_size);
            for (int j = seg_start; j < seg_end; j++) {
                this->servers[i].setService(
                    [&](SOCKET sock, char data[], int size, char host[]) {
                        box.packets[j] = std::vector<uchar>(data, data + size);
                    }
                );
                this->servers[i].receiveData(segment_size);
            }
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    for (int i = 0; i < this->size; i++) {
        threads[i].join();
    }

    for (int i = 0; i < packets_size; i++) {
        for (int j = 0; j < box.packets[i].size(); j++) {
            printf("%c", box.packets[i][j]);
        }
        printf("\n");
    }
}

void ClientConnectionManager::clean() {
    ports.clear();
    for (int i = 0; i < this->size; i++) {
        clients[i].hypnoClose();
    }
    clients.clear();
}

void ClientConnectionManager::connect(char* host, int port, int size, const char* type) {
    this->size = size;

    for (int i = 0; i < size; i++) {
        ports.push_back(std::to_string(port + i));
    }

    clients.resize(size);
    threads.resize(size);

    for (int pos = 0; pos < size; pos++) {
        threads[pos] = std::thread([&](){
            int i = pos;
            while (!clients[i].hypnoConnect(host, (char*) ports[i].c_str(), type));
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    for (int i = 0; i < size; i++) {
        threads[i].join();
    }

    printf("Connected!\n");
}

void ClientConnectionManager::send(PacketBox& box) {

    int packets_size = box.packets.size();

    this->clients[0].sendData((char*) &packets_size, 4);

    int d = packets_size / this->size + ((packets_size % this->size) != 0);

    for (int pos = 0; pos < this->size; pos++) {
        int i = pos;
        int seg_start = i * d;
        int seg_end = std::min((i + 1) * d, packets_size);
        threads[i] = std::thread([&](){
            for (int j = seg_start; j < seg_end; j++) {
                this->clients[i].sendData((char*)box.packets[j].data(), box.packets[j].size());
            }
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    for (int i = 0; i < this->size; i++) {
        threads[i].join();
    }

    printf("Sended!\n");
}