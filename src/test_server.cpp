#include "../include/connection_manager.h"
#include "../lib/hypno/hypno_data.h"

int main() {
    ServerConnectionManager server;
    server.listen(4401, 3, "TCP");
    PacketBox box;
    server.receive(10);
    server.clean();
}