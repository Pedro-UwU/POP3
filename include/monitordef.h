#ifndef MONITOR_DEF_H
#define MONITOR_DEF_H

#define MONITOR_BUFFER_SIZE 1024
#define MONITOR_MAX_ARG_LEN 40
#define MONITOR_MAX_CMD_LEN 40

enum monitor_error_codes {
    NO_ERROR = 0,
    INVALID_CHAR,
    NOT_LOGGED,
    WRONG_LOGIN,
    INVALID_USER,
    UNKNOWN_ERROR
};

#endif // !MONITOR_DEF_H
