#ifndef AUTH_USER_PARSER_H
#define AUTH_USER_PARSER_H
#include <stdbool.h>
#define MAX_USER_NAME 40 // RFC Defined
#define MAX_CMD_LEN 5

#include <server/parser.h>
#include <server/buffer.h>

typedef struct {
    parser_t * parser;
    size_t state_id;

    char uname[MAX_USER_NAME];
    char cmd[MAX_CMD_LEN];

    size_t total_cmd;
    size_t total_uname;
    bool ended;
} auth_user_parser_t;

void conf_auth_user_parser(void);
void free_auth_user_parser_conf(void);
void init_auth_user_parser(auth_user_parser_t* parser);
int auth_user_parse(struct selector_key* key, auth_user_parser_t* auth_parser, struct buffer* buffer);

#endif // !AUTH_USER_PARSER_H
