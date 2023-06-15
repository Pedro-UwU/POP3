#ifndef POP3_H
#define POP3_H
#include <server/stm.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <stdint.h>
#include <server/buffer.h>
#include <server/parsers/authUserParser.h>

#define GET_DATA(x) ((client_data*)(x)->data)
//32 kB of buffer
#define BUFFER_SIZE 32768

typedef struct client_data { // Add more items as we need them
    struct state_machine stm;
    union { // Parsers 
        auth_user_parser_t auth_user_parser;
    } parser;

    struct sockaddr_storage client_address;
    bool closed;
    int client_fd;

    uint8_t read_buffer_data[BUFFER_SIZE];
    struct buffer read_buffer_client;

    uint8_t write_buffer_data[BUFFER_SIZE];
    struct buffer write_buffer_client;
} client_data;

enum pop3_states {
    GREETING_WRITE = 0,
    AUTH_USER_READ,
    //AUTH_WRITE,
    /* ... */
    DONE,
    ERROR_POP3
};

void pop3AcceptPassive(struct selector_key* key);

#endif // !POP3_H
