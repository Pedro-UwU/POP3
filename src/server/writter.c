/**
 * MIT License - 2023
 * Copyright 2023 - Lopez Guzman, Zahnd
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the “Software”), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <server/buffer.h>
#include <server/selector.h>
#include <server/pop3.h>
#include <server/monitor.h>
#include <utils/logger.h>

#include <server/writter.h>

// Static strlen of msg to avoid rerunning the funcion everytime. Should be changed if the greeting_msg changes
#define MSG_LEN 19

static uint8_t *greeting_msg = (uint8_t *)"+OK server ready\r\n";

unsigned write_greeting(struct selector_key *key)
{
        log(DEBUG, "Entering greeting_write at fd: %d", key->fd);
        size_t written;
        written = send(key->fd, greeting_msg, MSG_LEN, MSG_NOSIGNAL);
        monitor_add_sent_bytes(written);

        if (written != MSG_LEN) {
                log(ERROR, "Greeting message returned strange written bytes: %d", (int)written);
                return ERROR_POP3;
        }

        selector_set_interest_key(key, OP_READ);
        log(INFO, "Greeting sent to fd %d", key->fd);
        return AUTH;
}

ssize_t write_msg(struct selector_key *key, const char *msg)
{
        if (key == NULL) {
                log(ERROR, "NULL key");
                return -1;
        }

        if (msg == NULL) {
                log(ERROR, "NULL msg");
                return -1;
        }

        client_data *data = GET_DATA(key);
        buffer *output_buffer = &data->write_buffer_client;

        size_t msg_len = strlen(msg);
        if (buffer_can_write(output_buffer) == false) {
                buffer_compact(output_buffer); // Try compacting
                if (buffer_can_write(output_buffer)) {
                        data->err_code = UNKNOWN_ERROR;
                        return -1;
                }
        }

        size_t write_limit = 0;
        char *out_w_str = (char *)buffer_write_ptr(output_buffer, &write_limit);

        // TODO Check (even tho it's unlikely) if the write_limit is less than the strlen of the msg
        strcpy(out_w_str, msg);
        buffer_write_adv(output_buffer, msg_len);

        size_t read_limit = 0;
        char *out_r_str = (char *)buffer_read_ptr(output_buffer, &read_limit);
        ssize_t sent_bytes = send(key->fd, out_r_str, read_limit, 0);
        monitor_add_sent_bytes(sent_bytes);
        if (sent_bytes < 0) { // Something went wrong
                data->err_code = UNKNOWN_ERROR;
                return -1;
        }

        log(DEBUG, "READ_LIMIT: %ld - sent_bytes: %ld", read_limit, sent_bytes);

        return sent_bytes;
}
