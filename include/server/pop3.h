#ifndef POP3_H
#define POP3_H
#include <server/parsers/authParser.h>
#include <server/parsers/transParser.h>
#include <server/parsers/updateParser.h>
#include <server/fileReader.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <stdint.h>
#include <pop3def.h>
#include <server/buffer.h>
#include <server/stm.h>
#include <utils/maildir.h>

#define GET_DATA(x) ((client_data *)(x)->data)
//32 kB of buffer
#define BUFFER_SIZE 32768

typedef struct client_data { // Add more items as we need them
        struct state_machine stm;
        union { // Parsers
                auth_parser_t auth_parser;
                trans_parser_t trans_parser;
                update_parser_t update_parser;
        } parser;

        struct sockaddr_storage client_address;
        bool closed;

        struct {
                bool finished; // Everything sent
                bool file; // Sending a file
                bool multiline; // Should send terminator
                void (*f)(struct selector_key *key); // Get more data to send
        } send;

        unsigned err_code;
        uint8_t read_buffer_data[BUFFER_SIZE];
        uint8_t write_buffer_data[BUFFER_SIZE];
        uint8_t user[MAX_ARG_LEN + 1]; // NULL terminated username
        int client_fd;
        int next_state;
        struct buffer write_buffer_client;
        struct buffer read_buffer_client;

        // RETR command
        struct file_reader_data fr_data;

        // LIST command
        unsigned n_listed;

        // User Maildir
        user_maildir_t maildir;
} client_data;

enum pop3_states {
        GREETING_WRITE = 0,
        AUTH,
        TRANSACTION,
        UPDATE,
        /* ... */
        DONE,
        ERROR_POP3
};

void pop3AcceptPassive(struct selector_key *key);

#endif // !POP3_H
