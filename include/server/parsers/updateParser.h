#ifndef UPDATE_PARSER_H
#define UPDATE_PARSER_H

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
        unsigned err_value;
} update_parser_t;

void conf_update_parser(void);
void free_update_parser(void);
void init_update_parser(update_parser_t *parser);
int update_parse(struct selector_key *key, update_parser_t *update_parser_t, struct buffer *buffer);

#endif // !UPDATE_PARSER_H
