#include "EasySocket.h"
#include <string.h>
#include <stdio.h>
// #include <thread>

#define MAX_BYTES 1000000

#ifdef WINDOWS
    #define __closesocket closesocket
#else
    #define __closesocket close
#endif

EasySocket::EasySocket() {
    server_address = NULL;
#ifdef WINDOWS
    WSADATA wsaData;
    int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (err) {
        printf("WSAStartup failed: %d\n", err);
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

int EasySocket::CreateServer(char* port, const char* type) {
    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    if (strcmp(type, "TCP") == 0) {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    }
    else if (strcmp(type, "UDP") == 0) {
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
    }
    else {
        printf("type error!\n");
        return -1;
    }
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
    if (strcmp(type, "TCP") == 0) {
        if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
            printf("listen failed!");
            __closesocket(ListenSocket);
            return 4;
        }
        printf("Listening port %s...\n", port);
        SOCKET ClientSocket;
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed!\n");
            __closesocket(ListenSocket);
            return 5;
        }
        printf("Client connected!\n");
        EasySocket::getInstance().server_socket = ClientSocket;
    }
    else if (strcmp(type, "UDP") == 0) {
        printf("UDP Socket is created!\n");
        EasySocket::getInstance().server_socket = ListenSocket;
    }
    return 0;
}

void EasySocket::FreeServer() {
    __closesocket(EasySocket::getInstance().server_socket);
}

void EasySocket::TCPReceive() {
    SOCKET socket = EasySocket::getInstance().server_socket;
    char buffer[MAX_BYTES];
    int bytesRead = recv(socket, buffer, MAX_BYTES, 0);
    if (bytesRead == 0) {
        printf("Received 0 bytes!\n");
        return;
    }
    EasySocket::getInstance().Services(socket, buffer, bytesRead);
    printf("Call Services!\n");
}

void EasySocket::UDPReceive() {
    SOCKET socket = EasySocket::getInstance().server_socket;
    char buffer[MAX_BYTES];
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int bytesRead = recvfrom(socket, buffer, sizeof(buffer), 0, (sockaddr*)&client_address, &client_address_size);
    char ipv4[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_address.sin_addr), ipv4, INET_ADDRSTRLEN);
    printf("from address = %s\n", ipv4);
    if (bytesRead < 0) return;
    EasySocket::getInstance().Services(socket, buffer, bytesRead);
}

// void EasySocket::NewThread(SOCKET client) {
// #ifdef WINDOWS
//     _beginthreadex(0, 0, ServClient, (void*)&client, 0, 0);
// #else

// #endif
// }

SOCKET EasySocket::ConnectTo(char* host, char* port, const char* type) {
    printf("!!!server_address = %d\n", server_address);
    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    if (strcmp(type, "TCP") == 0) {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    }
    else if (strcmp(type, "UDP") == 0) {
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
    }
    else {
        printf("type error!\n");
        return -1;
    }
    int err = getaddrinfo(host, port, &hints, &result);
    if (err) {
        printf("getaddrinfo failed: %d\n", err);
        return 0;
    }
    SOCKET ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        printf("socket failed!\n");
        freeaddrinfo(result);
        return 0;
    }
    if (strcmp(type, "TCP") == 0) {
        err = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
        char ipv4[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, result->ai_addr, ipv4, INET_ADDRSTRLEN);
        printf("from address = %s\n", ipv4);
        printf("!!!result->ai_addr = %d\n", result->ai_addr);
        freeaddrinfo(result);
        if (err == SOCKET_ERROR) {
            printf("connect failed: %d\n", err);
            __closesocket(ConnectSocket);
            return 0;
        }
        printf("Connect successful!\n");
    }
    else if (strcmp(type, "UDP") == 0) {
        // printf("!!!result = %d\n", result);
        // printf("!!!result->ai_addr = %d\n", result->ai_addr);
        char ipv4[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, result->ai_addr, ipv4, INET_ADDRSTRLEN);
        printf("from address = %s\n", ipv4);
        EasySocket::getInstance().server_address = result;
        printf("Socket created for UDP communication!\n");
    }
    return ConnectSocket;
}

bool EasySocket::SendData(SOCKET ConnectSocket, void* data, int size) {
    struct addrinfo* server_address = EasySocket::getInstance().server_address;
    // printf("server_address = %d\n", server_address);    
    if (server_address == NULL) {
        return send(ConnectSocket, (char*)data, size, 0);
    }
    printf("!server_address->ai_addr = %d\n", server_address->ai_addr);
    printf("!server_address->ai_addrlen = %d\n", server_address->ai_addrlen);
    printf("UDP is comming!\n");
    printf("size = %d\n", size);
    int bytesSend = sendto(ConnectSocket, (char*)data, size, 0, server_address->ai_addr, server_address->ai_addrlen);
    printf("bytesSend = %d\n", bytesSend);
}

void EasySocket::setServices(std::function<void(SOCKET, char[], int)> Services) {
    EasySocket::getInstance().Services = Services;
}
