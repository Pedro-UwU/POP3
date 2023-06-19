#include <pop3def.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <utils/logger.h>
#include <server/buffer.h>
#include <server/parsers/authParser.h>
#include <server/pop3.h>
#include <server/auth.h>
#include <server/user.h>
#include <server/writter.h>
#include <stdint.h>
#include <sys/socket.h>
#define MAX_BUFF_SIZE 1024

static char aux_buffer[MAX_BUFF_SIZE] = { 0 };

static int process_cmd(client_data *data);
static char *generateMsg(client_data *data, int status);

void init_auth(const unsigned state, struct selector_key *key)
{
        client_data *data = GET_DATA(key);
        init_auth_parser(&data->parser.auth_parser);
}

unsigned auth_read(struct selector_key *key)
{
        client_data *data = GET_DATA(key);
        if (data->is_sending == true) {
                log(ERROR, "WTF Shouldn't be in READ in auth with socket %d", key->fd);
                selector_set_interest_key(key, OP_WRITE);
                return AUTH;
        }
        buffer *data_buffer = &data->read_buffer_client;
        if (!buffer_can_write(data_buffer)) {
                buffer_compact(&data->read_buffer_client);
                if (buffer_can_write(data_buffer) == false) {
                        // If after compact can't read, there's something wrong. Try  going to OP_WRITE to read the buffer
                        if (buffer_can_read(&data->write_buffer_client)) {
                                selector_set_interest_key(key, OP_WRITE);
                                return AUTH;
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
        return AUTH;
}

unsigned auth_process(struct selector_key *key)
{
        client_data *data = GET_DATA(key);
        buffer *output_buffer = &data->write_buffer_client;
        buffer *input_buffer = &data->read_buffer_client;
        if (data->is_sending == true) {
                bool something_to_read = buffer_can_read(output_buffer);
                if (something_to_read == true) {
                        log(ERROR,
                            "WTF, is_sending = true and empty buffer in auth_process with socket %d",
                            key->fd);
                        data->is_sending = false;
                        return AUTH;
                }
                size_t read_limit = 0;
                uint8_t *read_buffer = buffer_read_ptr(output_buffer, &read_limit);
                ssize_t bytes_sent = send(key->fd, read_buffer, read_limit, 0);
                if (bytes_sent < 0) { // Something went wrong
                        data->err_code = UNKNOWN_ERROR;
                        return ERROR_POP3;
                }
                // send() does not return 0 except if the buffer had 0 bytes
                log(DEBUG, "READ_ADV_1");
                buffer_read_adv(output_buffer, bytes_sent);

                if (buffer_can_read(output_buffer) == false) { // Everything was sent
                        // Here I should check if the buffer needs to be refilled. In the Auth state this cannot happen, but in the Transition state, It may
                        data->is_sending = false;
                        return data->next_state;
                }
                return AUTH; // There's that needs to be send
        }

        if (buffer_can_read(output_buffer) == true) {
                log(ERROR,
                    "WTF Should be here if there's data to send in auth_process in socket %d",
                    key->fd);
                data->is_sending = true;
                return AUTH;
        }
        auth_parser_t *auth_parser = &data->parser.auth_parser;
        auth_parse(key, auth_parser, input_buffer);
        if (auth_parser->ended == false) { // The command is incomplete
                selector_set_interest_key(
                        key,
                        OP_READ); // Go to read again to wait for the rest of the command // TODO, check this
                return AUTH;
        }

        int status = process_cmd(data);

        char *msg = generateMsg(data, status);
        ssize_t sent_bytes = write_msg(key, msg);
        if (sent_bytes < 0) {
                return ERROR_POP3;
        }

        buffer_read_adv(output_buffer, sent_bytes);
        if (buffer_can_read(output_buffer) == true) { // Things left to send
                data->is_sending = true;
                return AUTH;
        }
        if (buffer_can_read(input_buffer) == true) { // No more things to read. (PIPELINING)
                selector_set_interest_key(key, OP_WRITE);
        } else {
                selector_set_interest_key(key, OP_READ);
        }
        int next_state = data->next_state;
        if (next_state == -1) {
                log(ERROR, "Invalid next state in auth_process with socket %d", key->fd);
                next_state = AUTH;
        }
        data->next_state = -1;
        if (next_state == AUTH) {
                init_auth_parser(auth_parser); // Init again to delete previous data
        }
        return next_state;
}

static int process_cmd(client_data *data)
{
        char *cmd = data->parser.auth_parser.cmd;
        if (strcmp("QUIT", cmd) == 0) {
                data->next_state = DONE;
                data->parser.auth_parser.quit = true;
                return 0;
        }
        if (strcmp("USER", cmd) == 0) {
                bool exists = user_exists(data->parser.auth_parser.arg);
                if (exists == true) {
                        if (user_get_state(data->parser.auth_parser.arg) != USER_OFFLINE) {
                                data->err_code = USER_ALREADY_ONLINE;
                                data->next_state = AUTH;
                                return -1;
                        }
                        data->next_state = AUTH;
                        if (data->user[0] != 0) { // Trying to log in with another user
                                user_set_state((char *)data->user, USER_OFFLINE);
                        }
                        strcpy((char *)data->user, data->parser.auth_parser.arg);
                        user_set_state((char *)data->user, USER_LOGGING);
                        return 0;
                }
                data->err_code = INVALID_USER;
                data->next_state = AUTH;
                return -1;
        }
        if (strcmp("PASS", cmd) == 0) {
                if (strlen((const char *)data->user) == 0) {
                        data->err_code = WRONG_TIME_PASS_CMD; // PASS command before USER
                        data->next_state = AUTH;
                        return -1;
                }
                bool logged = user_check_pass((const char *)data->user,
                                              (const char *)data->parser.auth_parser.arg);
                if (logged == false) {
                        user_set_state((const char *)data->user, USER_OFFLINE);
                        memset(data->user, 0, MAX_ARG_LEN);
                        data->err_code = WRONG_PASSWORD;
                        data->next_state = AUTH;
                        return -1;
                }
                user_set_state((const char *)data->user, USER_ONLINE);
                data->next_state = TRANSACTION;
                return 0;
        }
        if (data->err_code != INVALID_CHAR) {
                data->err_code = INVALID_CMD;
                data->next_state = AUTH;
                memset(data->user, 0, MAX_ARG_LEN + 1);
                return AUTH;
        }
        data->next_state = ERROR_POP3;
        return -1;
}

static char *generateMsg(client_data *data, int status)
{
        if (status == 0) { // No error
                if (data->parser.auth_parser.quit == true) {
                        return "+OK closing connection\r\n";
                }
                if (user_get_state((const char *)data->user) == USER_LOGGING) {
                        return "+OK valid user\r\n";
                }
                return "+OK logged in\r\n";
        }
        int err = data->err_code;
        if (err == INVALID_USER) {
                return "-ERR never heard of that user\r\n";
        }
        if (err == WRONG_TIME_PASS_CMD) {
                return "-ERR cant use PASS before USER\r\n";
        }
        if (err == WRONG_PASSWORD) {
                return "-ERR invalid password :(\r\n";
        }
        if (err == INVALID_CMD) {
                return "-ERR invalid command\r\n";
        }
        if (err == USER_ALREADY_ONLINE) {
                return "-ERR user already online\r\n";
        }
        return "-ERR unknown error\r\n";
}
