#include <server/user.h>
#include <pop3def.h>
#include <stddef.h>
#include <string.h>

#define MAX_USERS 512

static char users[MAX_USERS][MAX_ARG_LEN] = { { 0 } };
static char passwords[MAX_USERS][MAX_ARG_LEN] = { { 0 } };
static unsigned states[MAX_USERS] = { 0 };
static size_t total_users = 0;

bool user_is_connected(const char *user)
{
        for (size_t i = 0; i < total_users; i++) {
                char *aux_user = users[i];
                if (strcmp(user, aux_user) == 0) {
                        if (states[i] == USER_ONLINE)
                                return true;
                        return false;
                }
        }
        return false;
}

unsigned user_get_state(const char *user)
{
        for (size_t i = 0; i < total_users; i++) {
                char *aux_user = users[i];
                if (strcmp(user, aux_user) == 0) {
                        return states[i];
                }
                return USER_NOT_FOUND;
        }
        return USER_NOT_FOUND;
}
void user_set_state(const char *user, unsigned state)
{
        if (state >= USER_NOT_FOUND) {
                return;
        }
        for (size_t i = 0; i < total_users; i++) {
                char *aux_user = users[i];
                if (strcmp(user, aux_user) == 0) {
                        states[i] = state;
                }
        }
}

bool user_exists(const char *user)
{
        for (size_t i = 0; i < total_users; i++) {
                char *aux_user = users[i];
                if (strcmp(user, aux_user) == 0) {
                        return true;
                }
        }
        return false;
}
int user_add(const char *user, const char *pass)
{
        for (size_t i = 0; i < MAX_USERS; i++) {
                if (users[i][0] == 0) {
                        strcpy(users[i], user);
                        strcpy(passwords[i], pass);
                        states[i] = USER_OFFLINE;
                        total_users++;
                        return 0;
                }
        }
        return -1; // Can't add user
}

bool user_check_pass(const char *user, const char *pass)
{
        for (size_t i = 0; i < total_users; i++) {
                char *aux_user = users[i];
                if (strcmp(user, aux_user) == 0) {
                        if (strcmp(pass, passwords[i]) == 0) {
                                return true;
                        }
                        return false;
                }
        }
        return false;
}
