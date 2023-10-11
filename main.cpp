#include "EasySocket.h"
#include <stdio.h>

int main() {
    EasySocket easy_socket;
    char host[] = "127.0.0.1";
    char port[] = "1337";
    int input;
    printf("input = ");
    scanf("%d", &input);
    if (input == 1) {
        easy_socket.CreateServer(port);
    }
    else {
        SOCKET ConnectSocket = easy_socket.ConnectTo(host, port);
        char buff[256];
        for (int i = 0; i < 100; i++) {
            snprintf(buff, sizeof(buff), "Heartbeat %d\n", i);
            easy_socket.SendData(ConnectSocket, buff, sizeof(buff));
            sleep(1);
        }

    }
}
