#ifndef CLIENT_SPEC_H
#define CLIENT_SPEC_H

#define MONITOR_TERMINATOR "\r\n\r\n"
#define MONITOR_TERMINATOR_LEN 4

#define MONITOR_SEPARATOR " "

#define MONITOR_MAX_ARG_LEN 40

#define MONITOR_ANSWER_OK "OwO"
#define MONITOR_ANSWER_ERR "UwU"

typedef enum {
        QUIT,
        LOGIN,
        GET_USERS,
        GET_USER,
        GET_CURR_CONN,
        GET_TOTAL_CONN,
        GET_SENT_BYTES,
        ADD_USER,
        POPULATE_USER,
        DELETE_USER,
        COMMANDS,
        N_CMDS,
        CMD_INVALID,
} monitor_cmd;

static const int monitor_cmds_args[] = {
        0, // QUIT
        2, // LOGIN <user> <password>
        0, // GET_USERS
        1, // GET_USER <username>
        0, // GET_CURR_CONN
        0, // GET_TOTAL_CONN
        0, // GET_SENT_BYTES
        2, // ADD_USER <username> <password>
        1, // POPULATE_USER <username>
        1, // DELETE_USER <username>
        0, // COMMANDS
};

#endif // !CLIENT_SPEC_H
