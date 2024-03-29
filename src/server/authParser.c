/**
 * MIT License - 2023
 * Copyright 2023 - Lopez Guzman, Zahnd
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the “Software”), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "server/buffer.h"
#include <ctype.h>
#include <pop3def.h>
#include <server/pop3.h>
#include <server/parser.h>
#include <server/parsers/authParser.h>
#include <stdint.h>
#include <utils/logger.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>

#define PASS_CMD "PASS"

static void save_cmd(struct selector_key *key, uint8_t c);
static void save_arg(struct selector_key *key, uint8_t c);
static void final_arrival(struct selector_key *key, uint8_t c);
static void error_arrival(struct selector_key *key, uint8_t c);

enum auth_states {
        S0 = 0,
        S1,
        S2,
        S3,
        SERR,
};

static parser_state auth_states[] = {
        { .id = S0, .on_departure = save_cmd },
        { .id = S1, .on_departure = save_arg },
        { .id = S2 },
        { .id = S3, .is_final = true, .on_arrival = final_arrival },
        { .id = SERR, .is_final = true, .on_arrival = error_arrival },
};

static size_t total_transitions_per_state[] = {
        3, 2, 1, 0, 0
        //    S0,  S1,  S2,  S3,  SERR
};

static struct parser_transition S0_transitions[] = {
        { .from_state = S0, .to_state = S0 },
        { .from_state = S0, .to_state = S1 },
        { .from_state = S0, .to_state = S2 },
};
static struct parser_transition S1_transitions[] = {
        { .from_state = S1, .to_state = S1 },
        { .from_state = S1, .to_state = S2 },
};
static struct parser_transition S2_transitions[] = {
        { .from_state = S2, .to_state = S3 },
};
static struct parser_transition *S3_transitions = NULL;
static struct parser_transition *SERR_transitions = NULL;
static struct parser_transition **transitions_list;

static parser_t auth_inner_parser;

void conf_auth_parser(void)
{
        transitions_list = malloc((SERR + 1) * sizeof(parser_transition *));
        transitions_list[S0] = S0_transitions;
        transitions_list[S1] = S1_transitions;
        transitions_list[S2] = S2_transitions;
        transitions_list[S3] = S3_transitions;
        transitions_list[SERR] = SERR_transitions;

        for (int i = 0; i <= SERR; i++) {
                for (size_t j = 0; j < total_transitions_per_state[i]; j++) {
                        memset((void *)transitions_list[i][j].activators, 0,
                               ACTIVATORS_LEN * sizeof(bool));
                }
        }

        add_activator_except(&S0_transitions[0], (uint8_t *)" \r\n",
                             3); // From S1 to S1. Saving the command
        add_activator(&S0_transitions[1], ' '); // From S0 to S1. Command saved
        add_activator(&S0_transitions[2], '\r'); // From S0 to S2. Command withoyt args
        add_activator_except(&S1_transitions[0], (uint8_t *)"\r\n", 2); // From S1 to S1. Saving arg
        add_activator(&S1_transitions[1], '\r'); // From S1 to S2. End of args
        add_activator(&S2_transitions[0], '\n'); // From S2 to S3. \n after \r. Final

        auth_inner_parser.states = auth_states;
        auth_inner_parser.error_state = &auth_states[SERR];
        auth_inner_parser.initial_state = &auth_states[S0];
        auth_inner_parser.transitions = transitions_list;
        auth_inner_parser.transitions_per_state = total_transitions_per_state;
}

void free_auth_parser(void)
{
        free(transitions_list);
}

void init_auth_parser(auth_parser_t *parser)
{
        if (parser == NULL) {
                log(ERROR, "Trying to initialize NULL auth pass parser");
                return;
        }

        parser->parser = &auth_inner_parser;
        parser->state_id = S0;
        parser->total_arg = 0;
        parser->total_cmd = 0;
        parser->ended = false;
        parser->quit = false;
        parser->err_value = NO_ERROR;
        memset(parser->cmd, 0, MAX_CMD_LEN);
        memset(parser->arg, 0, MAX_ARG_LEN);
}

int auth_parse(struct selector_key *key, auth_parser_t *auth_parser, struct buffer *buffer)
{
        client_data *data = GET_DATA(key);
        int state = 0;
        while (data->parser.auth_parser.err_value == NO_ERROR && buffer_can_read(buffer) &&
               auth_parser->ended != true) {
                state = process_char(key, auth_parser->parser, auth_parser->state_id,
                                     buffer_read(buffer));
                auth_parser->state_id = state;
        }
        log(DEBUG, "Current Pass Parser State %ld", auth_parser->state_id);
        return state;
}

static void save_cmd(struct selector_key *key, uint8_t c)
{
        if (!isalpha(c)) {
                return;
        }
        client_data *data = GET_DATA(key);
        auth_parser_t *parser = &data->parser.auth_parser;
        if (parser->total_cmd >= MAX_CMD_LEN) {
                return; // Invalid command. No need to go to error
        }
        parser->cmd[parser->total_cmd] = c;
        parser->total_cmd++;
        return;
}

static void save_arg(struct selector_key *key, uint8_t c)
{
        if (!isprint(c)) {
                return;
        }
        client_data *data = GET_DATA(key);
        auth_parser_t *parser = &data->parser.auth_parser;
        if (parser->total_arg >= MAX_ARG_LEN) {
                parser->err_value = INVALID_ARG;
                return; // Invalid arg. No need to go to error
        }
        parser->arg[parser->total_arg] = c;
        parser->total_arg++;
        return;
}

static void final_arrival(struct selector_key *key, uint8_t c)
{
        client_data *data = GET_DATA(key);
        auth_parser_t *parser = &data->parser.auth_parser;
        parser->ended = true;
}

static void error_arrival(struct selector_key *key, uint8_t c)
{
        client_data *data = GET_DATA(key);
        auth_parser_t *parser = &data->parser.auth_parser;
        parser->err_value = INVALID_CHAR;
        parser->ended = true;
}
