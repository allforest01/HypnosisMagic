#include "socket_manager.h"
#include <chrono>
#include <thread>

// #define DEBUG

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

bool ServerSocketManager::TCPListen(char* port) {
    isTCPServer = true;

    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    int err = getaddrinfo(NULL, port, &hints, &result);
    if (err) {
        // printf("getaddrinfo failed: %d\n", err);
        return false;
    }
    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("tcp listen socket failed!\n");
        freeaddrinfo(result);
        return false;
    }

    err = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);
    if (err) {
        perror("bind failed");
        closesocket(ListenSocket);
        return false;
    }
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen failed!");
        closesocket(ListenSocket);
        return false;
    }
    printf("Listening port %s...\n", port);
    SOCKET ClientSocket;
    socklen_t client_address_size = sizeof(this->client_address);
    ClientSocket = accept(ListenSocket, (sockaddr*)(&this->client_address), &client_address_size);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed!\n");
        closesocket(ListenSocket);
        return false;
    }
    this->listen_socket = ClientSocket;
    // #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    //     int timeout = 60000; // 1 seconds
    // #else
    //     struct timeval timeout;
    //     timeout.tv_sec = 60; // 1 seconds
    //     timeout.tv_usec = 0;
    // #endif
    int flag = 1;
    if (setsockopt(this->listen_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int)) < 0) {
        printf("tcp listen setsockopt failed!\n");
        return false;
    }
    printf("Client connected!\n");
    return true;
}

bool ServerSocketManager::UDPListen(char* port) {
    isTCPServer = false;

    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;
    int err = getaddrinfo(NULL, port, &hints, &result);
    if (err) {
        // printf("getaddrinfo failed: %d\n", err);
        return false;
    }
    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed!\n");
        freeaddrinfo(result);
        return false;
    }
    err = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);
    if (err) {
        perror("bind failed");
        closesocket(ListenSocket);
        return false;
    }
    int bufferSize = 1024 * 1024; // set your desired buffer size
    if (setsockopt(ListenSocket, SOL_SOCKET, SO_RCVBUF, (char*)&bufferSize, sizeof(bufferSize)) < 0) {
        printf("udp listen setsockopt failed!\n");
        return false;
    }
    this->listen_socket = ListenSocket;
    printf("UDP Socket is created!\n");
    return true;
}

bool ServerSocketManager::Listen(char* port, const char* type) {
    if (strcmp(type, "TCP") == 0) return this->TCPListen(port);
    else if (strcmp(type, "UDP") == 0) return this->UDPListen(port);
    else { printf("type error: %s\n", type); return false; }
}

void ServerSocketManager::setReceiveCallback(std::function<void(SOCKET, char[], int, char[])> handleReceive) {
    this->handleReceive = handleReceive;
}

int ServerSocketManager::TCPReceive(int max_bytes) {
    // printf("TCPPPPPPPPPPPPPPP\n");
    SOCKET listen_socket = this->listen_socket;
    char* buffer = new char[max_bytes];
    // printf("max bytes = %d\n", max_bytes);
    int bytesRead = recv(listen_socket, buffer, max_bytes, 0);
    // printf("RECV = %d\n", max_bytes);
    if (bytesRead == -1) {
        delete[] buffer;
        return -1;
    }
    this->handleReceive(listen_socket, buffer, bytesRead, NULL);
    delete[] buffer;
    #ifdef DEBUG
    printf("bytesRead = %d\n", bytesRead);
    #endif
    return bytesRead;
}

int ServerSocketManager::UDPReceive(int max_bytes) {
    SOCKET listen_socket = this->listen_socket;
    char* buffer = new char[max_bytes];
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int bytesRead = recvfrom(listen_socket, buffer, max_bytes, 0, (sockaddr*)&client_address, &client_address_size);
    char ipv4[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_address.sin_addr), ipv4, INET_ADDRSTRLEN);
    // printf("ipv4 = %s\n", ipv4);
    if (bytesRead == -1) {
        delete[] buffer;
        return -1;
    }
    this->handleReceive(listen_socket, buffer, bytesRead, ipv4);
    delete[] buffer;
    #ifdef DEBUG
    printf("bytesRead = %d\n", bytesRead);
    #endif
    return bytesRead;
}

int ServerSocketManager::receiveData(int max_bytes) {
    if (this->isTCPServer) return TCPReceive(max_bytes);
    else return UDPReceive(max_bytes);
}

void ServerSocketManager::Close() {
    if (this->listen_socket) {
        closesocket(this->listen_socket);
        this->listen_socket = 0;
        this->handleReceive = nullptr;
    }
}

ServerSocketManager::~ServerSocketManager() {
    this->Close();
}

bool ClientSocketManager::TCPConnect(char* host, char* port) {
    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int err = getaddrinfo(host, port, &hints, &result);
    if (err) {
        // printf("getaddrinfo failed: %d\n", err);
        return false;
    }
    SOCKET connect_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (connect_socket == INVALID_SOCKET) {
        printf("tcp connect socket failed!\n");
        freeaddrinfo(result);
        return false;
    }
    err = connect(connect_socket, result->ai_addr, (int)result->ai_addrlen);
    // char ipv4[INET_ADDRSTRLEN];
    // inet_ntop(AF_INET, result->ai_addr, ipv4, INET_ADDRSTRLEN);
    freeaddrinfo(result);
    if (err == SOCKET_ERROR) {
        printf("(%d) ", err); fflush(stdout);
        closesocket(connect_socket);
        return false;
    }
    this->connect_socket = connect_socket;
    printf("Connect successful!\n");
    return true;
}

bool ClientSocketManager::UDPConnect(char* host, char* port) {
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
    int sendBufferSize = 1024 * 1024; // set your desired send buffer size
    if (setsockopt(connect_socket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, sizeof(sendBufferSize)) < 0) {
        printf("udp connect setsockopt failed!\n");
        closesocket(connect_socket);
        freeaddrinfo(result);
        return false;
    }
    char ipv4[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, result->ai_addr, ipv4, INET_ADDRSTRLEN);
    this->server_address = result;
    this->connect_socket = connect_socket;
    printf("Socket created for %s\n", ipv4);
    return true;
}

bool ClientSocketManager::Connect(char* host, char* port, const char* type) {
    if (strcmp(type, "TCP") == 0) return this->TCPConnect(host, port);
    else if (strcmp(type, "UDP") == 0) return this->UDPConnect(host, port);
    else { printf("type error: %s\n", type); return false; }
}

int ClientSocketManager::sendData(char* data, int size) {
    SOCKET connect_socket = this->connect_socket;
    struct addrinfo* server_address = this->server_address;
    if (server_address == NULL) {
        int bytesSend = send(connect_socket, data, size, 0);
        #ifdef DEBUG
        printf("TCP bytesSend = %d\n", bytesSend);
        #endif
        return bytesSend;
    }
    int bytesSend = sendto(connect_socket, data, size, 0, server_address->ai_addr, server_address->ai_addrlen);
    #ifdef DEBUG
    printf("UDP bytesSend = %d\n", bytesSend);
    #endif
    return bytesSend;
}

void ClientSocketManager::Close() {
    if (this->connect_socket) {
        closesocket(this->connect_socket);
        freeaddrinfo(this->server_address);
        this->connect_socket = 0;
        this->server_address = nullptr;
    }
}

ClientSocketManager::~ClientSocketManager() {
    this->Close();
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
