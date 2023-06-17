#ifndef GREETING
#define GREETING

#include "server/buffer.h"
#include <server/states/greeting.h>
#include <server/selector.h>
#include <server/pop3.h>
#include <utils/logger.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <string.h>

// Static strlen of msg to avoid rerunning the funcion everytime. Should be changed if the greeting_msg changes
#define MSG_LEN 19

static uint8_t *greeting_msg = (uint8_t *)"+OK server ready\r\n";

unsigned greeting_write(struct selector_key *key)
{
        log(DEBUG, "Entering greeting_write at fd: %d", key->fd);
        size_t written;
        written = send(key->fd, greeting_msg, MSG_LEN, MSG_NOSIGNAL);

        if (written != MSG_LEN) {
                log(ERROR,
                    "Greeting message returned strange written bytes: %d",
                    (int)written);
                return ERROR_POP3;
        }

        selector_set_interest_key(key, OP_READ);
        log(INFO, "Greeting sent to fd %d", key->fd);
        return AUTH_USER_READ;
}
#endif /* ifndef GREETING */
