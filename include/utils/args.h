#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>

typedef struct {
        bool version;

        struct {
                unsigned port;
                char *ip;
        } monitor;

        struct {
                unsigned port;
                char *ip;
        } server;

        char *user;
        char *pass;
} args_t;

// Returns optind. On error it returns < 0
int parse_args(int argc, char **argv, args_t *args);

#endif // !ARGS_H
