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
#ifndef MONITOR_CMDS_H
#define MONITOR_CMDS_H
#include <server/buffer.h>
#include <server/monitor.h>
#include <server/monitor.h>

enum monitor_cmd_code {
        MONITOR_RM_MAILDIR,
        MONITOR_POPULATE_MAILDIR,
};

void monitor_login_cmd(monitor_data *data, bool *logged);
void monitor_quit_cmd(monitor_data *data);
void monitor_get_users_cmd(struct monitor_collection_data_t *collected_data, char *msg,
                           size_t max_msg_len);
void monitor_get_one_user_cmd(struct monitor_collection_data_t *collected_data, monitor_data *data,
                              char *uname, char *msg, size_t max_msg_len);
void monitor_add_user_cmd(monitor_data *data, char *msg, size_t max_msg_len);
void monitor_delete_user_cmd(monitor_data *data, char *msg, size_t max_msg_len);
void monitor_delete_maildir(fd_selector s, monitor_data *data);
void monitor_populate_maildir(fd_selector s, monitor_data *data);
#endif // !MONITOR_CMDS_H
