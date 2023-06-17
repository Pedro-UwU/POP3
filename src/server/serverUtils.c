#ifndef SERVER_UTILS
#define SERVER_UTILS
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include "utils/logger.h"
#include "server/serverUtils.h"

int createTCPSocketServer(unsigned int port)
{
        log(DEBUG, "Port: %d", port);
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);

        const int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket < 0) {
                log(ERROR, "[serverUtils][createTCPSocketServer] Unable to create socket");
                return -1;
        }

        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

        if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                log(ERROR, "[serverUtils][createTCPSocketServer] Unable to bind socket %d",
                    serverSocket);
                close(serverSocket);
                return -1;
        }
        log(DEBUG, "[serverUtils][createTCPSocketServer] Socket %d bounded to port %d",
            serverSocket, port);

        if (listen(serverSocket, port)) {
                log(ERROR,
                    "[serverUtils][createTCPSocketServer] Unable to listen to port %d with socket %d",
                    port, serverSocket);
                close(serverSocket);
                return -1;
        }

        log(DEBUG, "[serverUtils][createTCPSocketServer] Socket %d listening", serverSocket);

        return serverSocket;
}

#endif
