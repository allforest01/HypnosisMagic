#include "EasySocket.h"
#include <stdio.h>

int main() {
    EasySocket easy_socket;
    char port[] = "1337";
    easy_socket.CreateServer(port);
}
