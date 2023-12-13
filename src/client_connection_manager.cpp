#include "client_connection_manager.h"

void ClientConnectionManager::clean() {
    ports.clear();
    for (int i = 0; i < this->size; i++) {
        clients[i].Close();
    }
    clients.clear();
    client_checker.Close();
    server_checker.Close();
}

ClientConnectionManager::~ClientConnectionManager() {
    this->clean();
}

void ClientConnectionManager::connect(char* host, int port, const char* type, int size) {
    this->size = size;

    for (int i = 0; i < size; i++) {
        ports.push_back(std::to_string(port + i));
    }

    clients.resize(size);

    std::vector<std::thread> threads;

    for (int pos = 0; pos < size; pos++) {
        int i = pos;
        // printf("server [%d] connect %s\n", i, (char*) ports[i].c_str());
        while (!clients[i].Connect(host, (char*) ports[i].c_str(), type));
        // printf("server [%d] connected\n", i);
    }

    // printf("client_checker %s\n", (char*) std::to_string(port + size).c_str());
    while (!client_checker.Connect(host, (char*) std::to_string(port + size).c_str(), "TCP"));
    // printf("client_checker connected\n");

    // printf("server_checker %s\n", (char*) std::to_string(port + size + 1).c_str());
    server_checker.Listen((char*) std::to_string(port + size + 1).c_str(), "TCP");
    // printf("server_checker connected\n");

    this->server_checker.setReceiveCallback(
        [&](SOCKET sock, char data[], int size, char host[]) {
            // printf("CHECK = %d\n", size);
        }
    );
}

void ClientConnectionManager::send(PacketBox& box) {

    int data_size = (int)box.data.size();
    int total_size = box.data[0].size() * (int)(data_size - 1) + box.data.back().size();
    int cur_packet_size = box.data[0].size();
    box.data.back().resize(cur_packet_size);

    // printf("-----------------------\n");
    // printf("data_size = %d\n", (int)data_size);
    // printf("total_size = %d\n", (int)total_size);
    // printf("cur_packet_size = %d\n", (int)cur_packet_size);

    std::vector<uchar> data_info;
    data_info.insert(data_info.end(), (char*) &data_size, (char*) &data_size + 4);
    data_info.insert(data_info.end(), (char*) &total_size, (char*) &total_size + 4);
    data_info.insert(data_info.end(), (char*) &cur_packet_size, (char*) &cur_packet_size + 4);

    this->client_checker.sendData((char*) data_info.data(), 12);

    int d = data_size / this->size + ((data_size % this->size) != 0);

    std::vector<std::thread> threads;

    for (int i = 0; i < this->size; i++) {
        threads.emplace_back(std::thread([&, i]()
        {
            int seg_start = i * d;
            int seg_end = std::min((i + 1) * d, data_size);

            for (int j = seg_start; j < seg_end; j++) {

                if (j == data_size - 1) {
                    // printf("---RECV CHECK----\n");
                    while (this->server_checker.receiveData(1) == -1);
                    // printf("-----------------\n");
                }

                while (this->clients[i].sendData((char*)box.data[j].data(), box.data[j].size()) == -1);

            }
        }));
    }

    for (int i = 0; i < this->size; i++) {
        threads[i].join();
    }

    // printf("Sent!\n");
}