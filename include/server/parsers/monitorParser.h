#ifndef MONITOR_PARSER_H
#define MONITOR_PARSER_H
#include <server/buffer.h>
#include <server/parser.h>
#include <server/selector.h>
#include <monitordef.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
        parser_t *parser;
        bool ended;
        bool quit;
        size_t state_id;
        size_t total_cmd;
        size_t total_arg;
        char cmd[MONITOR_MAX_CMD_LEN + 1];
        char arg[MONITOR_MAX_ARG_LEN + 1];
        unsigned err_value;
} monitor_parser_t;

void conf_monitor_parser(void);
void free_monitor_parser(void);
void init_monitor_parser(monitor_parser_t *parser);
int monitor_parse(struct selector_key *key, monitor_parser_t *monitor_parser_t,
                  struct buffer *buffer);

#endif // !AUTH_PARSER_H
