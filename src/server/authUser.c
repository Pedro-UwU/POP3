#include "server/buffer.h"
#include "utils/logger.h"
#include <server/pop3.h>
#include <server/selector.h>
#include <server/parsers/authUserParser.h>
#include <server/authUser.h>
#include <stddef.h>
#include <stdint.h>

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
    log(DEBUG, "read_limit: %d", (int)read_limit);
    size_t read_count = recv(key->fd, readBuffer, read_limit, 0);
    if (read_count > 0) 
        log(DEBUG, "auth_user_read read %ld bytes", read_count);
    if (read_count < 0) {
        return ERROR_POP3;
    }

    buffer_write_adv(&data->read_buffer_client, read_count);

    int state = auth_user_parse(key, &data->parser.auth_user_parser, &data->read_buffer_client);


    if (state == -1) {
        return ERROR_POP3;
    } 
    if (data->parser.auth_user_parser.ended == true) {
        return DONE;
    }
    return AUTH_USER_READ;
}
