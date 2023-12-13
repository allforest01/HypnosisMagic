#pragma once

#include <stdlib.h>
#include <thread>
#include <chrono>
#include <functional>
#include "socket_manager.h"
#include "data_manager.h"

class ServerConnectionManager {
private:
    int size;
    std::vector<std::string> ports;
    std::vector<ServerSocketManager> servers;
    ServerSocketManager server_checker;
    ClientSocketManager client_checker;
    std::function<void(PacketBox&)> onComplete;
public:
    ~ServerConnectionManager();
    void clean();
    void listen(char*, int, const char*, int);
    void receive();
    void setCompleteCallback(std::function<void(PacketBox&)>);
};
