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
} maildir_mail_t;

// Array of mails
typedef struct maildir_mails {
        maildir_mail_t *mails;
        unsigned len;
} maildir_mails_t;

// Maildir
typedef struct user_maildir {
        char user[MAX_ARG_LEN + 1]; // NULL terminated username

        maildir_mails_t new;

        char path[PATH_LEN]; // Path
        unsigned path_len; // length without "{cur,new,tmp}"
} user_maildir_t;

user_maildir_t *maildir_open(const char *username);
void maildir_close(user_maildir_t *maildir);

maildir_mails_t *maildir_list_new(user_maildir_t *maildir);

#endif // !MAILDIR_H
