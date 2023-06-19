#ifndef USER_H
#define USER_H

#include <stdbool.h>
enum user_states {
        USER_OFFLINE = 0, // Not connected
        USER_LOGGING, // USER command
        USER_ONLINE, // Connected
        USER_NOT_FOUND
};

bool user_is_connected(const char *user);
unsigned user_get_state(const char *user);
void user_set_state(const char *user, unsigned state);
bool user_exists(const char *user);
bool user_check_pass(const char *user, const char *pass);
int user_add(const char *user, const char *pass);

#endif // !USER_H
