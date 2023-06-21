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
