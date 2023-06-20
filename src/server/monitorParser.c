#include <monitordef.h>
#include <server/buffer.h>
#include <ctype.h>
#include <server/parser.h>
#include <server/parsers/monitorParser.h>
#include <server/monitor.h>
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

enum monitor_states {
        S0 = 0,
        S1,
        S2,
        S3,
        SERR,
};

static parser_state monitor_states[] = {
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

static parser_t monitor_inner_parser;

void conf_monitor_parser(void)
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

        monitor_inner_parser.states = monitor_states;
        monitor_inner_parser.error_state = &monitor_states[SERR];
        monitor_inner_parser.initial_state = &monitor_states[S0];
        monitor_inner_parser.transitions = transitions_list;
        monitor_inner_parser.transitions_per_state = total_transitions_per_state;
}

void free_monitor_parser(void)
{
        free(transitions_list);
}

void init_monitor_parser(monitor_parser_t *parser)
{
        if (parser == NULL) {
                log(ERROR, "Trying to initialize NULL auth pass parser");
                return;
        }

        parser->parser = &monitor_inner_parser;
        parser->state_id = S0;
        parser->total_arg = 0;
        parser->total_cmd = 0;
        parser->ended = false;
        parser->quit = false;
        parser->err_value = MONITOR_NO_ERROR;
        memset(parser->cmd, 0, MONITOR_MAX_CMD_LEN);
        memset(parser->arg, 0, MONITOR_MAX_ARG_LEN);
}

int monitor_parse(struct selector_key *key, monitor_parser_t *monitor_parser, struct buffer *buffer)
{
        monitor_data *data = ((monitor_data *)(key)->data);
        int state = 0;
        size_t aux = 0;
        uint8_t* to_delete = buffer_read_ptr(buffer, &aux);
        while (data->monitor_parser.err_value == MONITOR_NO_ERROR && buffer_can_read(buffer) &&
               monitor_parser->ended != true) {
                char c = buffer_read(buffer);
                state = process_char(key, monitor_parser->parser, monitor_parser->state_id,
                                     c);
                uint8_t* to_delete2 = buffer_read_ptr(buffer, &aux);
                log(DEBUG, "Read %c (%d) and new state is %d. Pointer diff = %d - read_ptr = %p - write_ptr = %p", c, (int)c, state, (int)(to_delete2 - to_delete), buffer->read, buffer->write);
                monitor_parser->state_id = state;
        }
        //log(DEBUG, "Current Pass Parser State %ld", monitor_parser->state_id);
        return state;
}

static void save_cmd(struct selector_key *key, uint8_t c)
{
        if (!isalpha(c)) {
                return;
        }
        monitor_data *data = ((monitor_data *)(key)->data);
        monitor_parser_t *parser = &data->monitor_parser;
        if (parser->total_cmd >= MONITOR_MAX_CMD_LEN) {
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
        monitor_data *data = ((monitor_data *)(key)->data);
        monitor_parser_t *parser = &data->monitor_parser;
        if (parser->total_arg >= MONITOR_MAX_ARG_LEN) {
                parser->err_value = MONITOR_INVALID_ARG;
                return; // Invalid arg. No need to go to error
        }
        parser->arg[parser->total_arg] = c;
        parser->total_arg++;
        return;
}

static void final_arrival(struct selector_key *key, uint8_t c)
{
        monitor_data *data = ((monitor_data *)(key)->data);
        monitor_parser_t *parser = &data->monitor_parser;
        parser->ended = true;
}

static void error_arrival(struct selector_key *key, uint8_t c)
{
        monitor_data *data = ((monitor_data *)(key)->data);
        monitor_parser_t *parser = &data->monitor_parser;
        parser->err_value = MONITOR_INVALID_CHAR;
        parser->ended = true;
}
