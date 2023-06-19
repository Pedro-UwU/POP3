#ifndef TRANS_PARSER_H
#define TRANS_PARSER_H
#include <server/buffer.h>
#include <server/parser.h>
#include <server/selector.h>
#include <stdbool.h>
#include <stddef.h>
#include <pop3def.h>
#include <utils/maildir.h>

typedef struct {
        parser_t *parser;
        bool ended;
        bool quit;
        size_t state_id;
        size_t total_cmd;
        size_t total_arg;
        char cmd[MAX_CMD_LEN + 1];
        char arg[(MAX_ARG_LEN * MAX_ARGS) + 1];
        size_t arg_read;
        unsigned err_value;
        user_maildir_t *maildir;
} trans_parser_t;

void conf_trans_parser(void);
void free_trans_parser(void);
void init_trans_parser(trans_parser_t *parser, const char *user);
int trans_parse(struct selector_key *key, trans_parser_t *trans_parser_t, struct buffer *buffer);

#endif // !AUTH_PARSER_H
