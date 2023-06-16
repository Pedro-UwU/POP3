#ifndef AUTH_USER_H
#define AUTH_USER_H

#include <server/selector.h>
 
void initAuthUser(const unsigned state, struct selector_key* key);
unsigned auth_user_read(struct selector_key* key);
unsigned auth_user_write(struct selector_key* key);
#endif // !AUTH_USER_H
