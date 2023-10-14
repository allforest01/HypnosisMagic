#include "EasySocket.h"
#include "EasyEvent.h"
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>

#ifdef WINDOWS
    #define sleep(x) Sleep(x * 1000)
    #define os 0 // windows
#else
    #define os 1 // macos
#endif

EasySocket easy_socket; EasyEvent easy_event;
SOCKET ConnectSocket;

void KeydownCallback(int keyCode) {
    // char buff[20];
    // snprintf(buff, sizeof(buff), "Press %d\n", keyCode);
    // easy_socket.SendData(ConnectSocket, buff, sizeof(buff));
    printf("%d\n", keyCode);
}

void KeyupCallback(int keyCode) {
    // std::cout << "Up " << keyCode << '\n';
}

int main() {
    // char port[] = "1337";
    // int input;
    // printf("input = ");
    // scanf("%d", &input);
    // if (input == 1) {
    //     easy_socket.CreateServer(port);
    // }
    // else
    {
        // char host[256];
        // printf("host = ");
        // scanf("%s", host);
        // ConnectSocket = easy_socket.ConnectTo(host, port);
        // if (!ConnectSocket) return 0;
        easy_event.setKeydownCallback(KeydownCallback);
        easy_event.setKeyupCallback(KeyupCallback);
        easy_event.StartHook();
        while (true) {
            easy_event.MsgLoop();
        }
        easy_event.Unhook();
    }

    // imshow("Test", easy_event.CaptureScreen());
    // cv::waitKey(5000);
}
