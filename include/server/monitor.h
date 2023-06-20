#ifndef MONITOR_H
#define MONITOR_H
#include <server/stm.h>
#include <server/buffer.h>
#include <server/parsers/monitorParser.h>
#include <server/user.h>
#include <monitordef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct monitor_data {
        bool closed;
        bool is_sending;
        bool logged;
        int client_fd;
        unsigned err_code;
        uint8_t read_buffer_data[MONITOR_BUFFER_SIZE];
        uint8_t write_buffer_data[MONITOR_BUFFER_SIZE];
        struct buffer write_buffer;
        struct buffer read_buffer;
        monitor_parser_t monitor_parser;
} monitor_data;

struct monitor_collection_data_t {
        unsigned long sent_bytes;
        unsigned long curr_connections;
        unsigned long total_connections;
        user_t *user_list;
};

void init_monitor(void);
void monitor_add_sent_bytes(unsigned long bytes);
void monitor_user_online(void);
void monitor_user_offline(void);
void acceptMonitorConnection(struct selector_key *key);

#endif // !MONITOR_H
