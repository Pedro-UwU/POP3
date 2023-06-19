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

static void handle_request(client_data *data, char msg[MAX_RSP_LEN]);
static void cmd_stat(client_data *data, char msg[MAX_RSP_LEN]);
static void cmd_list(client_data *data, char msg[MAX_RSP_LEN]);
static int get_new_mails(client_data *data, maildir_mails_t **mails, unsigned long *size);
static char *get_next_arg(client_data *data, char buf[MAX_ARG_LEN]);
static void mock_read_file(struct selector_key* key);

void init_trans(const unsigned int state, struct selector_key *key)
{
        client_data *data = GET_DATA(key);
        init_trans_parser(&data->parser.trans_parser, (const char *)data->user);
}

void finish_trans(const unsigned int state, struct selector_key *key)
{
        client_data *data = GET_DATA(key);
        maildir_close(data->parser.trans_parser.maildir);
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

        if (data->is_sending == true) {
                size_t bytes_to_send = 0;
                uint8_t *bytes = buffer_read_ptr(&data->write_buffer_client, &bytes_to_send);
                size_t can_send = bytes_to_send > MAX_BYTES_SEND ? MAX_BYTES_SEND : bytes_to_send;
                ssize_t sent = send(key->fd, bytes, can_send, 0);
                if (sent < 0) {
                        data->err_code = UNKNOWN_ERROR;
                        return ERROR_POP3;
                }
                buffer_read_adv(&data->write_buffer_client, sent);
                if ((size_t)sent == bytes_to_send) { // Everything was sent
                        if (data->sending_file == true) {
                                load_more_bytes(key);
                                // SET INTEREST OF FILE SELECTOR TO READ AND SELF TO NOOP
                                //
                        } else {
                                data->is_sending = false;
                                if (buffer_can_read(&data->read_buffer_client) == false) {
                                        selector_set_interest_key(key, OP_READ);
                                }
                                return data->next_state;
                        }
                }
                return TRANSACTION;
        }

        if (buffer_can_read(&data->read_buffer_client) == false) {
                selector_set_interest_key(key, OP_READ);
                return TRANSACTION;
        }

        trans_parser_t *parser = &data->parser.trans_parser;
        trans_parse(key, parser, &data->read_buffer_client);
        if (parser->ended == false) {
                selector_set_interest_key(key, OP_READ);
                return TRANSACTION;
        }
        // Hande request
        char msg[MAX_RSP_LEN];

        handle_request(data, msg);

        ssize_t sent_bytes = write_msg(key, msg);
        if (sent_bytes < 0) {
                log(ERROR, "Something went wrong when sending message.");
                return ERROR_POP3;
        }

        // TODO from here
        mock_read_file(key);
        if (buffer_can_read(&data->write_buffer_client) == false) {
                log(ERROR, "Nothing to read after handling the request in TRANSACTION. Socket %d",
                    key->fd);
                init_trans_parser(parser, (const char *)data->user);
                return TRANSACTION; // TODO Change to error
        }
        init_trans_parser(parser, (const char *)data->user);
        data->is_sending = true;
        return TRANSACTION;
}

static void handle_request(client_data *data, char msg[MAX_RSP_LEN])
{
        log(DEBUG, "HANDLING CMD: %s - ARGS: %s", data->parser.trans_parser.cmd,
            data->parser.trans_parser.arg);

        if (strcmp(data->parser.trans_parser.cmd, "STAT") == 0) {
                cmd_stat(data, msg);
        } else if (strcmp(data->parser.trans_parser.cmd, "LIST") == 0) {
                cmd_list(data, msg);
        } else {
                sprintf(msg, "-ERR invalid command\r\n");
        }

        data->next_state = TRANSACTION;
}

static void cmd_stat(client_data *data, char msg[MAX_RSP_LEN])
{
        maildir_mails_t *mails = NULL;
        unsigned long size = 0;
        if (get_new_mails(data, &mails, &size)) {
                log(ERROR, "NULL mails");
                sprintf(msg, "-ERR server error\r\n");
                return;
        }

        sprintf(msg, "+OK %d %ld\r\n", mails->len, size);
}

static void cmd_list(client_data *data, char msg[MAX_RSP_LEN])
{
        maildir_mails_t *mails = NULL;
        unsigned long size = 0;
        if (get_new_mails(data, &mails, &size) != 0) {
                log(ERROR, "NULL mails");
                sprintf(msg, "-ERR server error\r\n");
                return;
        }

        log(DEBUG, "list cmd: %s", data->parser.trans_parser.cmd);
        if (data->parser.trans_parser.total_arg == 0) {
                sprintf(msg, "+OK %d new messages (%ld octects)\r\n", mails->len, size);
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

        sprintf(msg, "+OK %d %d\r\n", n, mails->mails[n].size);
}

static int get_new_mails(client_data *data, maildir_mails_t **mails, unsigned long *size)
{
        *mails = maildir_list_new(data->parser.trans_parser.maildir);
        if (*mails == NULL) {
                return 1;
        }

        *size = 0;
        for (unsigned i = 0; i < (*mails)->len; i++) {
                *size += (*mails)->mails[i].size;
        }

        return 0;
}

static char *get_next_arg(client_data *data, char buf[MAX_ARG_LEN])
{
        size_t sep = 0;

        int i = 0;
        while (data->parser.trans_parser.arg_read > sep) {
                for (; i < 1 + (MAX_ARG_LEN * MAX_ARGS); i++) {
                        if (data->parser.trans_parser.arg[i] == ' ' ||
                            data->parser.trans_parser.arg[i] == '\0') {
                                sep++;
                                i++;
                                break;
                        }
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

static void mock_read_file(struct selector_key* key) {
    client_data* data = GET_DATA(key);
    char * cmd = data->parser.trans_parser.cmd;
    if (strcmp(cmd, "RETR") == 0) {
        data->sending_file = true;

        strcpy(data->fr_data.file_path, "/home/pedro/Documents/temp/random_file");
        data->fr_data.is_reading_file = &data->sending_file;
        data->fr_data.output_buffer = &data->write_buffer_client;
        data->fr_data.client_fd = key->fd;
        data->is_sending = true;
        data->next_state = TRANSACTION;
        init_file_reader(key, &(data->fr_data)) ;
        log(DEBUG, "Changing client to NOOP");
        selector_set_interest_key(key, OP_NOOP);
    } else {
        log(DEBUG ,"SADAASDASDASDS");
        return;
    }
}
