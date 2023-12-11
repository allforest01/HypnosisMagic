#pragma once

#include <stdio.h>
#include <string>
#include <functional>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
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

void initSocketManager();
void cleanSocketManager();

class ServerSocketManager {
private:
    SOCKET listen_socket;
    bool isTCPServer;
    std::function<void(SOCKET, char[], int, char[])> service;
public:
    struct sockaddr_in client_address;
    ServerSocketManager(): listen_socket(0), isTCPServer(false), service(nullptr) {}
    void TCPListen(char*);
    void UDPListen(char*);
    void Listen(char*, const char*);
    void Close();
    void setService(std::function<void(SOCKET, char[], int, char[])>);
    void TCPReceive(int);
    void UDPReceive(int);
    void receiveData(int);
};

class ClientSocketManager {
private:
    SOCKET connect_socket;
    struct addrinfo *server_address;
public:
    ClientSocketManager(): connect_socket(0), server_address(nullptr) {}
    bool TCPConnect(char*, char*);
    bool UDPConnect(char*, char*);
    bool Connect(char*, char*, const char*);
    void Close();
    bool sendData(char*, int);
};

bool broadcastMessage(char*, char*, int, int host);