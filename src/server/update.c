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
