#pragma once

#include <stdio.h>
#include <string>
#include <functional>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <unistd.h>
    typedef uint SOCKET;
    #define INVALID_SOCKET	(SOCKET)(~0)
    #define SOCKET_ERROR	(-1)
    #define closesocket close
#endif

void initHypnoSocket();
void cleanHypnoSocket();

class HypnoServer {
private:
    SOCKET listen_socket;
    std::function<void(SOCKET, char[], int, char[])> service;
public:
    struct sockaddr_in client_address;
    HypnoServer(): listen_socket(0), service(nullptr) {}
    void hypnoListen(char*, const char*);
    void hypnoClose();
    void setService(std::function<void(SOCKET, char[], int, char[])>);
    void TCPReceive(int max_bytes);
    void UDPReceive(int max_bytes);
};

class HypnoClient {
private:
    SOCKET connect_socket;
    struct addrinfo *server_address;
public:
    HypnoClient(): connect_socket(0), server_address(nullptr) {}
    bool hypnoConnect(char*, char*, const char*);
    void hypnoClose();
    bool sendData(char*, int);
};

bool broadcastMessage(char*, char*, int, int host);