#ifndef POP3
#define POP3

#include "server/buffer.h"
#include <server/stm.h>
#include <server/pop3.h>
#include <server/states/greeting.h>
#include <server/selector.h>
#include <stdlib.h>
#include <unistd.h>
#include <utils/logger.h>
#include <sys/socket.h>
#include <sys/select.h>

//It is 1023 because of the select() limitation: https://stackoverflow.com/questions/2332741/what-is-the-theoretical-maximum-number-of-open-tcp-connections-that-a-modern-lin
#define MAX_SOCKETS 1023

static void init_new_client_data(client_data* data, int new_fd, struct sockaddr_storage client_address);

static const fd_handler pop3_handler = {
    .handle_read = NULL,
    .handle_write = NULL,
    .handle_close = NULL,
    .handle_block = NULL
};

const struct state_definition client_states[] = {
    {
        .state = GREETING_WRITE,
        .on_read_ready = NULL,
    },
    {
        .state = ERROR_POP3,
    }
};



// When a new connection comes, we need to create a new passive socket and init all the client data
void pop3AcceptPassive(struct selector_key* key) {
    struct sockaddr_storage client_address;
    socklen_t client_address_len = sizeof(client_address);
    int new_client_socket = accept(key->fd, (struct sockaddr*)&client_address, &client_address_len);

    if (new_client_socket < 0) { // Couldn't create a socket
        log(ERROR, "Couldn create a new passive socket");
        return;
    }

    if (new_client_socket > MAX_SOCKETS) {
        log(INFO, "Socket %d is too hight, closing connection", new_client_socket);
        close(new_client_socket);
        return;
    }

    client_data* client_data_ptr = calloc(1, sizeof(client_data));
    if (client_data_ptr == NULL) {
        log(ERROR, "New client with socket %d can't alloc client data", new_client_socket);
        close(new_client_socket);
        return;
    }

    init_new_client_data(client_data_ptr, new_client_socket, client_address);

    selector_status status = selector_register(key->s, new_client_socket, &pop3_handler, OP_WRITE /* it starts with a greeting */, client_data_ptr);
        
    if (status != SELECTOR_SUCCESS) {
        log(ERROR, "New socket %d registering failed", new_client_socket);
        close(new_client_socket);
        free(client_data_ptr);
        return;
    }

    log(INFO, "New client in socket %d has been registered into seletor", new_client_socket);
    return;
}

static void init_new_client_data(client_data* data, int new_fd, struct sockaddr_storage client_address) {
    data->stm.initial = GREETING_WRITE;
    data->stm.states = client_states;
    data->stm.max_state = ERROR_POP3;
    data->closed = false;
    data->client_fd = new_fd;
    data->client_address = client_address; 
    buffer_init(&data->client_buffer, BUFFER_SIZE, data->client_buffer_data);

    stm_init(&data->stm);
}

#endif /* ifndef POP3 */