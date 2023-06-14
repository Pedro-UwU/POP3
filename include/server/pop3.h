#ifndef POP3_H
#define POP3_H
#include <server/stm.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <stdint.h>
#include <server/buffer.h>

#define GET_DATA(x) ((client_data*)(x)->data)
//32 kB of buffer
#define BUFFER_SIZE 32768

typedef struct client_data { // Add more items as we need them
    struct state_machine stm;
    union { // Parsers 
    } parser;

    struct sockaddr_storage client_address;
    bool closed;
    int client_fd;

    uint8_t client_buffer_data[BUFFER_SIZE];
    struct buffer client_buffer;
    
} client_data;

enum pop3_states {
    GREETING_WRITE = 0,
    //AUTH_READ,
    //AUTH_WRITE,
    /* ... */
    ERROR_POP3
};

void pop3AcceptPassive(struct selector_key* key);

#endif // !POP3_H
