#include <monitordef.h>
#include <server/buffer.h>
#include <server/user.h>
#include <server/monitor.h>
#include <server/monitorCommands.h>
#include <utils/logger.h>
#include <stdbool.h>
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

void monitor_get_users_cmd(monitor_data *data, buffer *out_buffer) {   
        if (buffer_can_write(out_buffer) == false) {
            log(ERROR, "Can't write in output_buffer");
        }
        for (int i = 0; i < MAX_USER
}

