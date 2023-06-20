#ifndef MONITOR_CMDS_H
#define MONITOR_CMDS_H
#include <server/buffer.h>
#include <server/monitor.h>

void monitor_login_cmd(monitor_data *data);
void monitor_get_users_cmd(monitor_data* data, buffer* out_buffer);

#endif // !MONITOR_CMDS_H
