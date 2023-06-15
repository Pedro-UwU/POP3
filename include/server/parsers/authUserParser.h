#ifndef AUTH_USER_PARSER_H
#define AUTH_USER_PARSER_H
#define MAX_USER_NAME 40 // RFC Defined
#define MAX_CMD_LEN 5

#include <server/parser.h>

typedef struct {
    parser_t * parser;
    size_t state_id;

    char uname[MAX_USER_NAME];
    char cmd[MAX_CMD_LEN];

    size_t total_cmd;
    size_t total_uname;
} auth_user_parser_t;

void conf_auth_user_parser(void);
void free_auth_user_parser_conf(void);
void init_auth_user_parser(auth_user_parser_t* parser);

#endif // !AUTH_USER_PARSER_H
