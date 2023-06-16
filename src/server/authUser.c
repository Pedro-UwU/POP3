#include "pop3def.h"
#include "server/buffer.h"
#include "utils/logger.h"
#include <server/pop3.h>
#include <server/selector.h>
#include <server/parsers/authUserParser.h>
#include <server/authUser.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

void initAuthUser(const unsigned state, struct selector_key* key) {
    log(DEBUG, "Initializing new Auth User Parser");
    client_data* data = GET_DATA(key);
    init_auth_user_parser(&data->parser.auth_user_parser);
}


unsigned auth_user_read(struct selector_key* key) {
    if (key == NULL) {
        log(ERROR, "auth_user_read NULL key");
        return ERROR_POP3;
    }

    client_data* data = GET_DATA(key);

    size_t read_limit = 0;
    uint8_t * readBuffer = buffer_write_ptr(&data->read_buffer_client, &read_limit);
    size_t read_count = recv(key->fd, readBuffer, read_limit, 0);
    if (read_count > 0) 
        log(DEBUG, "auth_user_read read %ld bytes", read_count);
    if (read_count < 0) {
        return ERROR_POP3;
    }
    if (read_count == 0) {
        log(DEBUG, "Socket %d closed connections", key->fd);
        return DONE;
    }

    buffer_write_adv(&data->read_buffer_client, read_count);

    int state = auth_user_parse(key, &data->parser.auth_user_parser, &data->read_buffer_client);


    if (state == -1) {
        return ERROR_POP3;
    } 
    if (data->parser.auth_user_parser.ended == true) {
        if (data->parser.auth_user_parser.error_code == 0) {
            return DONE;
        } else {
            return ERROR_POP3; // TODO GO TO AUTH_ERROR WITH AN ERROR CODE
        }
    }
    return AUTH_USER_READ;
}


unsigned auth_user_write(struct selector_key * key) {
    client_data* data = GET_DATA(key);
    if (data->err_code == NO_ERROR) {
        size_t written = 0;
        char buff[MAX_RSP_LEN] = {0};
        snprintf(buff, MAX_RSP_LEN, "+OK %s is a real hoopy frood" , data->user);
        size_t rsp_len = strlen(buff);
        written = send(key->fd, buff, MAX_RSP_LEN, 0);

        if (written != rsp_len) {
            log(ERROR, "Writing response to user, socket %d", key->fd);
            return UNKNOWN_ERROR;
        }

        // TODO check that if written > 0, should send the rest of the response in the next iteration 
        selector_set_interest_key(key, OP_READ); 
        return DONE;
    } else { // TODO send +ERR message
        return DONE;
    }

}
