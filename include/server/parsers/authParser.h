#ifndef AUTH_PARSER_H
#define AUTH_PARSER_H
#include <server/buffer.h>
#include <server/parser.h>
#include <server/selector.h>
#include <stdbool.h>
#include <stddef.h>
#include <pop3def.h>

typedef struct {
        parser_t *parser;
        bool ended;
        bool quit;
        size_t state_id;
        size_t total_cmd;
        size_t total_arg;
        char cmd[MAX_CMD_LEN + 1];
        char arg[MAX_ARG_LEN + 1];
        unsigned err_value;
} auth_parser_t;

void conf_auth_parser(void);
void free_auth_parser(void);
void init_auth_parser(auth_parser_t *parser);
int auth_parse(struct selector_key *key, auth_parser_t *auth_parser_t, struct buffer *buffer);

#endif // !AUTH_PARSER_H
