#include "socket_manager.h"

void initSocketManager() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
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
    printf("WSAStartup successful!\n");
#endif
}

void cleanSocketManager() {   
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    WSACleanup();
#endif
}

void ServerManager::TCPListen(char* port) {
    isTCPServer = true;

    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
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
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen failed!");
        closesocket(ListenSocket);
        return;
    }
    printf("Listening port %s...\n", port);
    SOCKET ClientSocket;
    socklen_t client_address_size = sizeof(this->client_address);
    ClientSocket = accept(ListenSocket, (sockaddr*)(&this->client_address), &client_address_size);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed!\n");
        closesocket(ListenSocket);
        return;
    }
    printf("Client connected!\n");
    this->listen_socket = ClientSocket;
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        int timeout = 500; // 500 miliseconds
    #else
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000; // 500 miliseconds
    #endif
    int flag = 1;
    if (setsockopt(this->listen_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int)) < 0) {
        printf("setsockopt failed!\n");
        return;
    }
}

void ServerManager::UDPListen(char* port) {
    isTCPServer = false;

    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
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
    printf("UDP Socket is created!\n");
    this->listen_socket = ListenSocket;
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        int timeout = 500; // 500 miliseconds
    #else
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000; // 500 miliseconds
    #endif
    if (setsockopt(this->listen_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
        printf("setsockopt failed!\n");
        return;
    }
}

void ServerManager::Listen(char* port, const char* type) {
    if (strcmp(type, "TCP") == 0) this->TCPListen(port);
    else if (strcmp(type, "UDP") == 0) this->UDPListen(port);
    else { printf("type error!\n"); return; }
}

void ServerManager::Close() {
    closesocket(this->listen_socket);
    listen_socket = 0;
    this->service = nullptr;
}

void ServerManager::setService(std::function<void(SOCKET, char[], int, char[])> service) {
    this->service = service;
}

void ServerManager::TCPReceive(int max_bytes) {
    SOCKET listen_socket = this->listen_socket;
    char* buffer = new char[max_bytes];
    int bytesRead = recv(listen_socket, buffer, max_bytes, 0);
    if (bytesRead <= 0) {
        delete[] buffer;
        return;
    }
    // printf("bytesRead = %d\n", bytesRead);
    this->service(listen_socket, buffer, bytesRead, NULL);
    delete[] buffer;
}

void ServerManager::UDPReceive(int max_bytes) {
    SOCKET listen_socket = this->listen_socket;
    char* buffer = new char[max_bytes];
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int bytesRead = recvfrom(listen_socket, buffer, max_bytes, 0, (sockaddr*)&client_address, &client_address_size);
    char ipv4[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_address.sin_addr), ipv4, INET_ADDRSTRLEN);
    // printf("ipv4 = %s\n", ipv4);
    if (bytesRead <= 0) {
        delete[] buffer;
        return;
    }
    // printf("bytesRead = %d\n", bytesRead);
    this->service(listen_socket, buffer, bytesRead, ipv4);
    delete[] buffer;
}

void ServerManager::receiveData(int max_bytes) {
    if (this->isTCPServer) TCPReceive(max_bytes);
    else UDPReceive(max_bytes);
}

bool ClientManager::TCPConnect(char* host, char* port) {
    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int err = getaddrinfo(host, port, &hints, &result);
    if (err) {
        printf("getaddrinfo failed: %d\n", err);
        return false;
    }
    SOCKET connect_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (connect_socket == INVALID_SOCKET) {
        printf("socket failed!\n");
        freeaddrinfo(result);
        return false;
    }
    err = connect(connect_socket, result->ai_addr, (int)result->ai_addrlen);
    // char ipv4[INET_ADDRSTRLEN];
    // inet_ntop(AF_INET, result->ai_addr, ipv4, INET_ADDRSTRLEN);
    freeaddrinfo(result);
    if (err == SOCKET_ERROR) {
        printf("connect failed: %d\n", err);
        // printf("to address = %s\n", ipv4);
        closesocket(connect_socket);
        return false;
    }
    printf("Connect successful!\n");
    this->connect_socket = connect_socket;
    return true;
}

bool ClientManager::UDPConnect(char* host, char* port) {
    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    int err = getaddrinfo(host, port, &hints, &result);
    if (err) {
        printf("getaddrinfo failed: %d\n", err);
        return false;
    }
    SOCKET connect_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (connect_socket == INVALID_SOCKET) {
        printf("socket failed!\n");
        freeaddrinfo(result);
        return false;
    }
    char ipv4[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, result->ai_addr, ipv4, INET_ADDRSTRLEN);
    this->server_address = result;
    printf("Socket created for UDP communication!\n");
    this->connect_socket = connect_socket;
    return true;
}

bool ClientManager::Connect(char* host, char* port, const char* type) {
    if (strcmp(type, "TCP") == 0) return this->TCPConnect(host, port);
    else if (strcmp(type, "UDP") == 0) return this->UDPConnect(host, port);
    else { printf("type error!\n"); return false; }
}

void ClientManager::Close() {
    closesocket(this->connect_socket);
    freeaddrinfo(this->server_address);
    connect_socket = 0;
    this->server_address = nullptr;
}

bool ClientManager::sendData(char* data, int size) {
    SOCKET connect_socket = this->connect_socket;
    struct addrinfo* server_address = this->server_address;
    if (server_address == NULL) {
        int bytesSend = send(connect_socket, data, size, 0);
        // printf("TCP bytesSend = %d\n", bytesSend);
        return bytesSend;
    }
    int bytesSend = sendto(connect_socket, data, size, 0, server_address->ai_addr, server_address->ai_addrlen);
    // printf("UDP bytesSend = %d\n", bytesSend);
    return bytesSend;
}

bool broadcastMessage(char* port, char* message, int size, int host = INADDR_BROADCAST) {
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == -1) {
        printf("Socket creation failed");
        return false;
    }

    int broadcastEnable = 1;
    if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcastEnable, sizeof(broadcastEnable)) == SOCKET_ERROR) {
        printf("Failed to enable broadcastMessage\n");
        closesocket(udpSocket);
        return false;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(port));
    serverAddr.sin_addr.s_addr = host;

    bool result = (sendto(udpSocket, message, size, 0, (sockaddr*)&serverAddr, sizeof(serverAddr)) != SOCKET_ERROR);

    closesocket(udpSocket);

    return result;
}
