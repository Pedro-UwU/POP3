#include <server/parser.h>
#include <stdbool.h>
#include <stdint.h>
#include <utils/logger.h>
#include <stddef.h>

int process_char(struct selector_key* key, parser_t * parser, unsigned int current_state_id, uint8_t c) {
    if (parser == NULL) {
        log(ERROR, "NULL parser in process_char");
        return -1;
    }

    size_t activator_index = 0;
    
    if (c < 20 || c > 127) { // Printable ASCII
        if (c == '\r') {
            activator_index = ACTIVATORS_LEN - 2; 
        } else if (c == '\n') {
            activator_index = ACTIVATORS_LEN - 1;
        } else {
            log(ERROR, "Non Printable ASCII char code: %d", c);
            return -1;
        }
    } else {
        activator_index = c - ' ';
    }

    parser_transition* state_transitions = parser->transitions[current_state_id];    
    for (size_t i = 0; i < parser->transitions_per_state[current_state_id]; i++) {
        if (state_transitions[i].activators[activator_index] == true) {
            if (state_transitions[i].from_state->on_departure != NULL) {
                state_transitions[i].from_state->on_departure(key, c);
            }
            if (state_transitions[i].to_state->on_arrival != NULL) {
                state_transitions[i].to_state->on_arrival(key, c);
            }
            return state_transitions[i].to_state->id;
        }
    }

    //If not matching transition
    if (parser->states[current_state_id].on_departure != NULL) {
        parser->states[current_state_id].on_departure(key, c);
    }

    if (parser->error_state->on_arrival != NULL) {
        parser->error_state->on_arrival(key, c);
    }
    return parser->error_state->id;
}

void add_activator_range(parser_transition* transition, uint8_t start, uint8_t end) {
    if (transition == NULL) {
        return;
    }
    if (start == '\r' || start == '\n' || end == '\r' || end == '\n') {
        log(ERROR, "Cannot use \\r or \\n as one extreme for the activator range");
        return;
    }
    
    for (uint8_t i = start; i <= end; i++) {
        uint8_t c = start + i;
        size_t activator_index = 0;
        if (c < 20 || c > 127) { // Printable ASCII
            if (c == '\r') {
                activator_index = ACTIVATORS_LEN - 2; 
            } else if (c == '\n') {
                activator_index = ACTIVATORS_LEN - 1;
            } 
        } else {
            activator_index = c - ' ';
        }
        transition->activators[activator_index] = true;
    }
}

void add_activator_except(parser_transition* transition, uint8_t* exceptions, size_t total_exception) {
    if (transition == NULL) {
        return;
    }
    for (uint8_t i = 0; i < ACTIVATORS_LEN; i++) {
        transition->activators[i] = true;
    }
    for (size_t i = 0; i < total_exception; i++) {
        uint8_t c = exceptions[i];
        if (c == 0) break; // In case a Null terminated string is passed as a paramenter
        size_t activator_index = 0;
        if (c < 20 || c > 127) { // Printable ASCII
        if (c == '\r') {
            activator_index = ACTIVATORS_LEN - 2; 
            } else if (c == '\n') {
                activator_index = ACTIVATORS_LEN - 1;
            } 
        } else {
            activator_index = c - ' ';
        }

        transition->activators[activator_index] = false;
    }
}

void add_activator(parser_transition *transition, uint8_t c) {
    size_t activator_index = 0;
        if (c < 20 || c > 127) { // Printable ASCII
        if (c == '\r') {
            activator_index = ACTIVATORS_LEN - 2; 
            } else if (c == '\n') {
                activator_index = ACTIVATORS_LEN - 1;
            } 
        } else {
            activator_index = c - ' ';
        }
        transition->activators[activator_index] = true;
}

bool isFinal(parser_state* state) {
    return state->is_final;
}
