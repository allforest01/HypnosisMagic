#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define WINDOWS
    // #ifndef WIN32_LEAN_AND_MEAN
    // #define WIN32_LEAN_AND_MEAN
    // #endif
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
    void ServClient(SOCKET, bool);
    // void NewThread(SOCKET);
    void (*Services)(SOCKET, char[], int);
    struct addrinfo *server_address;

public:
    static EasySocket& getInstance() {
        static EasySocket instance;
        return instance;
    }

    EasySocket();
    ~EasySocket();
    int CreateServer(char*, const char*);
    // void FreeServer();
    SOCKET ConnectTo(char*, char*, const char*);
    bool SendData(SOCKET, void*, int);
    void setServices(void (*)(SOCKET, char[], int));
};
