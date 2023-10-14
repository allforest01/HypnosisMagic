#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    // #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #if 1
    typedef uint	SOCKET;
    #else
    typedef int		SOCKET;
    #endif
    #define INVALID_SOCKET	(SOCKET)(~0)
    #define SOCKET_ERROR	(-1)
#endif

class EasySocket {
private:
    void ServClient(SOCKET);
    // void NewThread(SOCKET);
public:
    EasySocket();
    ~EasySocket();
    int CreateServer(char*);
    // void FreeServer();
    SOCKET ConnectTo(char*, char*);
    bool SendData(SOCKET, void*, int);
};
