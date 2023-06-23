#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>

typedef struct {
        bool version;
        bool debug;

        struct {
                unsigned port;
                char *port_s;
                char *ip;
        } monitor;

        struct {
                unsigned port;
                char *port_s;
                char *ip;
        } server;

        char *user;
        char *pass;

        char *ext_cmd;
} args_t;

// Returns optind. On error it returns < 0
int parse_args(int argc, char **argv, args_t *args);

#endif // !ARGS_H
