#pragma once

#include <stdlib.h>
#include <thread>
#include <chrono>
#include <functional>
#include "socket_manager.h"
#include "data_manager.h"

class ClientConnectionManager {
private:
    int size;
    std::vector<std::string> ports;
    std::vector<ClientSocketManager> clients;
    ClientSocketManager client_checker;
    ServerSocketManager server_checker;
public:
    ~ClientConnectionManager();
    void clean();
    void connect(char*, int, const char*, int);
    void send(PacketBox&);
};
