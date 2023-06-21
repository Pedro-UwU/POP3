#include <server/buffer.h>
#include <server/user.h>
#include <server/monitor.h>
#include <server/monitorCommands.h>
#include <utils/maildir.h>
#include <utils/logger.h>
#include <stdbool.h>
#include <monitordef.h>
#include <string.h>
#include <stdio.h>
#include <pop3def.h>

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

void monitor_login_cmd(monitor_data *data)
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
                data->logged = true;
                data->err_code = MONITOR_NO_ERROR;
                return;
        }
        data->err_code = MONITOR_WRONG_LOGIN;
}

void monitor_quit_cmd(monitor_data *data)
{
        data->closed = true;
}

void monitor_get_users_cmd(struct monitor_collection_data_t *collected_data, monitor_data *data,
                           char *msg, size_t max_msg_len)
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

void monitor_delete_user_cmd(monitor_data *data, char *msg, size_t max_msg_len) {
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
