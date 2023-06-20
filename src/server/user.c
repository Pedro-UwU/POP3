#include <server/user.h>
#include <pop3def.h>
#include <stddef.h>
#include <string.h>


static user_t users[MAX_USERS] = {0};


bool user_is_connected(const char *user)
{
        for (size_t i = 0; i < MAX_USERS; i++) {
                user_t aux_user = users[i];
                if (strcmp(user, aux_user.uname) == 0) {
                        if (users[i].state == USER_ONLINE)
                                return true;
                        return false;
                }
        }
        return false;
}

unsigned user_get_state(const char *user)
{
        for (size_t i = 0; i < MAX_USERS; i++) {
                user_t aux_user = users[i];
                if (strcmp(user, aux_user.uname) == 0) {
                        return users[i].state;
                }
        }
        return USER_NOT_FOUND;
}
void user_set_state(const char *user, unsigned state)
{
        if (state >= USER_NOT_FOUND) {
                return;
        }
        for (size_t i = 0; i < MAX_USERS; i++) {
                user_t aux_user = users[i];
                if (strcmp(user, aux_user.uname) == 0) {
                        users[i].state = state;
                }
        }
}

bool user_exists(const char *user)
{
        for (size_t i = 0; i < MAX_USERS; i++) {
                user_t aux_user = users[i];
                if (strcmp(user, aux_user.uname) == 0) {
                        return true;
                }
        }
        return false;
}
int user_add(const char *user, const char *pass)
{
        for (size_t i = 0; i < MAX_USERS; i++) {
                if (users[i].uname[0] == '\0') {
                        strcpy(users[i].uname, user);
                        strcpy(users[i].pass, pass);
                        users[i].state = USER_OFFLINE;
                        return 0;
                }
        }
        return -1; // Can't add user
}

bool user_check_pass(const char *user, const char *pass)
{
        for (size_t i = 0; i < MAX_USERS; i++) {
                user_t aux_user = users[i];
                if (strcmp(user, aux_user.uname) == 0) {
                        if (strcmp(pass, users[i].pass) == 0) {
                                return true;
                        }
                        return false;
                }
        }
        return false;
}
