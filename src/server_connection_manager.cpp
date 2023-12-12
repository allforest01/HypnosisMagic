#include "server_connection_manager.h"

void ServerConnectionManager::clean() {
    ports.clear();
    for (int i = 0; i < this->size; i++) {
        servers[i].Close();
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
        int i = pos;
        // printf("server [%d] listen %s\n", i, (char*) ports[i].c_str());
        servers[i].Listen((char*) ports[i].c_str(), type);
        // printf("server [%d] connected\n", i);
    }

    // printf("server_checker %s\n", (char*) std::to_string(port + size).c_str());
    server_checker.Listen((char*) std::to_string(port + size).c_str(), "TCP");
    // printf("server_checker connected\n");

    // printf("client_checker %s\n", (char*) std::to_string(port + size + 1).c_str());
    while (!client_checker.Connect(host, (char*) std::to_string(port + size + 1).c_str(), "TCP"));
    // printf("client_checker connected\n");
}

void ServerConnectionManager::setCompleteCallback(std::function<void(PacketBox&)> onComplete) {
    this->onComplete = onComplete;
}

void ServerConnectionManager::receive()
{
    int data_size = 0;
    int total_size = 0;
    int cur_packet_size = 0;
    this->server_checker.setReceiveCallback(
        [&](SOCKET sock, char data[], int size, char host[]) {
            data_size = *(int*)data;
            total_size = *(int*)((char*) data + 4);
            cur_packet_size = *(int*)((char*) data + 8);
        }
    );
    while (this->server_checker.receiveData(12) == -1);

    PacketBox box;
    box.size = data_size;

    // printf("-----------------------\n");
    // printf("data_size = %d\n", (int)data_size);
    // printf("total_size = %d\n", (int)total_size);
    // printf("cur_packet_size = %d\n", (int)cur_packet_size);

    box.data.resize(data_size);

    int d = data_size / this->size + ((data_size % this->size) != 0);

    std::vector<std::thread> threads;

    for (int i = 0; i < this->size; i++) {
        threads.emplace_back(std::thread([&, i]()
        {
            int seg_start = i * d;
            int seg_end = std::min((i + 1) * d, data_size);

            for (int j = seg_start; j < seg_end; j++) {

                int packet_size = cur_packet_size;

                if (j == data_size - 1) {
                    // printf("---SEND CHECK----\n");
                    while (this->client_checker.sendData((char*) "1", 1) == -1);
                    // printf("-----------------\n");
                }

                this->servers[i].setReceiveCallback(
                    [&](SOCKET sock, char data[], int size, char host[]) {
                        box.data[j].insert(box.data[j].end(), data, data + size);
                    }
                );
                
                int bytesRead = 0;
                while (packet_size) {
                    bytesRead = this->servers[i].receiveData(packet_size);
                    if (bytesRead == -1) continue;
                    // printf("(%d) ", bytesRead);
                    packet_size -= bytesRead;
                }
                // printf("[%d]\n", j);
            }
        }));
    }

    for (int i = 0; i < this->size; i++) {
        threads[i].join();
    }

    box.type = 'I';

    int last_size = total_size % cur_packet_size;
    if (last_size == 0) last_size = cur_packet_size;
    box.data.back().resize(last_size);

    onComplete(box);

    // printf("onComplete done!\n");
}
