#include <server/parsers/authPassParser.h>
#include <server/selector.h>
#include <server/pop3.h>
#include <utils/logger.h>

void initAuthPassword(const unsigned state, struct selector_key* key) {
    log(DEBUG, "Initializing Password Buffer");
    client_data* data = GET_DATA(key);
    init_auth_pass_parser(&data->parser.auth_pass_parser);
}


