#include <winsock2.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

int main() {
    WSADATA wsaData;
    int failedCode = 0;
    if (failedCode = WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        printf("WSAStartup failedCode: %d\n", &failedCode);
        return 1;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        fprintf(stderr, "Version 2.2 is not available!\n");
        WSACleanup();
        return 2;
    }

    // code here

    WSACleanup();
}
