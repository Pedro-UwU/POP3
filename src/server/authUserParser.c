#include <server/pop3.h>
#include <server/parser.h>
#include <server/parsers/authUserParser.h>
#include <server/selector.h>
#include <stddef.h>
#include <stdlib.h>
#include <utils/logger.h>
#include <stdint.h>
#include <string.h>

#define USER_CMD "USER"

enum auth_user_states {
    S0 = 0,
    S1,
    SERR, 
};

static size_t S0_TOTAL_TRANSITIONS = 2;

static parser_t auth_user_inner_parser;

static void saveCommand(struct selector_key* key, uint8_t c) {
    client_data* data = GET_DATA(key);
    auth_user_parser_t* auth_parser = &data->parser.auth_user_parser;
    if (auth_parser->total_cmd >= MAX_CMD_LEN) {
        log(DEBUG, "Command to long");
        return;
    }
    auth_parser->cmd[auth_parser->total_cmd] = c;
}

static void checkCommand(struct selector_key* key, uint8_t c) {
    client_data* data = GET_DATA(key);
    auth_user_parser_t* auth_parser = &data->parser.auth_user_parser;
    if (strcmp(USER_CMD, auth_parser->cmd) != 0) {
        log(DEBUG, "ERROR, invalid command");
        // goto error;
    }
    log(DEBUG, "USER Command detected");
}

static parser_state auth_states[] = {
    {
        .id = S0,
        .on_arrival = saveCommand,
    },
    {
        .id = S1,
        .on_arrival = checkCommand,
    },
    {
        .id = SERR
    }
};

static parser_transition S0_S0 = {
        .from_state = &auth_states[S0],
        .to_state = &auth_states[S1],
};

static parser_transition S0_S1 = {
        .from_state = &auth_states[S1],
        .to_state = &auth_states[S1]
};

static parser_transition** transitions_list;
static size_t* transitions_per_state_list;

void conf_auth_user_parser(void) {

    add_activator(&S0_S1, ' ');
    add_activator_except(&S0_S0, (uint8_t*)"\r\n", 2);

    transitions_per_state_list = malloc((SERR+1) *  sizeof(size_t));
    transitions_per_state_list[S0] = S0_TOTAL_TRANSITIONS * sizeof(parser_transition);
    transitions_per_state_list[S1] = 0;
    transitions_per_state_list[SERR] = 0;

    parser_transition* S0_trans = malloc(transitions_per_state_list[S0]);
    parser_transition* S1_trans = malloc(transitions_per_state_list[S1]);
    parser_transition* SERR_trans = malloc(transitions_per_state_list[SERR]);


    transitions_list = malloc((SERR+1) * sizeof(parser_transition*));
    transitions_list[S0] = S0_trans;
    transitions_list[S1] = S1_trans;
    transitions_list[SERR] = SERR_trans;


    auth_user_inner_parser.states = auth_states;
    auth_user_inner_parser.total_states = SERR;
    auth_user_inner_parser.initial_state = &auth_states[S0];
    auth_user_inner_parser.error_state = &auth_states[SERR];
    auth_user_inner_parser.transitions = transitions_list;
    auth_user_inner_parser.transitions_per_state = transitions_per_state_list;
}

void free_auth_user_parser_conf(void) {

    for (int i = 0; i < SERR+1; i++) {
        free(transitions_list[i]);
    }
    free(transitions_per_state_list);
    free(transitions_list);
}

void init_auth_user_parser(auth_user_parser_t *auth_parser) {
    
}
