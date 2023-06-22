#ifndef MONITOR_CMDS_H
#define MONITOR_CMDS_H
#include <server/buffer.h>
#include <server/monitor.h>
#include <server/monitor.h>

enum monitor_cmd_code {
        MONITOR_RM_MAILDIR,
        MONITOR_POPULATE_MAILDIR,
};

void monitor_login_cmd(monitor_data *data);
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
