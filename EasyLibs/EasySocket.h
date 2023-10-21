#pragma once

#include <stdio.h>
#include <string>
#include <functional>

#define MAX_BYTES 8192

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define WINDOWS
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <unistd.h>
    typedef uint SOCKET;
    #define INVALID_SOCKET	(SOCKET)(~0)
    #define SOCKET_ERROR	(-1)
    #define closesocket close
#endif

void initEasySocket();
void cleanEasySocket();

class EasyServer {
private:
    SOCKET listen_socket;
    std::function<void(SOCKET, char[], int)> services;
public:
    EasyServer(): listen_socket(0), services(nullptr) {}
    EasyServer(char*, const char*);
    ~EasyServer();
    void setServices(std::function<void(SOCKET, char[], int)>);
    void TCPReceive();
    void UDPReceive();
};

class EasyClient {
private:
    SOCKET connect_socket;
    struct addrinfo *server_address;
public:
    EasyClient(): server_address(nullptr) {}
    EasyClient(char*, char*, const char*);
    ~EasyClient();
    bool sendData(char*, int);
};
