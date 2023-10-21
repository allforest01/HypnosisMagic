#include "EasySocket.h"

void initEasySocket() {
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

void cleanEasySocket() {   
    #ifdef WINDOWS
        WSACleanup();
    #endif
}

EasyServer::EasyServer(char* port, const char* type) {
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
        return;
    }
    hints.ai_flags = AI_PASSIVE;
    int err = getaddrinfo(NULL, port, &hints, &result);
    if (err) {
        printf("getaddrinfo failed: %d\n", err);
        return;
    }
    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed!\n");
        freeaddrinfo(result);
        return;
    }
    err = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);
    if (err) {
        printf("bind failed: %d\n", err);
        closesocket(ListenSocket);
        return;
    }
    if (strcmp(type, "TCP") == 0) {
        if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
            printf("listen failed!");
            closesocket(ListenSocket);
            return;
        }
        printf("Listening port %s...\n", port);
        SOCKET ClientSocket;
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed!\n");
            closesocket(ListenSocket);
            return;
        }
        printf("Client connected!\n");
        this->listen_socket = ClientSocket;
    }
    else if (strcmp(type, "UDP") == 0) {
        printf("UDP Socket is created!\n");
        this->listen_socket = ListenSocket;
    }
}

EasyServer::~EasyServer() {
    closesocket(this->listen_socket);
    this->services = nullptr;
}

void EasyServer::setServices(std::function<void(SOCKET, char[], int)> services) {
    this->services = services;
}

void EasyServer::TCPReceive() {
    SOCKET listen_socket = this->listen_socket;
    char buffer[MAX_BYTES];
    int bytesRead = recv(listen_socket, buffer, MAX_BYTES, 0);
    printf("bytesRead = %d\n", bytesRead);
    if (bytesRead <= 0) return;
    this->services(listen_socket, buffer, bytesRead);
}

void EasyServer::UDPReceive() {
    SOCKET listen_socket = this->listen_socket;
    char buffer[MAX_BYTES];
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int bytesRead = recvfrom(listen_socket, buffer, sizeof(buffer), 0, (sockaddr*)&client_address, &client_address_size);
    // char ipv4[INET_ADDRSTRLEN];
    // inet_ntop(AF_INET, &(client_address.sin_addr), ipv4, INET_ADDRSTRLEN);
    printf("bytesRead = %d\n", bytesRead);
    if (bytesRead <= 0) return;
    this->services(listen_socket, buffer, bytesRead);
}

EasyClient::EasyClient(char* host, char* port, const char* type) {
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
        return;
    }
    int err = getaddrinfo(host, port, &hints, &result);
    if (err) {
        printf("getaddrinfo failed: %d\n", err);
        return;
    }
    SOCKET connect_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (connect_socket == INVALID_SOCKET) {
        printf("socket failed!\n");
        freeaddrinfo(result);
        return;
    }
    if (strcmp(type, "TCP") == 0) {
        err = connect(connect_socket, result->ai_addr, (int)result->ai_addrlen);
        char ipv4[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, result->ai_addr, ipv4, INET_ADDRSTRLEN);
        printf("from address = %s\n", ipv4);
        freeaddrinfo(result);
        if (err == SOCKET_ERROR) {
            printf("connect failed: %d\n", err);
            closesocket(connect_socket);
            return;
        }
        printf("Connect successful!\n");
    }
    else if (strcmp(type, "UDP") == 0) {
        char ipv4[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, result->ai_addr, ipv4, INET_ADDRSTRLEN);
        this->server_address = result;
        printf("Socket created for UDP communication!\n");
    }
    this->connect_socket = connect_socket;
}

EasyClient::~EasyClient() {
    closesocket(this->connect_socket);
    freeaddrinfo(this->server_address);
}

bool EasyClient::sendData(char* data, int size) {
    SOCKET connect_socket = this->connect_socket;
    struct addrinfo* server_address = this->server_address;
    if (server_address == NULL) return send(connect_socket, data, size, 0);
    int bytesSend = sendto(connect_socket, data, size, 0, server_address->ai_addr, server_address->ai_addrlen);
    printf("bytesSend = %d\n", bytesSend);
    return bytesSend;
}
