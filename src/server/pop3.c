#ifndef POP3
#define POP3

#include "pop3def.h"
#include "server/buffer.h"
#include <server/stm.h>
#include <server/pop3.h>
#include <server/states/greeting.h>
#include <server/selector.h>
#include <server/auth.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <utils/logger.h>
#include <sys/socket.h>
#include <sys/select.h>

//It is 1023 because of the select() limitation: https://stackoverflow.com/questions/2332741/what-is-the-theoretical-maximum-number-of-open-tcp-connections-that-a-modern-lin
#define MAX_SOCKETS 1023

static void init_new_client_data(client_data *data, int new_fd,
                                 struct sockaddr_storage client_address);

void pop3_handle_read(struct selector_key *key);
void pop3_handle_write(struct selector_key *key);
void pop3_handle_close(struct selector_key *key);

void close_connection(struct selector_key *key);

static const fd_handler pop3_handler = {
        .handle_read = pop3_handle_read,
        .handle_write = pop3_handle_write,
        .handle_close = NULL,
};

const struct state_definition client_states[] = { {
                                                          .state = GREETING_WRITE,
                                                          .on_write_ready = greeting_write,
                                                  },
                                                  {
                                                          .state = AUTH,
                                                          .on_arrival = init_auth,
                                                          .on_read_ready = auth_read,
                                                          .on_write_ready = auth_process,
                                                  },
                                                  {
                                                          .state = TRANSACTION,
                                                  },
                                                  {
                                                          .state = DONE,
                                                  },
                                                  {
                                                          .state = ERROR_POP3,
                                                  } };

// When a new connection comes, we need to create a new passive socket and init all the client data
void pop3AcceptPassive(struct selector_key *key)
{
        struct sockaddr_storage client_address;
        socklen_t client_address_len = sizeof(client_address);
        int new_client_socket =
                accept(key->fd, (struct sockaddr *)&client_address, &client_address_len);

        if (new_client_socket < 0) { // Couldn't create a socket
                log(ERROR, "Couldn create a new passive socket");
                return;
        }

        if (new_client_socket > MAX_SOCKETS) {
                log(INFO, "Socket %d is too hight, closing connection", new_client_socket);
                close(new_client_socket);
                return;
        }

        client_data *client_data_ptr = calloc(1, sizeof(client_data)); // TODO: free
        if (client_data_ptr == NULL) {
                log(ERROR, "New client with socket %d can't alloc client data", new_client_socket);
                close(new_client_socket);
                return;
        }

        init_new_client_data(client_data_ptr, new_client_socket, client_address);

        selector_status status = selector_register(key->s, new_client_socket, &pop3_handler,
                                                   OP_WRITE /* it starts with a greeting */,
                                                   client_data_ptr);

        if (status != SELECTOR_SUCCESS) {
                log(ERROR, "New socket %d registering failed", new_client_socket);
                close(new_client_socket);
                free(client_data_ptr);
                return;
        }

        log(INFO, "New client in socket %d has been registered into seletor", new_client_socket);
        return;
}

static void init_new_client_data(client_data *data, int new_fd,
                                 struct sockaddr_storage client_address)
{
        data->stm.initial = GREETING_WRITE;
        data->stm.states = client_states;
        data->stm.max_state = ERROR_POP3;
        data->err_code = NO_ERROR;
        data->closed = false;
        data->is_sending = false;
        data->client_fd = new_fd;
        data->client_address = client_address;
        data->next_state = -1;
        buffer_init(&data->read_buffer_client, BUFFER_SIZE, data->read_buffer_data);
        buffer_init(&data->write_buffer_client, BUFFER_SIZE, data->write_buffer_data);
        memset(data->user, 0, MAX_ARG_LEN + 1); // Set user buffer to null;

        stm_init(&data->stm);
}

void pop3_handle_write(struct selector_key *key)
{
        client_data *data = GET_DATA(key);
        const enum pop3_states status = stm_handler_write(&data->stm, key);
        if (status == ERROR_POP3 || status == DONE) {
                close_connection(key);
        }
}

void pop3_handle_read(struct selector_key *key)
{
        client_data *data = GET_DATA(key);
        const enum pop3_states status = stm_handler_read(&data->stm, key);
        if (status == ERROR_POP3 || status == DONE) {
                close_connection(key);
        }
}

void pop3_handle_close(struct selector_key *key)
{
        client_data *data = GET_DATA(key);
        stm_handler_close(&data->stm, key);
        close_connection(key);
}

void close_connection(struct selector_key *key)
{
        client_data *data = GET_DATA(key);
        //Already closed
        if (data->closed) {
                return;
        }

        data->closed = true;
        log(INFO, "Closing connection from socket %d", key->fd);

        selector_unregister_fd(key->s, key->fd);

        if (data != NULL) {
                // REMEMBER to free any allocated resources
                free(data);
        }

        close(key->fd);
}
#endif /* ifndef POP3 */
