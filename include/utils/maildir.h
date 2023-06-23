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
#ifndef MAILDIR_H
#define MAILDIR_H

#include <stdbool.h>
#include <stdint.h>

#include <pop3def.h>

// MAILDIR_ROOT + / + USERNAME + / + MAILDIR_DIR_NAME + / + {cur,new,tmp} + \0
#define PATH_LEN (13 + 8 + (MAX_ARG_LEN) + 1 + 3 + 1)

// Single mail
typedef struct maildir_mail {
        char *fname;
        unsigned flen;

        unsigned size;

        char *path;
        unsigned plen;

        bool read;
        bool del;
} maildir_mail_t;

// Array of mails
typedef struct maildir_mails {
        maildir_mail_t *mails;
        unsigned len;
        unsigned ndel;
} maildir_mails_t;

// Maildir
typedef struct user_maildir {
        char user[MAX_ARG_LEN + 1]; // NULL terminated username

        maildir_mails_t new;

        char path[PATH_LEN]; // Path
        unsigned path_len; // length without "{cur,new,tmp}"
} user_maildir_t;

int maildir_open(user_maildir_t *maildir, const char *username);
int maildir_set_username(user_maildir_t *maildir, const char *username);
void maildir_close(user_maildir_t *maildir);

maildir_mails_t *maildir_list_new(user_maildir_t *maildir);
int maildir_build(user_maildir_t *maildir);
int maildir_destroy(user_maildir_t *maildir_open);
void maildir_populate(user_maildir_t *maildir, unsigned n_mails);

bool maildir_is_read(maildir_mail_t *mail);
void maildir_set_read(maildir_mail_t *mail, bool r);
bool maildir_move_as_read(maildir_mail_t *mail);

bool maildir_is_del(maildir_mail_t *mail);
void maildir_set_del(maildir_mails_t *mails, unsigned n, bool d);
bool maildir_del_permanentely(maildir_mail_t *mail);

#endif // !MAILDIR_H
