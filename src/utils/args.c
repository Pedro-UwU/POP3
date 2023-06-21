#include <stdbool.h> // bool
#include <stdlib.h> // atol
#include <getopt.h> // getopt, optarg
#include <string.h> // strchr
#include <utils/logger.h>

#include <utils/args.h>

static unsigned parse_port(const char *p);
static int user(args_t *args, char *val);

int parse_args(int argc, char **argv, args_t *args)
{
        if (argv == NULL) {
                log(ERROR, "NULL argv");
                return -2;
        }

        if (args == NULL) {
                log(ERROR, "NULL args");
                return -2;
        }

        memset(args, 0, sizeof(args_t));

        args->version = false;

        args->monitor.port = 60401;
        args->monitor.port_s = "60401";
        args->monitor.ip = "127.0.0.1";

        args->server.port = 60711;
        args->server.port_s = "60711";
        args->server.ip = "127.0.0.1";

        int i = 0;
        while (i < argc) {
                char c = getopt(argc, argv, "hp:l:P:L:u:v");
                if (c == -1)
                        break;

                switch (c) {
                case 'h':
                        return -1;

                case 'p':
                        args->server.port_s = optarg;
                        args->server.port = parse_port(optarg);
                        if (args->server.port == 0) {
                                fprintf(stderr, "Invalid port: %s\n", optarg);
                                return -1;
                        }
                        break;
                case 'l':
                        args->server.ip = optarg;
                        break;
                case 'P':
                        args->monitor.port_s = optarg;
                        args->monitor.port = parse_port(optarg);
                        if (args->monitor.port == 0) {
                                fprintf(stderr, "Invalid port: %s\n", optarg);
                                return -1;
                        }
                        break;
                case 'L':
                        args->monitor.ip = optarg;
                        break;
                case 'u':
                        if (user(args, optarg) != 0) {
                                fprintf(stderr, "Invalid user/password: %s\n", optarg);
                                return -1;
                        }

                        break;
                case 'v':
                        args->version = true;
                        return 0;

                default:
                        fprintf(stderr, "Invalid argument '%c'\n", c);
                        return -1;
                }

                i++;
        }

        return optind;
}

// 0 means error
static unsigned parse_port(const char *p)
{
        if (p == NULL)
                return 0;

        long port = atol(p);
        if (port <= 0)
                port = 0;

        return (unsigned)port;
}

static int user(args_t *args, char *val)
{
        char *c = strchr(val, ':');
        if (c == NULL)
                return 1;

        val[c - val] = '\0';
        args->user = val;
        args->pass = 1 + c;

        if (val == c || *(1 + c) == '\0') {
                return 1;
        }

        return 0;
}
