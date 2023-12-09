#pragma once

#include <stdlib.h>
#include <thread>
#include <chrono>
#include <functional>
#include "../lib/hypno/hypno_socket.h"
#include "../lib/hypno/hypno_data.h"

class ClientConnectionManager {
private:
    int size;
    std::vector<std::string> ports;
    std::vector<HypnoClient> clients;
    HypnoClient client_checker;
    HypnoServer server_checker;
public:
    void clean();
    void connect(char*, int, const char*, int);
    void send(PacketBox&);
};
