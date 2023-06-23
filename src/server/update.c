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
#include <server/pop3.h>
#include <server/update.h>
#include <server/writter.h>
#include <utils/logger.h>

static void handle_request(client_data *data, char msg[MAX_RSP_LEN]);

void init_update(const unsigned state, struct selector_key *key)
{
        client_data *data = GET_DATA(key);
        init_update_parser(&data->parser.update_parser);

        selector_set_interest_key(key, OP_WRITE);
}

unsigned update_process(struct selector_key *key)
{
        client_data *data = GET_DATA(key);

        buffer *output_buffer = &data->write_buffer_client;

        // Handle request
        char msg[MAX_RSP_LEN];

        handle_request(data, msg);

        ssize_t sent_bytes = write_msg(key, msg);
        if (sent_bytes < 0) {
                log(ERROR, "Something went wrong when sending message.");
                return ERROR_POP3;
        }

        buffer_read_adv(output_buffer, sent_bytes);

        data->next_state = DONE;

        return data->next_state;
}

static void handle_request(client_data *data, char msg[MAX_RSP_LEN])
{
        maildir_mails_t *m = &data->maildir.new;

        unsigned n_read = 0;

        bool all_rm = true;
        bool all_mv = true;

        for (unsigned i = 0; i < m->len; i++) {
                if (maildir_is_del(&m->mails[i]) == true) {
                        all_rm &= maildir_del_permanentely(&m->mails[i]);
                } else if (maildir_is_read(&m->mails[i]) == true) {
                        all_mv &= maildir_move_as_read(&m->mails[i]);
                        n_read++;
                }
        }

        if (all_rm == false && all_mv == false) {
                sprintf(msg, "-ERR some messages could not be deleted,"
                             " and some could not be marked as read\r\n");
        } else if (all_rm == false) {
                sprintf(msg, "-ERR some messages could not be deleted\r\n");
        } else if (all_mv == false) {
                sprintf(msg, "-ERR some messages could not be marked as read\r\n");
        }

        if (m->len - m->ndel - n_read == 0)
                sprintf(msg, "+OK server signing off (maildrop empty)\r\n");
        else
                sprintf(msg, "+OK server signing off (%d unread, %d removed, %d total)\r\n",
                        m->len - n_read, m->ndel, m->len);
}
