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

#define MAX_BYTES 1440

void initEasySocket();
void cleanEasySocket();

class EasyServer {
private:
    SOCKET listen_socket;
    std::function<void(SOCKET, char[], int, char[])> service;
public:
    EasyServer(): listen_socket(0), service(nullptr) {}
    void elisten(char*, const char*);
    void eclose();
    void setService(std::function<void(SOCKET, char[], int, char[])>);
    void TCPReceive();
    void UDPReceive();
};

class EasyClient {
private:
    SOCKET connect_socket;
    struct addrinfo *server_address;
public:
    EasyClient(): connect_socket(0), server_address(nullptr) {}
    void econnect(char*, char*, const char*);
    void eclose();
    bool sendData(char*, int);
};
