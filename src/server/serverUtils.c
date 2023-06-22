#ifndef SERVER_UTILS
#define SERVER_UTILS

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#elif _POSIX_C_SOURCE < 200112L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h> // getaddrinfo
#include <sys/socket.h> // getaddrinfo, connect
#include <netdb.h> // getaddrinfo, addrinfo
#include <utils/logger.h>
#include <server/serverUtils.h>

int createTCPSocketServer(char *port, int protocol)
{
        log(DEBUG, "Port: %s", port);

        int fd = -1;
        struct addrinfo ainfo;
        struct addrinfo *sv_addr = NULL;

        memset(&ainfo, 0, sizeof(struct addrinfo));

        ainfo.ai_family = protocol;
        ainfo.ai_socktype = SOCK_STREAM;
        ainfo.ai_protocol = IPPROTO_TCP;
        ainfo.ai_flags = AI_PASSIVE;

        if (0 != getaddrinfo(NULL, port, &ainfo, &sv_addr)) {
                return -1;
        }

        for (struct addrinfo *addr = sv_addr; addr != NULL && fd < 0; addr = addr->ai_next) {
                fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
                if (fd < 0) {
                        log(ERROR, "[serverUtils][createTCPSocketServer] Unable to create socket");
                        continue;
                }

                setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
                if (protocol == AF_INET6) {
                        setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &(int){ 1 }, sizeof(int));
                }

                if (bind(fd, addr->ai_addr, addr->ai_addrlen) < 0) {
                        log(ERROR, "[serverUtils][createTCPSocketServer] Unable to bind socket %d",
                            fd);
                        close(fd);
                        continue;
                }
                log(DEBUG, "[serverUtils][createTCPSocketServer] Socket %d bounded to port %s", fd,
                    port);
        }

        freeaddrinfo(sv_addr);

        if (listen(fd, atoi(port))) {
                log(ERROR,
                    "[serverUtils][createTCPSocketServer] Unable to listen to port %s with socket %d",
                    port, fd);
                close(fd);
                return -1;
        }

        log(DEBUG, "[serverUtils][createTCPSocketServer] Socket %d listening", fd);

        return fd;
}

#endif
