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
#ifndef FILE_READER_H
#define FILE_READER_H

#include "server/buffer.h"
#define MAX_FILE_PATH 512
#define GET_FR_DATA(x) ((file_reader_data *)(x)->data)
#include <server/selector.h>
#include <stdio.h>

typedef struct file_reader_data {
        buffer *output_buffer;
        void (**file_reader)(struct selector_key *key);
        char file_path[MAX_FILE_PATH];
        int fd;
        int client_fd;
        FILE *fp;
} file_reader_data;

void init_file_reader(struct selector_key *key, file_reader_data *fr_data);
void load_more_bytes(struct selector_key *key);
void set_external_program(char *program);

#endif
