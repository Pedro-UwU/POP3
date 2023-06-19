#ifndef POP3_DEF
#define POP3_DEF

#define MAX_CMD_LEN 5
#define MAX_ARG_LEN 40
#define MAX_ARGS 5
#define MAX_RSP_LEN 512
#define MAX_BYTES_SEND 256
#define USER_CMD "USER"
#define QUIT_CMD "QUIT"

enum POP3ErrCodes {
        NO_ERROR = 0,
        LONG_PARAM,
        LONG_CMD,
        INVALID_CMD,
        INVALID_ARG,
        INVALID_USER,
        INVALID_CHAR,
        WRONG_PASSWORD,
        WRONG_TIME_PASS_CMD,
        USER_ALREADY_ONLINE,
        /* ... */
        UNKNOWN_ERROR,
};

#endif // !POP3_DEF
