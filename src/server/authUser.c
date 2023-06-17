#include "pop3def.h"
#include "server/buffer.h"
#include "utils/logger.h"
#include <server/pop3.h>
#include <server/selector.h>
#include <server/parsers/authUserParser.h>
#include <server/authUser.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

void initAuthUser(const unsigned state, struct selector_key *key)
{
        log(DEBUG, "Initializing new Auth User Parser");
        client_data *data = GET_DATA(key);
        init_auth_user_parser(&data->parser.auth_user_parser);
}

unsigned auth_user_read(struct selector_key *key)
{
        if (key == NULL) {
                log(ERROR, "auth_user_read NULL key");
                return ERROR_POP3;
        }

        client_data *data = GET_DATA(key);

        size_t read_limit = 0;
        uint8_t *readBuffer = buffer_write_ptr(&data->read_buffer_client, &read_limit);
        size_t read_count = recv(key->fd, readBuffer, read_limit, 0);
        if (read_count > 0)
                log(DEBUG, "auth_user_read read %ld bytes", read_count);
        if (read_count < 0) {
                return ERROR_POP3;
        }
        if (read_count == 0) {
                log(DEBUG, "Socket %d closed connections", key->fd);
                return DONE;
        }

        buffer_write_adv(&data->read_buffer_client, read_count);

        int state = auth_user_parse(key, &data->parser.auth_user_parser, &data->read_buffer_client);

        if (state == -1) {
                return ERROR_POP3;
        }
        if (data->parser.auth_user_parser.ended == true) {
                if (data->parser.auth_user_parser.error_code == 0) {
                        return DONE;
                } else {
                        return ERROR_POP3; // TODO GO TO AUTH_ERROR WITH AN ERROR CODE
                }
        }
        return AUTH_USER_READ;
}

unsigned auth_user_write(struct selector_key *key)
{
        /*
     * if buffer_can_read(write_buffer) // There's something to send in the buffer.
     *     send(msg)
     *     if sent < 0:
     *         ERROR
     *     if sent == 0:
     *         CONNECTION_CLOSED
     *     if sent == len(msg) // If we sent all the message
     *         client_data->is_sending = false // Indicate that we are no sending anymore
     *         return NEXT_STATE
     * else if is_sending:
     *     // Shouldn't get here because the if statement should turn the flag off if all the message was sen't, but por las dudas
     *     is_sending = false;
     *     ret NEXT_STATE
     * else:
     *     // There's nothing in the buffer and it's not sending anything, so I should generate the message
     *     msg = build_msg();
     *     send(msg)
     *     if sent < 0:
     *         ERROR
     *     if sent == 0:
     *         CONNECTION_CLOSED
     *     if sent == len(msg):
     *         // All the msg was sent
     *         ret NEXT_STATE
     *     is_sending = true
     *     ret NEXT_STATE
     *
    */
        client_data *data = GET_DATA(key);
        if (buffer_can_read(&data->write_buffer_client)) {
                ssize_t total_bytes_to_send = 0;
                uint8_t *bytes_to_send =
                        buffer_read_ptr(&data->write_buffer_client, &total_bytes_to_send);
                ssize_t bytes_sent = send(key->fd, bytes_to_send, total_bytes_to_send,
                                          0); // TODO Check flags
                if (bytes_sent < 0) {
                        data->is_sending = false;
                        data->err_code =
                                UNKNOWN_ERROR; // TODO Check if a different Error code is correct
                        return ERROR_POP3;
                }
                if (bytes_sent == 0) {
                        return DONE;
                }
                if (bytes_sent == total_bytes_to_send) {
                        data->is_sending = false;
                        if (data->err_code == NO_ERROR) {
                                return AUTH_PASS_READ;
                        } else {
                                return AUTH_USER_READ; // Ask for the user again
                        }
                } else {
                        return AUTH_USER_WRITE;
                }
        }
}
