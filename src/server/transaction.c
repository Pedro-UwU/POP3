#include <stdint.h>
#include <string.h> // strcmp
#include <sys/socket.h>

#include <pop3def.h>
#include <server/buffer.h>
#include <server/fileReader.h>
#include <server/pop3.h>
#include <server/transaction.h>
#include <server/parsers/transParser.h>
#include <server/writter.h>
#include <utils/logger.h>
#include <utils/maildir.h> // maildir_*

static unsigned send_data(struct selector_key *key);
static void handle_request(struct selector_key *key, char msg[MAX_RSP_LEN]);

static void cmd_stat(client_data *data, char msg[MAX_RSP_LEN]);
static void cmd_list(client_data *data, char msg[MAX_RSP_LEN]);
static void cmd_retr(struct selector_key *key, char msg[MAX_RSP_LEN]);
static void cmd_dele(client_data *data, char msg[MAX_RSP_LEN]);
static void cmd_rset(client_data *data, char msg[MAX_RSP_LEN]);
static int get_new_mails(client_data *data, maildir_mails_t **mails, size_t *size,
                         size_t *del_size);
static char *get_next_arg(client_data *data, char buf[MAX_ARG_LEN]);

void init_trans(const unsigned int state, struct selector_key *key)
{
        client_data *data = GET_DATA(key);
        init_trans_parser(&data->parser.trans_parser, (const char *)data->user);
}

unsigned trans_read(struct selector_key *key)
{
        client_data *data = GET_DATA(key);
        if (data->is_sending == true) {
                log(ERROR, "WTF Shouldn't be in READ in transaction with socket %d", key->fd);
                selector_set_interest_key(key, OP_WRITE);
                return TRANSACTION;
        }
        buffer *data_buffer = &data->read_buffer_client;
        if (!buffer_can_write(data_buffer)) {
                buffer_compact(&data->read_buffer_client);
                if (buffer_can_write(data_buffer) == false) {
                        // If after compact can't read, there's something wrong. Try  going to OP_WRITE to read the buffer
                        if (buffer_can_read(&data->write_buffer_client)) {
                                selector_set_interest_key(key, OP_WRITE);
                                return TRANSACTION;
                        }
                        data->err_code = UNKNOWN_ERROR;
                        return ERROR_POP3;
                }
        }

        size_t write_limit = 0;
        uint8_t *write_buffer = buffer_write_ptr(data_buffer, &write_limit);
        ssize_t read_count = recv(key->fd, write_buffer, write_limit, 0); // TODO Check flags

        if (read_count < 0) { // something went wrong
                data->err_code = UNKNOWN_ERROR;
                return ERROR_POP3;
        }
        if (read_count == 0) { // Connection close
                return DONE;
        }
        buffer_write_adv(data_buffer, read_count);
        selector_set_interest_key(key, OP_WRITE);
        return TRANSACTION;
}

unsigned trans_process(struct selector_key *key)
{
        client_data *data = GET_DATA(key);

        buffer *output_buffer = &data->write_buffer_client;
        buffer *input_buffer = &data->read_buffer_client;

        if (data->is_sending == true)
                return send_data(key);

        if (buffer_can_read(input_buffer) == false) {
                selector_set_interest_key(key, OP_READ);
                return TRANSACTION;
        }

        trans_parser_t *parser = &data->parser.trans_parser;
        trans_parse(key, parser, input_buffer);
        if (parser->ended == false) {
                selector_set_interest_key(key, OP_READ);
                return TRANSACTION;
        }

        // Handle request
        char msg[MAX_RSP_LEN];

        handle_request(key, msg);

        ssize_t sent_bytes = write_msg(key, msg);
        if (sent_bytes < 0) {
                log(ERROR, "Something went wrong when sending message.");
                return ERROR_POP3;
        }

        buffer_read_adv(output_buffer, sent_bytes);
        if (buffer_can_read(output_buffer) == true) { // Things left to send
                data->is_sending = true;
                goto finally;
        }

        if (buffer_can_read(input_buffer) == true) { // No more things to read. (PIPELINING)
                selector_set_interest_key(key, OP_WRITE);
        } else {
                selector_set_interest_key(key, OP_READ);
        }

finally:
        if (data->next_state == TRANSACTION)
                init_trans_parser(parser, (const char *)data->user);

        return data->next_state;
}

static unsigned send_data(struct selector_key *key)
{
        client_data *data = GET_DATA(key);

        buffer *output_buffer = &data->write_buffer_client;
        buffer *input_buffer = &data->read_buffer_client;

        size_t bytes_to_send = 0;
        uint8_t *bytes = buffer_read_ptr(output_buffer, &bytes_to_send);
        size_t can_send = bytes_to_send > MAX_BYTES_SEND ? MAX_BYTES_SEND : bytes_to_send;

        ssize_t sent = send(key->fd, bytes, can_send, 0);
        if (sent < 0) {
                data->err_code = UNKNOWN_ERROR;
                return ERROR_POP3;
        }

        buffer_read_adv(output_buffer, sent);
        if ((size_t)sent == bytes_to_send) { // Everything was sent
                if (data->sending_file == true) {
                        // SET INTEREST OF FILE SELECTOR TO READ AND SELF TO NOOP
                        load_more_bytes(key);
                } else {
                        data->is_sending = false;
                        if (buffer_can_read(input_buffer) == false)
                                selector_set_interest_key(key, OP_READ);

                        return data->next_state;
                }
        }

        return TRANSACTION;
}

static void handle_request(struct selector_key *key, char msg[MAX_RSP_LEN])
{
        client_data *data = GET_DATA(key);
        char *cmd = data->parser.trans_parser.cmd;

        log(DEBUG, "[TRANSACTION] CMD: %s | ARGS: %s", cmd, data->parser.trans_parser.arg);

        if (strcmp(cmd, "STAT") == 0) {
                cmd_stat(data, msg);
        } else if (strcmp(cmd, "LIST") == 0) {
                cmd_list(data, msg);
        } else if (strcmp(cmd, "RETR") == 0) {
                cmd_retr(key, msg);
        } else if (strcmp(cmd, "DELE") == 0) {
                cmd_dele(data, msg);
        } else if (strcmp(cmd, "RSET") == 0) {
                cmd_rset(data, msg);
        } else if (strcmp(cmd, "NOOP") == 0) {
                sprintf(msg, "+OK\r\n");
        } else {
                sprintf(msg, "-ERR invalid command\r\n");
        }

        data->next_state = TRANSACTION;
}

static void cmd_stat(client_data *data, char msg[MAX_RSP_LEN])
{
        maildir_mails_t *mails = NULL;
        size_t size = 0;
        size_t del_size = 0;
        if (get_new_mails(data, &mails, &size, &del_size)) {
                log(ERROR, "NULL mails");
                sprintf(msg, "-ERR server error\r\n");
                return;
        }

        sprintf(msg, "+OK %d %ld\r\n", mails->len - mails->ndel, size - del_size);
}

static void cmd_list(client_data *data, char msg[MAX_RSP_LEN])
{
        maildir_mails_t *mails = NULL;
        size_t size = 0;
        size_t del_size = 0;
        if (get_new_mails(data, &mails, &size, &del_size) != 0) {
                log(ERROR, "NULL mails");
                sprintf(msg, "-ERR server error\r\n");
                return;
        }

        log(DEBUG, "list cmd: %s", data->parser.trans_parser.cmd);
        if (data->parser.trans_parser.total_arg == 0) {
                sprintf(msg, "+OK %d new messages (%ld octects)\r\n", mails->len - mails->ndel,
                        size - del_size);
                // TODO list each mail info
                return;
        }

        char buf[MAX_ARG_LEN] = { 0 };

        char *arg = get_next_arg(data, buf);
        int n = atoi(arg);

        if (n == 0) {
                sprintf(msg, "-ERR could not understand argument, %d messages in maildrop\r\n",
                        mails->len);
                return;
        } else if (n < 0 || (unsigned)n > mails->len) {
                sprintf(msg, "-ERR no such message, only %d messages in maildrop\r\n", mails->len);
                return;
        }

        if (maildir_is_del(&mails->mails[n - 1]) == true)
                sprintf(msg, "-ERR message %d deleted\r\n", n);
        else
                sprintf(msg, "+OK %d %d\r\n", n, mails->mails[n - 1].size);
}

static void cmd_retr(struct selector_key *key, char msg[MAX_RSP_LEN])
{
        client_data *data = GET_DATA(key);

        maildir_mails_t *mails = NULL;
        size_t size = 0;
        if (get_new_mails(data, &mails, &size, NULL) != 0) {
                log(ERROR, "NULL mails");
                sprintf(msg, "-ERR server error\r\n");
                return;
        }

        char buf[MAX_ARG_LEN] = { 0 };

        char *arg = get_next_arg(data, buf);
        int n = atoi(arg);

        if (n == 0) {
                sprintf(msg, "-ERR could not understand argument, %d messages in maildrop\r\n",
                        mails->len);
                return;
        } else if (n < 0 || (unsigned)n > mails->len) {
                sprintf(msg, "-ERR no such message, only %d messages in maildrop\r\n", mails->len);
                return;
        }

        // Mark as read
        maildir_set_read(&mails->mails[n - 1], false);
        sprintf(msg, "+OK %d octects\r\n", mails->mails[n - 1].size);

        data->sending_file = true;

        strcpy(data->fr_data.file_path, mails->mails[n - 1].path);

        data->fr_data.is_reading_file = &data->sending_file;
        data->fr_data.output_buffer = &data->write_buffer_client;
        data->fr_data.client_fd = key->fd;
        data->is_sending = true;

        init_file_reader(key, &(data->fr_data));

        log(DEBUG, "Changing client to NOOP");
        selector_set_interest_key(key, OP_NOOP);
}

static void cmd_dele(client_data *data, char msg[MAX_RSP_LEN])
{
        maildir_mails_t *mails = NULL;
        unsigned long size = 0;
        if (get_new_mails(data, &mails, &size, NULL) != 0) {
                log(ERROR, "NULL mails");
                sprintf(msg, "-ERR server error\r\n");
                return;
        }

        if (data->parser.trans_parser.total_arg == 0) {
                sprintf(msg, "-ERR must give message to delete, %d messages in maildrop\r\n",
                        mails->len);
                return;
        }

        char buf[MAX_ARG_LEN] = { 0 };

        char *arg = get_next_arg(data, buf);
        int n = atoi(arg);

        if (n == 0) {
                sprintf(msg, "-ERR could not understand argument, %d messages in maildrop\r\n",
                        mails->len);
                return;
        } else if (n < 0 || (unsigned)n > mails->len) {
                sprintf(msg, "-ERR no such message, only %d messages in maildrop\r\n", mails->len);
                return;
        }

        if (maildir_is_del(&mails->mails[n - 1]) == true) {
                sprintf(msg, "-ERR message %d already deleted\r\n", n);
        } else {
                maildir_set_del(mails, n - 1, true);
                sprintf(msg, "+OK message %d deleted\r\n", n);
        }
}

static void cmd_rset(client_data *data, char msg[MAX_RSP_LEN])
{
        maildir_mails_t *mails = NULL;
        unsigned long size = 0;
        if (get_new_mails(data, &mails, &size, NULL) != 0) {
                log(ERROR, "NULL mails");
                sprintf(msg, "-ERR server error\r\n");
                return;
        }

        // RFC indicates that all mails should be marked as NOT deleted
        for (size_t i = 0; i < mails->len; i++) {
                maildir_set_del(mails, i, false);
        }

        sprintf(msg, "+OK maildrop has %d messages (%ld octects)\r\n", mails->len, size);
}

static int get_new_mails(client_data *data, maildir_mails_t **mails, size_t *size, size_t *del_size)
{
        *mails = maildir_list_new(&data->maildir);
        if (*mails == NULL)
                return 1;

        maildir_mails_t *m = *mails;

        *size = 0;
        if (del_size != NULL)
                *del_size = 0;

        for (unsigned i = 0; i < m->len; i++) {
                *size += m->mails[i].size;

                if (del_size != NULL && maildir_is_del(&m->mails[i]) == true) {
                        *del_size += m->mails[i].size;
                }
        }

        return 0;
}

static char *get_next_arg(client_data *data, char buf[MAX_ARG_LEN])
{
        size_t sep = 0;

        int i = 0;
        while (data->parser.trans_parser.arg_read > sep) {
                while (i < 1 + (MAX_ARG_LEN * MAX_ARGS)) {
                        if (data->parser.trans_parser.arg[i] == ' ') {
                                i++; // Point to char after separator
                                sep++;
                                break;
                        } else if (data->parser.trans_parser.arg[i] == '\0') {
                                sep++;
                                break;
                        }
                        i++;
                }
        }

        for (int j = 0; j < 1 + MAX_ARG_LEN; j++) {
                if (data->parser.trans_parser.arg[i + j] == ' ' ||
                    data->parser.trans_parser.arg[i + j] == '\0') {
                        buf[j] = '\0';
                        break;
                }

                buf[j] = data->parser.trans_parser.arg[i + j];
        }

        data->parser.trans_parser.arg_read++;

        return buf;
}
