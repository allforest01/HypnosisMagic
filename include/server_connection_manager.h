#pragma once

#include <stdlib.h>
#include <thread>
#include <chrono>
#include <functional>
#include "../lib/hypno/hypno_socket.h"
#include "../lib/hypno/hypno_data.h"

class ServerConnectionManager {
private:
    int size;
    std::vector<std::string> ports;
    std::vector<HypnoServer> servers;
    HypnoServer server_checker;
    HypnoClient client_checker;
    std::function<void(PacketBox&)> onComplete;
public:
    void clean();
    void listen(char*, int, const char*, int);
    void receive();
    void setCompleteCallback(std::function<void(PacketBox&)>);
};
