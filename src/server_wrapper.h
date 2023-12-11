#include <mutex>

#include "socket_manager.h"
#include "events_manager.h"

struct ServerWrapper {
    ClientSocketManager client_mouse;
    ClientSocketManager client_keyboard;
    ServerSocketManager server_screen;

    std::queue<MouseEvent> mouse_events;
    std::queue<KeyboardEvent> keyboard_events;
};
