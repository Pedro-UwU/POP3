
#include "monitordef.h"
#include "pop3def.h"
#include <server/parsers/monitorParser.h>
#include <server/buffer.h>
#include <server/stm.h>
#include <server/monitor.h>
#include <server/selector.h>
#include <server/monitorCommands.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <server/user.h>
#include <stdio.h>
#include <utils/logger.h>
#include <sys/socket.h>
#include <string.h>

#define MAX_SOCKETS 1023
#define MAX_MSG_LEN 1024

static struct monitor_collection_data_t collected_data = {
        .sent_bytes = 0,
        .curr_connections = 0,
        .total_connections = 0,
        .user_list = NULL,
};

static bool write_in_buffer(buffer *buff, const char *msg, const char *log_error);
static bool continue_sending(struct selector_key *key);
static bool handle_error(struct selector_key *key);
static bool handle_finished_cmd(struct selector_key *key);
static bool handle_cmd(struct selector_key *key);
static void close_connection(struct selector_key *key);

void init_monitor(void)
{
        collected_data.user_list = get_user_array();
}

static void handleMonitorRead(struct selector_key *key)
{
        monitor_data *data = ((monitor_data *)(key)->data);
        if (data->is_sending == true) {
                log(ERROR,
                    "WTF Shouldn't be reading data when there's data to send in monitor socket %d",
                    key->fd);
                selector_set_interest_key(key, OP_WRITE);
                return;
        }

        if (buffer_can_write(&data->read_buffer) == false) {
                log(ERROR, "Read buffer full in monitor socket %d", key->fd);
                data->is_sending = true;
                selector_set_interest_key(key, OP_WRITE);
        }
        size_t can_write = 0;
        uint8_t *read_buffer = buffer_write_ptr(&data->read_buffer, &can_write);
        ssize_t read_bytes = recv(key->fd, read_buffer, can_write, 0);
        if (read_bytes < 0) {
                log(ERROR, "Something went wrong in recv in monitor socket %d", key->fd);
                close_connection(key);
                return;
        }
        if (read_bytes == 0) {
                log(DEBUG, "read 0 bytes");
                close_connection(key);
                return;
        }
        buffer_write_adv(&data->read_buffer, read_bytes);
        selector_set_interest_key(key, OP_WRITE);
}

static void handleMonitorClose(struct selector_key *key)
{
        close_connection(key);
}

static void handleMonitorWrite(struct selector_key *key)
{
        monitor_data *data = ((monitor_data *)(key)->data);
        if (data->is_sending) {
                bool data_remaining = continue_sending(key);
                if (data_remaining == false) {
                        data->is_sending = false;
                }
                if (buffer_can_read(&data->read_buffer) == false &&
                    data->cmd_data.finished_cmd == false) {
                        selector_set_interest_key(key, OP_READ);
                }
                return;
        }

        if (data->cmd_data.finished_cmd == true) {
                bool can_continue = handle_finished_cmd(key);
                if (can_continue == false) {
                        close_connection(key);
                }
                data->is_sending = true;
                data->cmd_data.finished_cmd = false;
                return;
        }

        monitor_parser_t *parser = &data->monitor_parser;
        monitor_parse(key, parser, &data->read_buffer);
        if (parser->ended == true) {
                if (parser->err_value != MONITOR_NO_ERROR) {
                        bool can_continue = handle_error(key);
                        if (can_continue == false) {
                                close_connection(key);
                        }
                        return;
                }
                data->is_sending = true;
                bool can_continue = handle_cmd(key);
                if (can_continue == false) {
                        close_connection(key);
                        return;
                }
                init_monitor_parser(parser);
                return;
        }
        if (buffer_can_read(&data->read_buffer) == false) { // Nothing else to read
                selector_set_interest_key(key, OP_READ);
        }
}

static struct fd_handler monitor_handle = {
        .handle_read = handleMonitorRead,
        .handle_write = handleMonitorWrite,
        .handle_close = handleMonitorClose,
};

static void init_new_monitor_data(monitor_data *data, int fd)
{
        data->logged = false;
        data->closed = false;
        data->is_sending = false;
        data->client_fd = fd;
        data->err_code = MONITOR_NO_ERROR;
        memset(&data->write_buffer_data, 0, MONITOR_BUFFER_SIZE);
        memset(&data->read_buffer_data, 0, MONITOR_BUFFER_SIZE);
        memset(&data->cmd_data, 0, sizeof(monitor_cmd_data_t));
        buffer_init(&data->write_buffer, MONITOR_BUFFER_SIZE, data->write_buffer_data);
        buffer_init(&data->read_buffer, MONITOR_BUFFER_SIZE, data->read_buffer_data);
        init_monitor_parser(&data->monitor_parser);
}

void acceptMonitorConnection(struct selector_key *key)
{
        struct sockaddr_storage address;
        socklen_t address_len = sizeof(address);
        int new_client_socket = accept(key->fd, (struct sockaddr *)&address, &address_len);

        if (new_client_socket < 0) { // Couldn't create a socket
                log(ERROR, "Couldn create a new monitor passive socket");
                return;
        }

        if (new_client_socket > MAX_SOCKETS) {
                log(INFO, "Socket %d is too hight, closing monitor connection", new_client_socket);
                close(new_client_socket);
                return;
        }

        monitor_data *monitor_data_ptr = calloc(1, sizeof(monitor_data)); // TODO: free
        if (monitor_data_ptr == NULL) {
                log(ERROR, "New client with socket %d can't alloc client data", new_client_socket);
                close(new_client_socket);
                return;
        }

        init_new_monitor_data(monitor_data_ptr, new_client_socket);

        selector_status status = selector_register(key->s, new_client_socket, &monitor_handle,
                                                   OP_READ, monitor_data_ptr);

        if (status != SELECTOR_SUCCESS) {
                log(ERROR, "New monitor socket %d registering failed", new_client_socket);
                close(new_client_socket);
                free(monitor_data_ptr);
                return;
        }

        log(INFO, "New client in socket %d has been registered into seletor", new_client_socket);
        return;
}

static bool write_in_buffer(buffer *buff, const char *msg, const char *log_error)
{
        if (msg == NULL) {
                log(ERROR, "Can't send NULL message");
                return false;
        }
        size_t len = strlen(msg);
        if (buffer_can_write(buff) == false) {
                if (log_error != NULL) {
                        log(ERROR, "%s", log_error);
                        return false;
                }
        }
        size_t can_write = 0;
        uint8_t *output_ptr = buffer_write_ptr(buff, &can_write);
        if (can_write < len) {
                log(ERROR, "Not enogh space in write buffer. %s", log_error);
                return false;
        }
        strcpy((char *)output_ptr, msg);
        buffer_write_adv(buff, len);
        return true;
}

static bool continue_sending(struct selector_key *key)
{
        monitor_data *data = ((monitor_data *)(key)->data);

        if (buffer_can_read(&data->write_buffer) == false) {
                selector_set_interest_key(key, OP_READ);
        }

        size_t can_read = 0;
        uint8_t *output_buffer = buffer_read_ptr(&data->write_buffer, &can_read);
        ssize_t sent = send(key->fd, output_buffer, can_read, 0);
        if (sent < 0) {
                log(ERROR, "Something went wring in send in monitor socket %d", key->fd);
                close_connection(key);
                return false;
        }
        buffer_read_adv(&data->write_buffer, sent);
        return buffer_can_read(
                &data->write_buffer); // Return true if there's still data to send, false if not.
}

static bool handle_error(struct selector_key *key)
{
        monitor_data *data = ((monitor_data *)(key)->data);
        buffer *output_buffer = &data->write_buffer;
        unsigned err_code = data->err_code;
        data->err_code = MONITOR_NO_ERROR;
        if (err_code == MONITOR_NO_ERROR) {
                log(ERROR, "Handling error when error code is NO ERROR monitoring socket %d",
                    key->fd);
                return true;
        }
        if (err_code == MONITOR_UNKNOWN_ERROR) {
                write_in_buffer(output_buffer, "UwU Unexpected Error\r\n\r\n", NULL);
                return false;
        }
        if (err_code == MONITOR_WRONG_LOGIN) {
                write_in_buffer(output_buffer, "UwU Wrong username or password\r\n\r\n", NULL);
                return true;
        }
        if (err_code == MONITOR_INVALID_ARG) {
                write_in_buffer(output_buffer, "UwU Invalid Argument\r\n\r\n", NULL);
                return true;
        }
        if (err_code == MONITOR_INVALID_USER) {
                write_in_buffer(output_buffer, "UwU Sorry that user doesn't exists\r\n\r\n", NULL);
                return true;
        }
        if (err_code == MONITOR_ALREADY_LOGGED) {
                write_in_buffer(output_buffer, "UWU User Already Logged\r\n\r\n", NULL);
                return true;
        }
        if (err_code == MONITOR_INVALID_CMD) {
                write_in_buffer(output_buffer, "UwU Invalid command\r\n\r\n", NULL);
                return true;
        }
        if (err_code == MONITOR_FULL_USERS) {
                write_in_buffer(output_buffer, "UwU Max number of users reached\r\n\r\n", NULL);
                return true;
        }
        if (err_code == MONITOR_USER_EXISTS) {
                write_in_buffer(output_buffer, "UwU User already exists!\r\n\r\n", NULL);
                return true;
        }
        if (err_code == MONITOR_NOT_LOGGED) {
                write_in_buffer(output_buffer, "UwU Login required\r\n\r\n", NULL);
                return true;
        }
        if (err_code == MONITOR_USER_ONLINE) {
                write_in_buffer(output_buffer, "UwU Can't modify online user\r\n\r\n", NULL);
                return true;
        }
        if (err_code == MONITOR_CANT_CREATE_MAILDIR) {
                write_in_buffer(output_buffer, "UwU Can't create user maildir\r\n\r\n", NULL);
                return true;
        }
        if (err_code == MONITOR_CANT_RM_MAILDIR) {
                write_in_buffer(output_buffer, "UwU Can't remove user maildir\r\n\r\n", NULL);
                return true;
        }
        if (err_code == MONITOR_CMD_ERROR) {
                write_in_buffer(output_buffer, "UwU Error Removing files\r\n\r\n", NULL);
                return true;
        }

        write_in_buffer(output_buffer, "UwU Unexpected Error\r\n\r\n", NULL);
        return false;
}

static bool handle_finished_cmd(struct selector_key *key)
{
        monitor_data *data = ((monitor_data *)(key)->data);
        monitor_cmd_data_t *cmd_data = &data->cmd_data;
        unsigned err_code = cmd_data->err_code;
        unsigned cmd_code = cmd_data->cmd_code;
        if (err_code != MONITOR_NO_ERROR) {
                data->err_code = cmd_data->err_code;
                return handle_error(key);
        }
        char msg[MAX_MSG_LEN] = { 0 };

        if (cmd_code == MONITOR_RM_MAILDIR) {
                if (collected_data.user_list == NULL) {
                        data->err_code = MONITOR_NOT_USER_LIST;
                } else {
                        monitor_delete_user_cmd(data, msg, MAX_MSG_LEN);
                }
        }

        if (data->err_code != MONITOR_NO_ERROR) {
                return handle_error(key);
        }

        write_in_buffer(&data->write_buffer, msg, "Can't write msg from handling cmd");
        return true;
}

static bool handle_cmd(struct selector_key *key)
{
        monitor_data *data = ((monitor_data *)(key)->data);
        monitor_parser_t *parser = &data->monitor_parser;
        char *cmd = parser->cmd;
        char msg[MAX_MSG_LEN] = { 0 };
        if (strcmp(cmd, "LOGIN") == 0) {
                monitor_login_cmd(data);
                if (data->err_code == MONITOR_NO_ERROR) {
                        snprintf(msg, MAX_MSG_LEN, "OwO Successfully logged\r\n\r\n");
                }
        } else if (strcmp(cmd, "QUIT") == 0) {
                return false;
        } else if (strcmp(cmd, "COMMANDS") == 0) {
                snprintf(msg, MAX_MSG_LEN,
                         "OwO Commands:\r\n"
                         "LOGIN\r\n"
                         "QUIT\r\n"
                         "COMMANDS\r\n"
                         "GET_CURR_CONN\r\n"
                         "GET_TOTAL_CONN\r\n"
                         "GET_SENT_BYTES\r\n"
                         "GET_USERS\r\n"
                         "GET_USER <username>\r\n"
                         "ADD_USER <username> <password>\r\n"
                         "DELETE_USER <username>\r\n"
                         "\r\n");
        } else if (data->logged == false) {
                data->err_code = MONITOR_NOT_LOGGED;
        } else if (strcmp(cmd, "GET_CURR_CONN") == 0) {
                snprintf(msg, MAX_MSG_LEN, "OwO %ld\r\n\r\n", collected_data.curr_connections);
        } else if (strcmp(cmd, "GET_TOTAL_CONN") == 0) {
                snprintf(msg, MAX_MSG_LEN, "OwO %ld\r\n\r\n", collected_data.total_connections);
        } else if (strcmp(cmd, "GET_SENT_BYTES") == 0) {
                snprintf(msg, MAX_MSG_LEN, "OwO %llu\r\n\r\n", collected_data.sent_bytes);
        } else if (strcmp(cmd, "GET_USERS") == 0) {
                if (collected_data.user_list == NULL) {
                        data->err_code = MONITOR_NOT_USER_LIST;
                } else {
                        monitor_get_users_cmd(&collected_data, msg, MAX_MSG_LEN);
                }
        } else if (strcmp(cmd, "GET_USER") == 0) {
                if (collected_data.user_list == NULL) {
                        data->err_code = MONITOR_NOT_USER_LIST;
                } else {
                        char *username = parser->arg;
                        monitor_get_one_user_cmd(&collected_data, data, username, msg, MAX_MSG_LEN);
                }
        } else if (strcmp(cmd, "ADD_USER") == 0) {
                if (collected_data.user_list == NULL) {
                        data->err_code = MONITOR_NOT_USER_LIST;
                } else {
                        monitor_add_user_cmd(data, msg, MAX_MSG_LEN);
                }
        } else if (strcmp(cmd, "DELETE_USER") == 0) {
                if (collected_data.user_list == NULL) {
                        data->err_code = MONITOR_NOT_USER_LIST;
                } else {
                        monitor_delete_maildir(key->s, data);
                        data->is_sending = false;
                        selector_set_interest(key->s, key->fd, OP_NOOP);
                        return true; // Executes a command
                }
        }
        /* else if to all the commands */
        else {
                data->err_code = MONITOR_INVALID_CMD;
        }

        if (data->err_code != MONITOR_NO_ERROR) {
                return handle_error(key);
        }

        write_in_buffer(&data->write_buffer, msg, "Can't write msg from handling cmd");
        return true;
}

static void close_connection(struct selector_key *key)
{
        monitor_data *data = ((monitor_data *)(key)->data);
        //Already closed
        if (data->closed) {
                return;
        }

        data->closed = true;
        log(INFO, "Closing connection from monitor socket %d", key->fd);

        selector_unregister_fd(key->s, key->fd);

        if (data != NULL) {
                // REMEMBER to free any allocated resources
                free(data);
        }
        close(key->fd);
}

void monitor_add_sent_bytes(unsigned long bytes)
{
        collected_data.sent_bytes += bytes;
}

void monitor_add_connection(void)
{
        collected_data.curr_connections += 1;
        collected_data.total_connections += 1;
}

void monitor_close_connection(void)
{
        collected_data.curr_connections -= 1;
}
