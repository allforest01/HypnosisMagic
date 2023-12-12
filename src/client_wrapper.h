#include <mutex>

#include "socket_manager.h"
#include "events_manager.h"
#include "client_connection_manager.h"

struct ClientWrapper {
    std::string PORT_S, PORT_M, PORT_K;

    ServerSocketManager server_mouse;
    ServerSocketManager server_keyboard;
    ClientConnectionManager client_screen;

    std::queue<PacketBox> frame_box_queue;
    std::queue<KeyboardEvent> keyboard_events;
};
