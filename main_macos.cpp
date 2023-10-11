#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main() {
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
}
