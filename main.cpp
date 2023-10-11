#include "EasySocket.h"
#include <stdio.h>

#ifdef WINDOWS
    #define sleep(x) Sleep(x * 1000)
#endif

int main() {
    EasySocket easy_socket;
    char port[] = "1337";
    int input;
    printf("input = ");
    scanf("%d", &input);
    if (input == 1) {
        easy_socket.CreateServer(port);
    }
    else {
        char host[256];
        printf("host = ");
        scanf("%s", host);
        SOCKET ConnectSocket = easy_socket.ConnectTo(host, port);
        if (!ConnectSocket) return 0;
        char buff[256];
        for (int i = 0; i < 100; i++) {
            snprintf(buff, sizeof(buff), "Heartbeat %d\n", i);
            easy_socket.SendData(ConnectSocket, buff, sizeof(buff));
            sleep(1);
        }
    }
}
