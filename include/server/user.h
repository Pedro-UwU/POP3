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
#ifndef USER_H
#define USER_H
#include <stdbool.h>
#include <pop3def.h>

#define MAX_USERS 1022

typedef struct user_t {
        char uname[MAX_ARG_LEN + 1];
        char pass[MAX_ARG_LEN + 1];
        unsigned state;
} user_t;

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
int user_delete(const char *user);
struct user_t *get_user_array(void);

#endif // !USER_H
