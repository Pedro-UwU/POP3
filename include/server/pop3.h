#ifndef POP3_H
#define POP3_H
#include <sys/socket.h>
#include <stdbool.h>
#include <stdint.h>
#include <pop3def.h>
#include <server/buffer.h>
#include <server/stm.h>
#include <server/parsers/authUserParser.h>
#include <server/parsers/authPassParser.h>

#define GET_DATA(x) ((client_data*)(x)->data)
//32 kB of buffer
#define BUFFER_SIZE 32768

typedef struct client_data { // Add more items as we need them
    struct state_machine stm;
    union { // Parsers 
        auth_user_parser_t auth_user_parser;
        auth_pass_parser_t auth_pass_parser;
    } parser;

    struct sockaddr_storage client_address;
    bool closed;
    int client_fd;
    struct buffer write_buffer_client;
    struct buffer read_buffer_client;

    uint8_t read_buffer_data[BUFFER_SIZE];
    uint8_t write_buffer_data[BUFFER_SIZE];
    uint8_t user[MAX_ARG_LEN + 1]; // NULL terminated username

} client_data;

enum pop3_states {
    GREETING_WRITE = 0,
    AUTH_USER_READ,
    AUTH_USER_WRITE,
    AUTH_PASS_READ,
    AUTH_PASS_WRITE,
    //AUTH_WRITE,
    /* ... */
    DONE,
    ERROR_POP3
};

void pop3AcceptPassive(struct selector_key* key);

#endif // !POP3_H
