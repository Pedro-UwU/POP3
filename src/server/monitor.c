#include <server/parsers/monitorParser.h>
#include <server/buffer.h>
#include <server/stm.h>
#include <server/monitor.h>
#include <server/selector.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/_types/_ssize_t.h>
#include <utils/logger.h>
#include <sys/socket.h>
#include <string.h>

#define MAX_SOCKETS 1023

static bool pop3_server_running = false;

struct monitor_data_t {
    unsigned long sent_bytes;
    unsigned long curr_connections;
    unsigned long total_connections;
    char** user_list;
};

static struct monitor_data_t collected_data = {
    .sent_bytes = 0,
    .curr_connections = 0,
    .total_connections = 0,
    .user_list = NULL,
};

static void handleMonitorRead(struct selector_key* key);
static void handleMonitorWrite(struct selector_key* key);
static void handleMonitorClose(struct selector_key* key);
static bool write_in_buffer(buffer* buff, const char* msg, const char *log_error);
static void write_server_down_msg(struct selector_key* key);
static bool continue_sending(struct selector_key* key);
static bool handle_error(struct selector_key* key);
static void handle_cmd(struct selector_key* key);
static void close_connection(struct selector_key* key);


static struct fd_handler monitor_handle = {
    .handle_read = handleMonitorRead,
    .handle_write = handleMonitorWrite,
    .handle_close = handleMonitorClose,
};


static void init_new_monitor_data(monitor_data* data, int fd) {
    data->logged = false;
    data->closed = false;
    data->is_sending = false;
    data->client_fd = fd;
    memset(&data->write_buffer_data, 0, MONITOR_BUFFER_SIZE);
    memset(&data->read_buffer_data, 0, MONITOR_BUFFER_SIZE);
    buffer_init(&data->write_buffer, MONITOR_BUFFER_SIZE, data->write_buffer_data);
    buffer_init(&data->read_buffer, MONITOR_BUFFER_SIZE, data->read_buffer_data);
    init_monitor_parser(&data->monitor_parser);
}

void acceptMonitorRead(struct selector_key* key) {
        struct sockaddr_storage address;
        socklen_t address_len = sizeof(address);
        int new_client_socket =
                accept(key->fd, (struct sockaddr *)&address, &address_len);

        if (new_client_socket < 0) { // Couldn't create a socket
                log(ERROR, "Couldn create a new monitor passive socket");
                return;
        }

        if (new_client_socket > MAX_SOCKETS) {
                log(INFO, "Socket %d is too hight, closing monitor connection", new_client_socket);
                close(new_client_socket);
                return;
        }

        monitor_data* monitor_data_ptr = calloc(1, sizeof(monitor_data)); // TODO: free
        if (monitor_data_ptr == NULL) {
                log(ERROR, "New client with socket %d can't alloc client data", new_client_socket);
                close(new_client_socket);
                return;
        }

        init_new_monitor_data(monitor_data_ptr, new_client_socket);

        selector_status status = selector_register(key->s, new_client_socket, &monitor_handle,
                                                   OP_WRITE /* it starts with a greeting */,
                                                   monitor_data_ptr);

        if (status != SELECTOR_SUCCESS) {
                log(ERROR, "New monitor socket %d registering failed", new_client_socket);
                close(new_client_socket);
                free(monitor_data_ptr);
                return;
        }

        log(INFO, "New client in socket %d has been registered into seletor", new_client_socket);
        return;
}

static void handleMonitorRead(struct selector_key* key) {
        monitor_data* data = ((monitor_data*)(key)->data);
        if (pop3_server_running == false) {
            write_server_down_msg(key);
            data->is_sending = true;
            selector_set_interest_key(key, OP_WRITE);
        }
        if (data->is_sending == true) {
                log(ERROR, "WTF Shouldn't be reading data when there's data to send in monitor socket %d", key->fd);
                selector_set_interest_key(key, OP_WRITE);
                return;
        }

        if (buffer_can_write(&data->read_buffer) == false) {
                log(ERROR, "Read buffer full in monitor socket %d", key->fd);                
                data->is_sending = true;
                selector_set_interest_key(key, OP_WRITE);
        }
        size_t can_write = 0;
        uint8_t* read_buffer = buffer_read_ptr(&data->read_buffer, &can_write);
        ssize_t read_bytes = recv(key->fd, read_buffer, can_write, 0);
        if (read_bytes < 0) {
            log(ERROR, "Something went wrong in recv in monitor socket %d", key->fd);
            close_connection(key);
            return;
        }
        if (read_bytes == 0) {
            close_connection(key);
            return;
        }
        buffer_write_adv(&data->read_buffer, read_bytes);
        selector_set_interest_key(key, OP_WRITE);
}
    
static void handleMonitorClose(struct selector_key* key) {
        close_connection(key);

}

static void handleMonitorWrite(struct selector_key* key) {
        monitor_data* data = ((monitor_data*)(key)->data);
        if (data->is_sending) {
            bool data_remaining = continue_sending(key);
            if (data_remaining == false) {
                selector_set_interest_key(key, OP_READ);
                data->is_sending = false;
            }
            return;
        }

        monitor_parser_t* parser = &data->monitor_parser;
        monitor_parse(key, parser, &data->read_buffer);
        if (parser->ended == true) {
            if (parser->err_value != NO_ERROR) {
                bool can_continue = handle_error(key);
                if (can_continue == false) {
                    close_connection(key);
                }
                return;
            }
            handle_cmd(key);
            data->is_sending = true;
        }
}

static void write_server_down_msg(struct selector_key* key) {
        static const char* msg = ":( POP3 Server is offline\r\n";
        static const char* log_error = "Cant send server down msg";
        monitor_data* data = ((monitor_data*)(key)->data);
        write_in_buffer(&data->write_buffer, msg, log_error);
        return;
}

static bool write_in_buffer(buffer* buff, const char* msg, const char *log_error) {
        if (msg == NULL) {
                log(ERROR, "Can't send NULL message");
                return false;
        }
        size_t len = strlen(msg);
        if (buffer_can_write(buff) == false) {
                if (log_error != NULL) {
                        log(ERROR, "%s", log_error);
                        return false;
                }
        }
        size_t can_write = 0;
        uint8_t* output_ptr = buffer_write_ptr(buff, &can_write);
        if (can_write < len) {
            log (ERROR, "Not enogh space in write buffer. %s", log_error);
            return false;
        }
        strcpy((char*)output_ptr, msg);
        buffer_write_adv(buff, len);
        return true;
}

static bool continue_sending(struct selector_key* key) {
        monitor_data* data = ((monitor_data*)(key)->data);
        
        if (buffer_can_read(&data->write_buffer) == false) {
            selector_set_interest_key(key, OP_READ);
        }

        size_t can_read = 0;
        uint8_t* output_buffer = buffer_read_ptr(&data->write_buffer, &can_read);
        ssize_t sent = send(key->fd, output_buffer, can_read, 0);
        if (sent < 0) {
            log(ERROR, "Something went wring in send in monitor socket %d", key->fd);
            close_connection(key);
            return false;
        }
        buffer_read_adv(&data->write_buffer, sent);
        return buffer_can_read(&data->write_buffer); // Return true if there's still data to send, false if not.
}

static bool handle_error(struct selector_key* key) {
    // TODO
    return false;
}

static void handle_cmd(struct selector_key* key) {
    return;
}


static void close_connection(struct selector_key* key) {
        monitor_data* data = ((monitor_data*)(key)->data);
        //Already closed
        if (data->closed) {
                return;
        }

        data->closed = true;
        log(INFO, "Closing connection from monitor socket %d", key->fd);

        selector_unregister_fd(key->s, key->fd);

        if (data != NULL) {
                // REMEMBER to free any allocated resources
                free(data);
        }
        close(key->fd);
}
