#ifndef FILE_READER_H
#define FILE_READER_H

#include "server/buffer.h"
#define MAX_FILE_PATH 512
#define GET_FR_DATA(x) ((file_reader_data *)(x)->data)
#include <server/selector.h>
#include <stdio.h>

typedef struct file_reader_data {
        buffer *output_buffer;
        bool *is_reading_file;
        char file_path[MAX_FILE_PATH];
        int fd;
        int client_fd;
        FILE *fp;
} file_reader_data;

void init_file_reader(struct selector_key *key, file_reader_data *fr_data);
void load_more_bytes(struct selector_key *key);

#endif
