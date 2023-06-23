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
#define _XOPEN_SOURCE 700

#include "server/pop3.h"
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <server/buffer.h>
#include <server/user.h>
#include <server/monitor.h>
#include <server/monitorCommands.h>
#include <utils/maildir.h>
#include <monitordef.h>
#include <utils/logger.h>
#include <stdbool.h>
#include <monitordef.h>
#include <string.h>
#include <stdio.h>

static void handle_command(struct selector_key *key)
{
        log(DEBUG, "In handling command");
        monitor_cmd_data_t *data = ((monitor_cmd_data_t *)(key)->data);
        char aux_buffer[1024] = { 0 };
        int bytes_read = read(data->cmd_fd, aux_buffer, 1024);
        log(DEBUG, "Read Bytes %d", bytes_read);
        if (bytes_read != 5) { // Not empty response
                if (data->cmd_code == MONITOR_RM_MAILDIR) {
                        data->err_code = MONITOR_CANT_RM_MAILDIR;
                } else if (data->cmd_code == MONITOR_POPULATE_MAILDIR) {
                        data->err_code = MONITOR_POPULATE_ERROR;
                } else {
                        data->err_code = MONITOR_UNKNOWN_ERROR;
                }
        } else {
                data->err_code = MONITOR_NO_ERROR;
        }

        data->finished_cmd = true;
        close(data->cmd_fd);
        selector_unregister_fd(key->s, data->cmd_fd);
        selector_set_interest(key->s, data->client_fd, OP_WRITE);
}

static void init_cmd_data(monitor_cmd_data_t *cmd_data, int client_fd, fd_selector s,
                          unsigned cmd_code)
{
        cmd_data->client_fd = client_fd;
        cmd_data->err_code = MONITOR_NO_ERROR;
        cmd_data->cmd_code = cmd_code;
        cmd_data->finished_cmd = false;
        cmd_data->s = s;
}

static struct fd_handler monitorCMDHandler = {
        .handle_read = handle_command,
        .handle_write = NULL,
};

static bool executeCommand(char *cmd, monitor_cmd_data_t *data)
{
        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
                log(ERROR, "Can't execute command \"%s\"", cmd);
                return false;
        }
        int fd = fileno(fp);
        data->cmd_fd = fd;
        selector_status ss = selector_register(data->s, fd, &monitorCMDHandler, OP_READ, data);
        if (ss != SELECTOR_SUCCESS) {
                close(fd);
                log(ERROR, "Can't register monitor command");
                return false;
        }
        log(DEBUG, "Command executed");
        return true;
}

static bool check_credentials(const char *user, const char *pass)
{
        if (strcmp(user, "admin") == 0) {
                if (strcmp(pass, "admin") == 0) {
                        return true;
                }
        }
        return false;
}

static bool extract_credentials(const char *args, char *username, char *password)
{
        size_t argsLen = strlen(args);
        if (argsLen < 4) {
                return false; // String is not well-formatted
        }
        int scannedFields = sscanf(args, "%s %s", username, password);

        if (scannedFields != 2) {
                return false; // String is not well-formatted
        }
        return true; // String is well-formatted
}

/* Commands ----------------------------------------------------------------------------------------------------------------*/

void monitor_login_cmd(monitor_data *data, bool *logged)
{
        char *args = data->monitor_parser.arg;
        char username[40] = { 0 };
        char password[40] = { 0 };
        int valid_credentials = extract_credentials(args, username, password);
        if (valid_credentials == false) {
                data->err_code = MONITOR_INVALID_ARG;
                // TODO Handle Errors
                return;
        }
        if (check_credentials(username, password) == true) {
                if (*logged == true) {
                        data->err_code = MONITOR_ALREADY_LOGGED;
                        return;
                }
                *logged = true;
                data->logged = true;
                data->err_code = MONITOR_NO_ERROR;
                return;
        }
        data->err_code = MONITOR_WRONG_LOGIN;
}

void monitor_quit_cmd(monitor_data *data)
{
        data->logged = false;
}

void monitor_get_users_cmd(struct monitor_collection_data_t *collected_data, char *msg,
                           size_t max_msg_len)
{
        size_t wrote = sprintf(msg, "OwO\r\n");
        for (int i = 0; i < MAX_USERS; i++) {
                if (collected_data->user_list[i].uname[0] == '\0') {
                        continue;
                }
                char *state_str =
                        (collected_data->user_list[i].state == USER_ONLINE)  ? "ONLINE" :
                        (collected_data->user_list[i].state == USER_LOGGING) ? "LOGGING_IN" :
                                                                               "OFFLINE";
                char aux_buffer[64] = { 0 };
                size_t line_len = sprintf(aux_buffer, "%s %s\r\n",
                                          collected_data->user_list[i].uname, state_str);
                if (line_len > (max_msg_len - 2 - wrote)) {
                        break; // Cannot write a msg larger than max_msg_len;
                }
                strcat(msg, aux_buffer);
        }
        strcat(msg, "\r\n");
}

void monitor_get_one_user_cmd(struct monitor_collection_data_t *collected_data, monitor_data *data,
                              char *uname, char *msg, size_t max_msg_len)
{
        for (int i = 0; i < MAX_USERS; i++) {
                if (collected_data->user_list[i].uname[0] == '\0') {
                        continue;
                }
                char *aux_uname = collected_data->user_list[i].uname;
                if (strcmp(uname, aux_uname) == 0) {
                        char *state_str = (collected_data->user_list[i].state == USER_ONLINE) ?
                                                  "ONLINE" :
                                          (collected_data->user_list[i].state == USER_LOGGING) ?
                                                  "LOGGING_IN" :
                                                  "OFFLINE";
                        snprintf(msg, max_msg_len, "OwO %s %s\r\n\r\n", uname, state_str);
                        return;
                }
        }
        data->err_code = MONITOR_INVALID_USER;
}

void monitor_add_user_cmd(monitor_data *data, char *msg, size_t max_msg_len)
{
        char *args = data->monitor_parser.arg;
        char uname[MAX_ARG_LEN];
        char pass[MAX_ARG_LEN];
        int valid_credentials = extract_credentials(args, uname, pass);
        if (valid_credentials == false) {
                data->err_code = MONITOR_INVALID_ARG;
                return;
        }
        int added = user_add(uname, pass);
        if (added == -1) {
                data->err_code = MONITOR_FULL_USERS;
                return;
        }
        if (added == -2) {
                data->err_code = MONITOR_USER_EXISTS;
                return;
        }

        user_maildir_t md;
        memset(&md, 0, sizeof(user_maildir_t));
        maildir_open(&md, uname);
        int maildir_created = maildir_build(&md);
        if (maildir_created == -1) {
                user_delete(uname);
                data->err_code = MONITOR_CANT_CREATE_MAILDIR;
                return;
        }

        snprintf(msg, max_msg_len, "OwO User %s added\r\n\r\n", uname);
        return;
}

void monitor_delete_user_cmd(monitor_data *data, char *msg, size_t max_msg_len)
{
        char *uname = data->monitor_parser.arg;
        int deleted = user_delete(uname);
        if (deleted == -1) {
                data->err_code = MONITOR_INVALID_USER;
                return;
        }
        if (deleted == -2) {
                data->err_code = MONITOR_USER_ONLINE;
                return;
        }

        snprintf(msg, max_msg_len, "OwO User %s deleted\r\n\r\n", uname);
        return;
}

void monitor_delete_maildir(fd_selector s, monitor_data *data)
{
        char *uname = data->monitor_parser.arg;
        if (user_exists(uname) == false) {
                data->err_code = MONITOR_USER_EXISTS;
                return;
        }

        char cmd[1024];
        user_maildir_t md;
        maildir_open(&md, data->monitor_parser.arg);
        snprintf(cmd, 1024, "rm -rf %s; echo DONE", md.path);

        init_cmd_data(&data->cmd_data, data->client_fd, s, MONITOR_RM_MAILDIR);
        executeCommand(cmd, &data->cmd_data);
}

void monitor_populate_maildir(fd_selector s, monitor_data *data)
{
        char *uname = data->monitor_parser.arg;
        if (user_exists(uname) == false) {
                data->err_code = MONITOR_INVALID_USER;
                return;
        }
        char cmd[1024];
        user_maildir_t md;
        maildir_open(&md, data->monitor_parser.arg);
        sprintf(cmd,
                "id=$(date +%%s); "
                "for i in $(seq 1 10); do "
                "dd if=/dev/urandom bs=1KB count=$((12 + (%d*i) %% 49152)) 2> /dev/null | base64 > \"%s/new/${id}_mail\"${i}; printf \'\\r\\n\' >> \"%s/new/${id}_mail\"${i}; done; "
                "[ $(find %s/new/ -maxdepth 1 -type f -name \"${id}_mail*\" | wc -l) -eq 10 ] "
                "&& echo \"DONE\" || rm %s/new/\"${id}\"*",
                1111, md.path, md.path, md.path);
        init_cmd_data(&data->cmd_data, data->client_fd, s, MONITOR_POPULATE_MAILDIR);
        executeCommand(cmd, &data->cmd_data);
}
