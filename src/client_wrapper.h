#include <mutex>

#include "socket_manager.h"
#include "events_manager.h"

struct ClientWrapper {
    ServerSocketManager server_mouse;
    ServerSocketManager server_keyboard;
    ClientSocketManager client_screen;

    std::queue<PacketBox> frame_box_queue;
    std::queue<KeyboardEvent> keyboard_events;
};
