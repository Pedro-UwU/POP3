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
