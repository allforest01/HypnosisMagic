#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
#endif

int main() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    WSADATA wsaData;
    int err = 0;
    if (err = WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        printf("WSAStartup failed: %d\n", &err);
        return 1;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        fprintf(stderr, "Version 2.2 is not available!\n");
        WSACleanup();
        return 2;
    }
#endif

    int status;
    addrinfo hints;
    addrinfo *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo("www.google.com", NULL, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    for (addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            char ipstr[INET_ADDRSTRLEN];
            sockaddr_in* ipv4 = (sockaddr_in*)(p->ai_addr);
            void* addr = &(ipv4->sin_addr);
            inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
            printf("%s\n", ipstr);
        } else {
            char ipstr[INET6_ADDRSTRLEN];
            sockaddr_in6* ipv6 = (sockaddr_in6*)(p->ai_addr);
            void* addr = &(ipv6->sin6_addr);
            inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
            printf("%s\n", ipstr);
        }
    }

    freeaddrinfo(servinfo);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    WSACleanup();
#endif
}
