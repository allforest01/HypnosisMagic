#pragma once

#include <stdlib.h>
#include <thread>
#include <chrono>
#include "../lib/hypno/hypno_socket.h"
#include "../lib/hypno/hypno_data.h"

class ServerConnectionManager {
private:
    int size;
    std::vector<std::string> ports;
    std::vector<HypnoServer> servers;
    std::vector<std::thread> threads;
public:
    void clean();
    void listen(int, int, const char*);
    void receive(PacketBox&, int);
};

class ClientConnectionManager {
private:
    int size;
    std::vector<std::string> ports;
    std::vector<HypnoClient> clients;
    std::vector<std::thread> threads;
public:
    void clean();
    void connect(char*, int, int, const char*);
    void send(PacketBox&);
};
