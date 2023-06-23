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
#ifndef MONITOR_H
#define MONITOR_H
#include <server/stm.h>
#include <server/buffer.h>
#include <server/parsers/monitorParser.h>
#include <server/user.h>
#include <monitordef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct monitor_cmd_data {
        int cmd_fd;
        int client_fd;
        bool finished_cmd;
        fd_selector s;
        unsigned cmd_code;
        unsigned err_code;
} monitor_cmd_data_t;

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
        monitor_cmd_data_t cmd_data;
} monitor_data;

struct monitor_collection_data_t {
        unsigned long long sent_bytes;
        unsigned long curr_connections;
        unsigned long total_connections;
        user_t *user_list;
};

void init_monitor(void);
void monitor_add_sent_bytes(unsigned long bytes);
void monitor_user_online(void);
void monitor_user_offline(void);
void monitor_add_connection(void);
void monitor_close_connection(void);
void acceptMonitorConnection(struct selector_key *key);

#endif // !MONITOR_H
