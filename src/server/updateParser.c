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
#include <stdint.h>
#include <utils/logger.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>

#include <server/parsers/updateParser.h>

#define PASS_CMD "PASS"

static void final_arrival(struct selector_key *key, uint8_t c);
static void error_arrival(struct selector_key *key, uint8_t c);

enum update_states {
        S0 = 0,
        SERR,
};

static parser_state update_states[] = {
        { .id = S0, .is_final = true, .on_arrival = final_arrival },
        { .id = SERR, .is_final = true, .on_arrival = error_arrival },
};

static size_t total_transitions_per_state[] = {
        0, 0
        //    S0, SERR
};

static struct parser_transition *S0_transitions = NULL;
static struct parser_transition *SERR_transitions = NULL;
static struct parser_transition **transitions_list;

static parser_t update_inner_parser;

void conf_update_parser(void)
{
        transitions_list = malloc((SERR + 1) * sizeof(parser_transition *));
        transitions_list[S0] = S0_transitions;
        transitions_list[SERR] = SERR_transitions;

        for (int i = 0; i <= SERR; i++) {
                for (size_t j = 0; j < total_transitions_per_state[i]; j++) {
                        memset((void *)transitions_list[i][j].activators, 0,
                               ACTIVATORS_LEN * sizeof(bool));
                }
        }

        update_inner_parser.states = update_states;
        update_inner_parser.error_state = &update_states[SERR];
        update_inner_parser.initial_state = &update_states[S0];
        update_inner_parser.transitions = transitions_list;
        update_inner_parser.transitions_per_state = total_transitions_per_state;
}

void free_update_parser(void)
{
        free(transitions_list);
}

void init_update_parser(update_parser_t *parser)
{
        if (parser == NULL) {
                log(ERROR, "Trying to initialize NULL update parser");
                return;
        }

        parser->parser = &update_inner_parser;
        parser->state_id = S0;
        parser->ended = false;
        parser->quit = false;
        parser->err_value = NO_ERROR;
}

static void final_arrival(struct selector_key *key, uint8_t c)
{
        client_data *data = GET_DATA(key);
        update_parser_t *parser = &data->parser.update_parser;
        parser->ended = true;
}

static void error_arrival(struct selector_key *key, uint8_t c)
{
        client_data *data = GET_DATA(key);
        update_parser_t *parser = &data->parser.update_parser;
        parser->err_value = UNKNOWN_ERROR;
        parser->ended = true;
}
