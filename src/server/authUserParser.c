#include "server/buffer.h"
#include <server/pop3.h>
#include <server/parser.h>
#include <server/parsers/authUserParser.h>
#include <server/selector.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <utils/logger.h>
#include <utils/stringUtils.h>
#include <stdint.h>
#include <string.h>

#define USER_CMD "USER"
#define QUIT_CMD "QUIT"


enum auth_user_states {
    S0 = 0,
    S1,
    S2, // USER command Written
    S3, // QUIT command Written
    S4, // Invalid command
    S5, // '\r' received
    S6, // '\n' after '\r'
    SERR, 
};

enum auth_errors {
    NO_ERROR = 0,
    LONG_PARAM,
    LONG_COMMAND,
    INVALID_USER,
    UNKNOWN_ERROR,
};

static parser_t auth_user_inner_parser;

static void saveCommand(struct selector_key* key, uint8_t c) {
    client_data* data = GET_DATA(key);
    auth_user_parser_t* auth_parser = &data->parser.auth_user_parser;
    if (auth_parser->total_cmd >= MAX_CMD_LEN) {
        auth_parser->needs_to_transit = SERR;
        auth_parser->error_code = LONG_COMMAND;
        return;
    }
    auth_parser->cmd[auth_parser->total_cmd] = c;
    auth_parser->total_cmd++;
    log(DEBUG, "Read %s", auth_parser->cmd);
}

static void checkCommand(struct selector_key* key, uint8_t c) {
    client_data* data = GET_DATA(key);
    auth_user_parser_t* auth_parser = &data->parser.auth_user_parser;
    convertToUpper(auth_parser->cmd); // POP3 is case insentive RFC 1939
    if (strcmp(auth_parser->cmd, USER_CMD) == 0) {
        log(DEBUG, "USER command detected");
        auth_parser->needs_to_transit = S2;
        //In case that the buffer had more info written
        buffer_compact(&data->write_buffer_client);
        return;
    }
    if (strcmp(auth_parser->cmd, QUIT_CMD) == 0) {
        auth_parser->needs_to_transit = S3;
        buffer_compact(&data->write_buffer_client);
        return;
    }
    auth_parser->needs_to_transit = S4;

    return; //TODO HANDLE ERROR
    
}

static void saveUserChar(struct selector_key* key, uint8_t c) {
    if (c == 0) { // Comes from S1
        return;
    }
    client_data* data = GET_DATA(key);
    auth_user_parser_t* auth_parser = &data->parser.auth_user_parser;
    if (auth_parser->total_uname >= MAX_USER_NAME) {
        auth_parser->needs_to_transit = SERR;        
        auth_parser->error_code = LONG_PARAM;
        return;
    }
    auth_parser->uname[auth_parser->total_uname] = c;
    auth_parser->total_uname++;
    return;
}

static void consumeChar(struct selector_key* key, uint8_t c) {
    return;
}

static void consumeCharAndSetError(struct selector_key* key, uint8_t c) {
    client_data* data = GET_DATA(key);
    auth_user_parser_t* auth_parser = &data->parser.auth_user_parser;
    if (auth_parser->error_code == NO_ERROR) {
        auth_parser->error_code = UNKNOWN_ERROR;
    }
    return;
}


static void processCommand(struct selector_key* key, uint8_t c) {
    client_data* data = GET_DATA(key);
    auth_user_parser_t* auth_parser = &data->parser.auth_user_parser;
    
    if (auth_parser->error_code != NO_ERROR || strcmp(auth_parser->cmd, QUIT_CMD) == 0) {
        auth_parser->ended = true;
    } else if (auth_parser->total_uname > 0 && strcmp(auth_parser->uname, "PEDRO") != 0) { // TODO check real username
        auth_parser->error_code = INVALID_USER;
    } else {
        auth_parser->user_found = true;
        log(DEBUG, "Logged user PEDRO");
    }

    auth_parser->ended = true;
}

static parser_state auth_states[] = {
    {
        .id = S0,
        .on_arrival = saveCommand,
    },
    {
        .id = S1,
        .on_arrival = checkCommand,
        .on_departure = NULL,
    },
    {
        .id = S2, 
        .on_arrival = saveUserChar,
    },
    {
        .id = S3, 
    },
    {
        .id = S4,
        .on_arrival = consumeChar,
    },
    {
        .id = S5,
    },
    {
        .id = S6,
        .is_final = true,
        .on_arrival = processCommand,
    },
    {
        .id = SERR,
        .on_arrival = consumeCharAndSetError,
        .is_final = false, // Has to consume everything until a \r\n
    }
};

static size_t S0_total_transitions = 3;
static struct parser_transition S0_transitions[] = {
    {.from_state = S0, .to_state = S0},
    {.from_state = S0, .to_state = S1},
    {.from_state = S0, .to_state = S5},
};

static size_t S1_total_transitions = 3;
static struct parser_transition S1_transitions[] = {
    {.from_state = S1, .to_state = S2},
    {.from_state = S1, .to_state = S3},
    {.from_state = S1, .to_state = S4},
};

static size_t S2_total_transitions = 2;
static struct parser_transition S2_transitions[] = {
    {.from_state = S2, .to_state = S2},
    {.from_state = S2, .to_state = S5},
};

static size_t S3_total_transitions = 1;
static struct parser_transition S3_transitions[] = {
    {.from_state = S3, .to_state = S5},
};

static size_t S4_total_transitions = 2;
static struct parser_transition S4_transitions[] = {
    {.from_state = S4, .to_state = S4},
    {.from_state = S4, .to_state = S5},
};

static size_t S5_total_transitions = 1;
static struct parser_transition S5_transitions[] = {
    {.from_state = S5, .to_state = S6},
};

static size_t SERR_total_transitions = 2;
static struct parser_transition SERR_transitions[] = {
    {.from_state = SERR, .to_state = SERR},
    {.from_state = SERR, .to_state = S5}
};

static struct parser_transition** transitions_list;
static size_t* transitions_per_state_list;

void conf_auth_user_parser(void) {
    transitions_per_state_list = malloc((SERR+1) *  sizeof(size_t));
    transitions_per_state_list[S0] = S0_total_transitions;
    transitions_per_state_list[S1] = S1_total_transitions;
    transitions_per_state_list[S2] = S2_total_transitions;
    transitions_per_state_list[S3] = S3_total_transitions;
    transitions_per_state_list[S4] = S4_total_transitions;
    transitions_per_state_list[S5] = S5_total_transitions;
    transitions_per_state_list[S6] = 0;
    transitions_per_state_list[SERR] = SERR_total_transitions;

    transitions_list = malloc((SERR+1) * sizeof(parser_transition*));
    transitions_list[S0] = S0_transitions;
    transitions_list[S1] = S1_transitions;
    transitions_list[S2] = S2_transitions;
    transitions_list[S3] = S3_transitions;
    transitions_list[S4] = S4_transitions;
    transitions_list[S5] = S5_transitions;
    transitions_list[S6] = NULL;
    transitions_list[SERR] = SERR_transitions;

    for (int i = 0; i < (SERR+1); i++) {
        for (size_t j = 0; j < transitions_per_state_list[i]; j++) {
            memset((void *)&transitions_list[i][j].activators, 0, ACTIVATORS_LEN);
        }
    }

    add_activator_except(&transitions_list[S0][0], (uint8_t *)" \r\n", strlen(" \r\n"));        // For the first command
    add_activator(&transitions_list[S0][1], ' ');                                               // Space after command
    add_activator(&transitions_list[S0][2], '\r');                                              // From S0 to S5 in case there's no space
    add_activator_except(&transitions_list[S2][0], (uint8_t*) "\r\n", 3);                       // From S2 to S2 saving the username
    add_activator(&transitions_list[S2][1], '\r');                                              // From S2 to S5
    add_activator(&transitions_list[S3][0], '\r');                                              // From S3 to S5 
    add_activator_except(&transitions_list[S4][0], (uint8_t*)"\r", 1);                          // From S4 to S4, consuming every char after Invalid command
    add_activator(&transitions_list[S4][1], '\r');                                              // From S4 to S5 
    add_activator(&transitions_list[S5][0], '\n');                                              // From S5 to S6, final state
                                                                                                //

    add_activator_except(&transitions_list[SERR][0], (uint8_t*)"\r", 1);                        // From SERR to SERR, consumes everything
    add_activator(&transitions_list[SERR][1], '\r');                                            // From SERR to S5

    auth_user_inner_parser.states = auth_states;
    auth_user_inner_parser.total_states = SERR;
    auth_user_inner_parser.initial_state = &auth_states[S0];
    auth_user_inner_parser.error_state = &auth_states[SERR];
    auth_user_inner_parser.transitions = transitions_list;
    auth_user_inner_parser.transitions_per_state = transitions_per_state_list;
}

void free_auth_user_parser_conf(void) {
    free(transitions_per_state_list);
    free(transitions_list);
}

void init_auth_user_parser(auth_user_parser_t *auth_parser) {
    if (auth_parser == NULL) {
        log(ERROR, "Trying to initialize NULL auth parser");
        return;
    }
    memset(&auth_parser->cmd, 0, MAX_CMD_LEN);
    memset(&auth_parser->uname, 0, MAX_USER_NAME);
    auth_parser->parser = &auth_user_inner_parser;
    auth_parser->state_id = auth_parser->parser->initial_state->id;
    auth_parser->total_cmd = 0;
    auth_parser->total_uname = 0;
    auth_parser->ended = false;
    auth_parser->quit = false;
    auth_parser->user_found = false;
    auth_parser->needs_to_transit = -1;
    auth_parser->error_code = NO_ERROR;
}

int auth_user_parse(struct selector_key* key, auth_user_parser_t* auth_parser, struct buffer* buffer) {
    int state = 0;
    while(buffer_can_read(buffer) && auth_parser->ended != true) {
        state = process_char(key, auth_parser->parser, auth_parser->state_id, buffer_read(buffer));
        auth_parser->state_id = state;
        if (state == -1 || auth_parser->parser->states[state].is_final == true)  {
            auth_parser->ended = true;
        } else if (auth_parser->needs_to_transit != -1) {
            state = transit_to(key, auth_parser->parser, auth_parser->state_id, auth_parser->needs_to_transit);
            auth_parser->state_id = state;
            auth_parser->needs_to_transit = -1;
            log(DEBUG, "Auth User Parser Transitioning to state: %d", state);
        }
    }
    log(DEBUG, "Current Parser State %ld", auth_parser->state_id);
    return state;
}
