#define _XOPEN_SOURCE 700

#include <server/fileReader.h>
#include <server/selector.h>
#include <server/pop3.h>
#include <server/buffer.h>
#include <utils/logger.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

static void handle_file_reader(struct selector_key *key);
static void change_interests(struct selector_key *key);

static struct fd_handler file_reader_handler = {
        .handle_read = handle_file_reader,
        .handle_write = NULL,
        .handle_block = NULL,
};

static void handle_file_reader(struct selector_key *key)
{
        file_reader_data *fr_data = ((file_reader_data *)key->data);
        buffer *output_buffer = fr_data->output_buffer;
        if (buffer_can_write(output_buffer) == false) {
                log(DEBUG, "File reader trying to write in full buffer of socket %d",
                    fr_data->client_fd);
                change_interests(key);
                return;
        }

        size_t can_read_bytes = 0;
        uint8_t *write_ptr = buffer_write_ptr(output_buffer, &can_read_bytes);
        ssize_t read_bytes = read(fr_data->fd, write_ptr, can_read_bytes);

        change_interests(key);
        if (read_bytes < 0) {
                log(ERROR, "Read of cat wen't wrong");
                selector_unregister_fd(key->s, key->fd);
                return;
        }
        if (read_bytes == 0) { // EOF
                selector_unregister_fd(key->s, key->fd);
                *(fr_data->file_reader) = NULL;
                close(fr_data->fd);
                return;
        }
        buffer_write_adv(output_buffer, read_bytes);
        return;
}

void init_file_reader(struct selector_key *key, file_reader_data *fr_data)
{
        char aux_buffer[1024] = { 0 };
        sprintf(aux_buffer, "cat \"%s\" | sed '1!s/^\\./\\.\\./g'", fr_data->file_path);

        FILE *fp = popen(aux_buffer, "r");
        if (fp == NULL) {
                log(ERROR, "Couldn't open file and run command");
                return;
        }

        fr_data->fp = fp;
        int fd = fileno(fp);
        fr_data->fd = fd;

        //TODO should I make the fd not blocking?
        selector_register(key->s, fd, &file_reader_handler, OP_READ, fr_data);
        log(DEBUG, "Changing client to NOOP");
        selector_set_interest_key(key, OP_NOOP);
}

void load_more_bytes(struct selector_key *key)
{
        client_data *data = GET_DATA(key);
        file_reader_data *fr_data = &data->fr_data;
        log(DEBUG, "Changing client to NOOP and file_reader to READ to read more bytes");
        selector_set_interest_key(key, OP_NOOP);
        selector_set_interest(key->s, fr_data->fd, OP_READ);
}

static void change_interests(struct selector_key *key)
{
        file_reader_data *fr_data = GET_FR_DATA(key);
        log(DEBUG, "Changing client to WRITE and file_reader to NOOP");
        selector_set_interest(key->s, fr_data->client_fd, OP_WRITE);
        selector_set_interest_key(key, OP_NOOP);
}
