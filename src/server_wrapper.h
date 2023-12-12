#include <mutex>

#include "socket_manager.h"
#include "events_manager.h"
#include "frame_wrapper.h"
#include "server_connection_manager.h"

struct ServerWrapper {
    std::string client_host;
    std::string PORT_S, PORT_M, PORT_K;

    FrameWrapper frame_wrapper;

    ClientSocketManager client_mouse;
    ClientSocketManager client_keyboard;
    ServerConnectionManager server_screen;

    std::queue<MouseEvent> mouse_events;
    std::queue<KeyboardEvent> keyboard_events;
};
