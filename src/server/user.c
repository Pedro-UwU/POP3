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
#include <server/user.h>
#include <pop3def.h>
#include <stddef.h>
#include <string.h>

static user_t users[MAX_USERS] = { 0 };
static size_t total_users = 0;

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
        if (total_users == MAX_USERS) {
                return -1;
        }
        for (size_t i = 0; i < MAX_USERS; i++) {
                if (users[i].uname[0] == '\0') {
                        strcpy(users[i].uname, user);
                        strcpy(users[i].pass, pass);
                        users[i].state = USER_OFFLINE;
                        total_users++;
                        return 0;
                } else if (strcmp(user, users[i].uname) == 0) {
                        return -2; // User already exists
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

struct user_t *get_user_array(void)
{
        return users;
}

int user_delete(const char *user)
{
        if (user == NULL) {
                return -1; //
        }
        for (size_t i = 0; i < MAX_USERS; i++) {
                if (strcmp(user, users[i].uname) == 0) {
                        if (users[i].state != USER_OFFLINE) {
                                return -2;
                        }
                        memset(&users[i].uname, 0, MAX_ARG_LEN + 1);
                        memset(&users[i].pass, 0, MAX_ARG_LEN + 1);
                        users[i].state = USER_OFFLINE;
                        total_users--;
                        return 0;
                }
        }
        return -1;
}
