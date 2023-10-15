#include "EasySocket.h"
#include "EasyEvent.h"
#include <stdio.h>
// #include <opencv2/opencv.hpp>
#include <iostream>

#define MAX_BYTES 256

#ifdef WINDOWS
    #define sleep(x) Sleep(x * 1000)
    #define os 0 // windows
#else
    #define os 1 // macos
#endif

EasyEvent easy_event;

EasySocket easy_socket;
SOCKET ConnectSocket;

void KeyDownCallback(int keyCode) {
    // char buff[MAX_BYTES];
    // snprintf(buff, sizeof(buff), "Press %d\n", keyCode);
    // easy_socket.SendData(ConnectSocket, buff, sizeof(buff));
    std::cout << "Down " << keyCode << '\n';
}

void KeyUpCallback(int keyCode) {
    std::cout << "Up " << keyCode << '\n';
}

void LDownCallback(int x, int y) {
    std::cout << "Left Down" << '\n';
}

void LUpCallback(int x, int y) {
    std::cout << "Left Up" << '\n';
}

void RDownCallback(int x, int y) {
    std::cout << "Right Down" << '\n';
}

void RUpCallback(int x, int y) {
    std::cout << "Right Up" << '\n';
}

void MoveCallback(int x, int y) {
    std::cout << "Move " << x << ' ' << y << '\n';
}

void Services(SOCKET id, char data[], int size) {
    printf("%d (%d): %s", id, size, data);
}

int main() {
    // char port[] = "1337";
    // int input;
    // printf("input = ");
    // scanf("%d", &input);
    // if (input == 1) {
    //     easy_socket.setServices(Services);
    //     easy_socket.CreateServer(port);
    // }
    // else
    {
        // char host[256];
        // printf("host = ");
        // scanf("%s", host);
        // ConnectSocket = easy_socket.ConnectTo(host, port);
        // if (!ConnectSocket) return 0;

        easy_event.setKeyDownCallback(KeyDownCallback);
        easy_event.setKeyUpCallback(KeyUpCallback);
        easy_event.setLDownCallback(LDownCallback);
        easy_event.setLUpCallback(LUpCallback);
        easy_event.setRDownCallback(RDownCallback);
        easy_event.setRUpCallback(RUpCallback);
        easy_event.setMoveCallback(MoveCallback);

        easy_event.StartHook();
        while (true) {
            easy_event.MsgLoop();
        }
        easy_event.Unhook();
    }

    // imshow("Test", easy_event.CaptureScreen());
    // cv::waitKey(5000);

}
