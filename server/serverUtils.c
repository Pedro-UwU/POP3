#ifndef SERVER_UTILS
#define SERVER_UTILS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include "../shared/util.h"
#include "../shared/logger.h"
#include "serverUtils.h"
#define MAX_BUFFER 512

static char addrBuffer[MAX_BUFFER];

int createTCPSocketServer(const char* service) {
    //Construct the server address structure
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; //For IPv4 and IPv6
    hints.ai_flags = AI_PASSIVE; // Because is a server socket
    hints.ai_socktype = SOCK_STREAM; // TCP Stream
    hints.ai_protocol = IPPROTO_TCP; // TCP

    struct addrinfo* addrArray;
    int addrReturnValue = getaddrinfo(NULL, service, &hints, &addrArray);
    if (addrReturnValue == -1) {
        log(FATAL, "Error getting address info: %s", strerror(errno));        
    }

    int servSocket = -1;
    for (struct addrinfo * addr = addrArray; addr != NULL && servSocket == -1; addr = addr->ai_next) {
        servSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (servSocket < 0) {
            log(DEBUG, "Can't create socket on %s: %s", printAddressPort(addr, addrBuffer), strerror(errno));
            continue;
        }

        //Bind to all the address and set socket to listen
        if ((bind(servSocket, addr->ai_addr, addr->ai_addrlen) == 0) && (listen(servSocket, MAX_PENDING) == 0)) {
            struct sockaddr_storage localAddr;
            socklen_t addrSize = sizeof(localAddr);
            if (getsockname(servSocket, (struct sockaddr *) &localAddr, &addrSize) >= 0) {
                printSocketAddress((struct sockaddr *) &localAddr, addrBuffer);
                log(INFO, "Binding to %s", addrBuffer);
            }
        } else {
            log(DEBUG, "Can't bind %s", strerror(errno));
            close(servSocket);
            servSocket = -1;
        }
    }
    freeaddrinfo(addrArray);
    return servSocket;
}

#endif 
