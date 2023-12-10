#include "../include/server_connection_manager.h"

void ServerConnectionManager::clean() {
    ports.clear();
    for (int i = 0; i < this->size; i++) {
        servers[i].hypnoClose();
    }
    servers.clear();
}

void ServerConnectionManager::listen(char* host, int port, const char* type, int size) {
    this->size = size;

    for (int i = 0; i < size; i++) {
        ports.push_back(std::to_string(port + i));
    }

    servers.resize(size);

    std::vector<std::thread> threads;

    for (int pos = 0; pos < size; pos++) {
        // std::thread cur_thread([&]()
        {
            int i = pos;
            printf("START HYPNO LISTEN %d %s\n", port, (char*) ports[i].c_str());
            servers[i].hypnoListen((char*) ports[i].c_str(), type);
            printf("END HYPNO LISTEN %d %s\n", port, (char*) ports[i].c_str());
        }
        // );
        // threads.emplace_back(std::move(cur_thread));
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // for (int i = 0; i < size; i++) {
    //     threads[i].join();
    // }

    // std::this_thread::sleep_for(std::chrono::milliseconds(16));

    printf("OKE %s\n", (char*) std::to_string(port + size).c_str());
    server_checker.hypnoListen((char*) std::to_string(port + size).c_str(), "TCP");
    printf("OKE\n");

    printf("OKI %s\n", (char*) std::to_string(port + size + 1).c_str());
    while (!client_checker.hypnoConnect(host, (char*) std::to_string(port + size + 1).c_str(), "TCP"));
    printf("OKI\n");

    printf("Listened!\n");
}

void ServerConnectionManager::setCompleteCallback(std::function<void(PacketBox&)> onComplete) {
    this->onComplete = onComplete;
}

void ServerConnectionManager::receive() {

    int data_size = 0;
    int total_size = 0;
    int cur_packet_size = 0;
    this->server_checker.setCallback(
        [&data_size, &total_size, &cur_packet_size](SOCKET sock, char data[], int size, char host[]) {
            data_size = *(int*)data;
            total_size = *(int*)((char*) data + 4);
            cur_packet_size = *(int*)((char*) data + 8);
        }
    );
    while (this->server_checker.receiveData(12) == -1);

    PacketBox box;
    box.size = data_size;

    printf("-----------------------\n");
    printf("data_size = %d\n", (int)data_size);
    printf("total_size = %d\n", (int)total_size);
    printf("cur_packet_size = %d\n", (int)cur_packet_size);

    box.data.resize(data_size);

    int d = data_size / this->size + ((data_size % this->size) != 0);

    std::vector<std::thread> threads;

    // printf("this->size = %d\n", this->size);

    // int sub_total_size = total_size;

    for (int i = 0; i < this->size; i++) {
        // std::thread cur_thread([&, pos]()
        threads.emplace_back(std::thread([&, i]()
        {
            int seg_start = i * d;
            int seg_end = std::min((i + 1) * d, data_size);

            for (int j = seg_start; j < seg_end; j++) {

                int packet_size = cur_packet_size;
                // if (j == data_size - 1) {
                //     if (total_size % cur_packet_size) {
                //         packet_size = total_size % cur_packet_size;
                //     }
                //     printf("mod = %d\n", packet_size);
                //     printf("---SEND CHECK----\n");
                //     while (this->client_checker.sendData((char*) "1", 1) == -1);
                //     printf("-----------------\n");
                // }
                // sub_total_size -= packet_size;

                // printf("------READ SIZE-----\n");
                // this->servers[i].setCallback(
                //     [&packet_size](SOCKET sock, char data[], int size, char host[]) {
                //         packet_size = *(int*)data;
                //     }
                // );
                // while (this->servers[i].receiveData(4) == -1);
                // printf("---------------\n");

                // std::this_thread::sleep_for(std::chrono::milliseconds(5));

                // printf("---SEND CHECK----\n");
                //this->checker.sendData((char*) "1", 1);
                // printf("-------\n");

                // std::this_thread::sleep_for(std::chrono::milliseconds(5));

                // printf("data_size packet_size = %d %d\n", data_size, packet_size);
                // fflush(stdout);

                // std::this_thread::sleep_for(std::chrono::milliseconds(16));

                // printf("------READ DATA-----\n");
                this->servers[i].setCallback(
                    [&](SOCKET sock, char data[], int size, char host[]) {
                        // printf("size = %d\n", size);
                        box.data[j].insert(box.data[j].end(), data, data + size);
                    }
                );
                
                int bytesRead = 0;
                while (packet_size) {
                    bytesRead = this->servers[i].receiveData(packet_size);
                    if (bytesRead == -1) continue;
                    packet_size -= bytesRead;
                    // fflush(stdout);
                }
                printf("[%d] ", j);
                // printf("---------------\n");
            }
            // std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        // );
        // cur_thread.join();
        ));
    }

    for (int i = 0; i < this->size; i++) {
        threads[i].join();
    }

    // std::this_thread::sleep_for(std::chrono::milliseconds(16));

    printf("Done!\n");

    box.type = 'I';

    // printf("onComplete!\n");
    int last_size = total_size % cur_packet_size;
    if (last_size == 0) last_size = cur_packet_size;
    box.data.back().resize(last_size);

    onComplete(box);
}
