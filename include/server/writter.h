#ifndef WRITTER_H
#define WRITTER_H

#include <stddef.h> // ssize_t
#include <server/pop3.h> // selector_key

unsigned write_greeting(struct selector_key *key);

/**
 * Send msg to client
 *
 * Returns bytes sent or -1 on error. Sets client_data err_code on POP3 error.
 */
ssize_t write_msg(struct selector_key *key, const char *msg);

#endif // !WRITTER_H
