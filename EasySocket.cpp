#include "EasySocket.h"
#include <string.h>
#include <stdio.h>
// #include <thread>

EasySocket::EasySocket() {
#ifdef WINDOWS
    WSADATA wsaData;
    int err = 0;
    if (err = WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        printf("WSAStartup failed: %d\n", &err);
        return;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        fprintf(stderr, "Version 2.2 is not available!\n");
        WSACleanup();
        return;
    }
#endif
}

EasySocket::~EasySocket() {   
#ifdef WINDOWS
    WSACleanup();
#endif
}

void __closesocket(SOCKET &socket) {
#ifdef WINDOWS
    closesocket(socket);
#else
    close(socket);
#endif
}

int EasySocket::CreateServer(char* port) {
    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    int err = getaddrinfo(NULL, port, &hints, &result);
    if (err) {
        printf("getaddrinfo failed: %d\n", err);
        return 1;
    }
    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed!\n");
        freeaddrinfo(result);
        return 2;
    }
    err = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);
    if (err) {
        printf("bind failed: %d\n", err);
        __closesocket(ListenSocket);
        return 3;
    }
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen failed!");
        __closesocket(ListenSocket);
        return 4;
    }
    printf("Listening port %s...\n", port);
    SOCKET ClientSocket;
    while (true) {
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed!\n");
            __closesocket(ListenSocket);
            continue;
        }
        ServClient(ClientSocket);
    }
    return 0;
}

void EasySocket::FreeServer() {
    // ...
}

void EasySocket::ServClient(SOCKET client) {
    printf("Client connected!\n");
    char chunk[256];
    while (recv(client, chunk, 256, 0)) {
        printf("%d: %s\n", client, chunk);
    }
}

// void EasySocket::NewThread(SOCKET client) {
// #ifdef WINDOWS
//     _beginthreadex(0, 0, ServClient, (void*)&client, 0, 0);
// #else

// #endif
// }

// void EasySocket::ConnectTo(char* host, char* port) {

// }