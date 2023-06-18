#include "server/buffer.h"
#include <pop3def.h>
#include <stdint.h>
#include <sys/socket.h>
#include <utils/logger.h>
#include <server/pop3.h>
#include <server/transaction.h>
#include <server/parsers/transParser.h>

void init_trans(const unsigned int state, struct selector_key *key) {
    client_data* data = GET_DATA(key);
    init_trans_parser(&data->parser.trans_parser);
}

unsigned trans_read(struct selector_key *key) {
    client_data* data = GET_DATA(key);
    if (data->is_sending == true) {
        log(ERROR, "WTF Shouldn't be in READ in transaction with socket %d",key->fd);
        selector_set_interest_key(key, OP_WRITE);
        return TRANSACTION;
    }
    buffer* data_buffer = &data->read_buffer_client;
    if (!buffer_can_write(data_buffer)) {
        buffer_compact(&data->read_buffer_client);
        if (buffer_can_write(data_buffer) == false) {
            // If after compact can't read, there's something wrong. Try  going to OP_WRITE to read the buffer
            if (buffer_can_read(&data->write_buffer_client)) {
                selector_set_interest_key(key, OP_WRITE);
                return TRANSACTION;
            }
            data->err_code = UNKNOWN_ERROR;
            return ERROR_POP3;
        }
    }

    size_t write_limit = 0;
    uint8_t* write_buffer = buffer_write_ptr(data_buffer, &write_limit);
    ssize_t read_count = recv(key->fd, write_buffer, write_limit, 0); // TODO Check flags
                                                                     
    if (read_count < 0) { // something went wrong 
        data->err_code = UNKNOWN_ERROR;
        return ERROR_POP3;
    }
    if (read_count == 0) { // Connection close
        return DONE;
    }
    buffer_write_adv(data_buffer, read_count);
    selector_set_interest_key(key, OP_WRITE);
    return AUTH;

}


unsigned trans_process(struct selector_key *key) {
    client_data* data = GET_DATA(key);
    // If sending: 
    //  -> to_send = len(msg)
    //  -> can_send = min(len(msg), MAX_BYTES_TO_SEND);
    //  -> send(msg, can_send);
    //  -> if sended == to_send
    //      -> if sending_file == true:
    //          selector_set_interest(data->file_key, OP_READ); // Tell the file reader to read again
    //          selector_set_interest(key, OP_NOOP); // 
    //      -> else:
    //          -> sending = false
    //  -> else:
    //      return TRANS  // Keep sending
    if (data->is_sending == true) {
        size_t bytes_to_send = 0;
        uint8_t* bytes = buffer_read_ptr(&data->write_buffer_client, &bytes_to_send);
        size_t can_send = bytes_to_send > MAX_BYTES_SEND ? MAX_BYTES_SEND : bytes_to_send;
        ssize_t sent = send(key->fd, bytes, can_send, 0);
        if (sent < 0) {
            data->err_code = UNKNOWN_ERROR;
            return ERROR_POP3;
        }
        buffer_read_adv(&data->write_buffer_client, sent);
        if ((size_t)sent == bytes_to_send) { // Everything was sent
            if (data->sending_file == true) {
                // SET INTEREST OF FILE SELECTOR TO READ AND SELF TO NOOP
            } else {
                data->is_sending = false;
                return data->next_state;
            }
        }
        return TRANSACTION;
    }

    //
    // 
    //
    // If not sending:
    //  -> if input_buffer is empty 
    //      -> return TRANS
    //  -> input = read_buffer(input_buffer)
    //  -> parse(input)
    //  -> if (parser is not finished)
    //      -> register OP_READ
    //      -> return TRANS
    //  -> handle_request(client_data);
    //  -> if (can_read(output_buffer)):
    //      -> send(output_buffer)
    //      -> if (sent < len(msg)):
    //          -> sending = true;
    //          -> return TRANS
    //      if (input_buffer is empty) 
    //          set_interes(key, OP_READ);
    //      }
    //      return data->next_state
    //  -> WTF, Shouldn't be here, but ok
    //  -> set_interest(key, OP_READ);
    //  -> return TRANS
    if (buffer_can_read(&data->read_buffer_client) == false) {
        selector_set_interest_key(key, OP_READ);
        return TRANSACTION;
    }
    size_t input_read = 0;
    uint8_t* input_bytest = buffer_read_ptr(&data->read_buffer_client, &input_read);

    trans_parser_t* parser = &data->parser.trans_parser;
    trans_parse(key, parser,  &data->read_buffer_client);
    if (parser->ended == false) {
        selector_set_interest_key(key, OP_READ);
        return TRANSACTION;
    }
    

    



}
