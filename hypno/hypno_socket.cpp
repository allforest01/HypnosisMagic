#include "hypno_socket.h"

void initHypnoSocket() {
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
        printf("WSAStartup successful!\n");
    #endif
}

void cleanHypnoSocket() {   
    #ifdef WINDOWS
        WSACleanup();
    #endif
}

void HypnoServer::hypnoListen(char* port, const char* type) {
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
        socklen_t client_address_size = sizeof(this->client_address);
        ClientSocket = accept(ListenSocket, (sockaddr*)(&this->client_address), &client_address_size);
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
    #ifdef WINDOWS
        int timeout = 500; // 500 miliseconds
    #else
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000; // 500 miliseconds
    #endif
    if (strcmp(type, "TCP") == 0) {
        int flag = 1;
        if (setsockopt(this->listen_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int)) < 0) {
            printf("setsockopt failed!\n");
            return;
        }
    }
    else if (strcmp(type, "UDP") == 0) {
        if (setsockopt(this->listen_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
            printf("setsockopt failed!\n");
            return;
        }
    }
}

void HypnoServer::hypnoClose() {
    closesocket(this->listen_socket);
    listen_socket = 0;
    this->service = nullptr;
}

void HypnoServer::setService(std::function<void(SOCKET, char[], int, char[])> service) {
    this->service = service;
}

void HypnoServer::TCPReceive(int max_bytes) {
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

void HypnoServer::UDPReceive(int max_bytes) {
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

bool HypnoClient::hypnoConnect(char* host, char* port, const char* type) {
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
        return false;
    }
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
    if (strcmp(type, "TCP") == 0) {
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
    }
    else if (strcmp(type, "UDP") == 0) {
        char ipv4[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, result->ai_addr, ipv4, INET_ADDRSTRLEN);
        this->server_address = result;
        printf("Socket created for UDP communication!\n");
    }
    this->connect_socket = connect_socket;
    return true;
}

void HypnoClient::hypnoClose() {
    closesocket(this->connect_socket);
    freeaddrinfo(this->server_address);
    connect_socket = 0;
    this->server_address = nullptr;
}

bool HypnoClient::sendData(char* data, int size) {
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