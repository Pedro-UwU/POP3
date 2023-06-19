#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <server/selector.h>

void init_trans(const unsigned state, struct selector_key *key);

unsigned trans_read(struct selector_key *key);
unsigned trans_process(struct selector_key *key);

#endif // !TRANSACTION_H
