#ifndef CLIENT_CONNECT_H
#define CLIENT_CONNECT_H

#include <utils/args.h>

int connection_close(int socket);
int connection_open(args_t *data);

#endif // !CLIENT_CONNECT_H
