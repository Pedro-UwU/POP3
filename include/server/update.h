#ifndef UPDATE_H
#define UPDATE_H

#include <server/selector.h>

void init_update(const unsigned state, struct selector_key *key);
unsigned update_process(struct selector_key *key);

#endif // !UPDATE_H
