#ifndef POP3_DEF
#define POP3_DEF

#define MAX_CMD_LEN 5
#define MAX_ARG_LEN 40
#define USER_CMD "USER"
#define QUIT_CMD "QUIT"

enum POP3ErrCodes {
    NO_ERROR = 0,
    LONG_PARAM,
    LONG_CMD,
    INVALID_CMD,
    INVALID_USER,
    INVALID_CHAR,
    WRONG_PASSWORD,
    /* ... */
    UNKNOWN_ERROR,
};

#endif // !POP3_DEF