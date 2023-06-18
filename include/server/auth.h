#ifndef AUTH_H
#define AUTH_H

#include <server/selector.h>
#include <server/pop3.h>
#include <pop3def.h>

void init_auth(const unsigned state, struct selector_key* key);
unsigned auth_read(struct selector_key* key);
unsigned auth_process(struct selector_key* key);

#endif // !AUTH_H
