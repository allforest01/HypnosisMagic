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
    std::function<void(SOCKET, char[], int, char[])> handleReceive;
public:
    struct sockaddr_in client_address;
    ServerSocketManager(): listen_socket(0), isTCPServer(false), handleReceive(nullptr) {}
    ~ServerSocketManager();
    bool TCPListen(char*);
    bool UDPListen(char*);
    bool Listen(char*, const char*);
    void Close();
    void setReceiveCallback(std::function<void(SOCKET, char[], int, char[])>);
    int TCPReceive(int);
    int UDPReceive(int);
    int receiveData(int);
};

class ClientSocketManager {
private:
    SOCKET connect_socket;
    struct addrinfo *server_address;
public:
    ClientSocketManager(): connect_socket(0), server_address(nullptr) {}
    ~ClientSocketManager();
    bool TCPConnect(char*, char*);
    bool UDPConnect(char*, char*);
    bool Connect(char*, char*, const char*);
    void Close();
    int sendData(char*, int);
};

bool broadcastMessage(char*, char*, int, int host);