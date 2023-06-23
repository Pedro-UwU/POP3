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
#ifndef CLIENT_CMD_H
#define CLIENT_CMD_H

#include <stdbool.h>
#include <client/spec.h>

monitor_cmd get_cmd_index(const char *cmd);
bool valid_cmd_argc(const monitor_cmd cmd, const int argc);

int cmd_exec(int fd, monitor_cmd cmd, char **args);

int cmd_quit(int fd);

int cmd_login(int fd, const char *user, const char *pass);

int cmd_get_users(int fd);

int cmd_get_user(int fd, const char *user);

int cmd_get_curr_conn(int fd);

int cmd_get_total_conn(int fd);

int cmd_get_sent_bytes(int fd);

int cmd_add_user(int fd, const char *user, const char *pass);

int cmd_populate_user(int fd, const char *user);

int cmd_delete_user(int fd, const char *user);

int cmd_commands(int fd);

#endif // !CLIENT_CMD_H
