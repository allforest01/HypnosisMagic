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

void KeydownCallback(int keyCode) {
    std::cout << "Down " << keyCode << '\n';
}

void KeyupCallback(int keyCode) {
    std::cout << "Up " << keyCode << '\n';
}

int main() {
    EasySocket easy_socket;
    EasyEvent easy_event;
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
        // char buff[256];
        // for (int i = 0; i < 100; i++) {
        //     snprintf(buff, sizeof(buff), "Heartbeat %d\n", i);
        //     easy_socket.SendData(ConnectSocket, buff, sizeof(buff));
        //     sleep(1);
        // }
        easy_event.StartHook();
        easy_event.setKeydownCallback(KeydownCallback);
        easy_event.setKeyupCallback(KeyupCallback);
        while (true) {
            easy_event.MsgLoop();
        }
        easy_event.Unhook();
    }

    // imshow("Test", easy_event.CaptureScreen());
    // cv::waitKey(5000);
}
