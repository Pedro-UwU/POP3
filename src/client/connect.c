/**
 * MIT License - 2023
 * Copyright 2023 - Lopez Guzman, Zahnd
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the “Software”), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#elif _POSIX_C_SOURCE < 200112L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#include <unistd.h> // close
#include <string.h> // memset
#include <sys/types.h> // getaddrinfo
#include <sys/socket.h> // getaddrinfo, connect
#include <netdb.h> // getaddrinfo, addrinfo

#include <client/connect.h>

int connection_close(int socket)
{
        if (socket >= 0)
                return close(socket);

        return 0;
}

int connection_open(args_t *data)
{
        struct addrinfo ainfo;

        memset(&ainfo, 0, sizeof(struct addrinfo));

        ainfo.ai_family = AF_UNSPEC; // IPv4 or IPv6
        ainfo.ai_socktype = SOCK_STREAM;
        ainfo.ai_protocol = IPPROTO_TCP;

        struct addrinfo *sv_addr;
        if (0 != getaddrinfo(data->monitor.ip, data->monitor.port_s, &ainfo, &sv_addr)) {
                return -1;
        }

        int fd = -1;

        for (struct addrinfo *addr = sv_addr; addr != NULL && fd < 0; addr = addr->ai_next) {
                fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
                if (fd >= 0 && 0 != connect(fd, addr->ai_addr, addr->ai_addrlen)) {
                        close(fd);
                        fd = -1;
                }
        }

        freeaddrinfo(sv_addr);
        return fd;
}
