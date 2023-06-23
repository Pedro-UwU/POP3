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
#ifndef PARSER_H
#define PARSER_H
#include <server/selector.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// Delete - ' ' + 1 + \n + \r
#define ACTIVATORS_LEN (127 - 32 + 1 + 2)
typedef void (*parser_state_function)(struct selector_key *key, uint8_t c);

typedef struct parser_state {
        unsigned id;
        parser_state_function on_arrival;
        parser_state_function on_departure;
        bool is_final;
} parser_state;

typedef struct parser_transition {
        unsigned from_state;
        unsigned to_state;
        bool activators[ACTIVATORS_LEN];
} parser_transition;

typedef struct {
        parser_state *initial_state;
        parser_state *error_state;
        parser_state *states;
        parser_transition **transitions;
        size_t total_states;
        size_t *transitions_per_state;
} parser_t;

int process_char(struct selector_key *key, parser_t *paser, unsigned int current_state_id,
                 uint8_t c);
void add_activator_range(parser_transition *transition, uint8_t start, uint8_t end);
void add_activator_except(parser_transition *transition, uint8_t *exceptions,
                          size_t total_exception);
void add_activator(parser_transition *transition, uint8_t c);
bool isFinal(parser_state *state);
int transit_to(struct selector_key *key, parser_t *parser, unsigned int current_state_id,
               unsigned int to_state_id);

#endif // !PARSER_H
