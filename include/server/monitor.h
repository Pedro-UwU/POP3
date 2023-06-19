#ifndef MONITOR_H
#define MONITOR_H
#include <server/stm.h>
#include <server/buffer.h>
#include <server/parsers/monitorParser.h>
#include <monitordef.h>
#include <stdbool.h>
#include <stdint.h>


typedef struct monitor_data {
    bool closed;
    bool is_sending;
    bool logged;
    uint8_t read_buffer_data[MONITOR_BUFFER_SIZE];
    uint8_t write_buffer_data[MONITOR_BUFFER_SIZE];
    int client_fd;
    struct buffer write_buffer;
    struct buffer read_buffer;
    monitor_parser_t monitor_parser;
} monitor_data;

void init_monitor(void);
void monitor_add_sent_bytes(unsigned long bytes);
void monitor_user_online(void);
void monitor_user_offline(void);

#endif // !MONITOR_H
