#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#elif _POSIX_C_SOURCE < 200809L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h> // NULL
#include <dirent.h> // DIR
#include <string.h> // strcpy
#include <sys/stat.h> // stat

#include <utils/logger.h>
#include <utils/maildir.h>

#define MAILDIR_ROOT "bin/maildirs"

#define MAILDIR_DIR_NAME "Maildir"
#define MAILDIR_CUR "cur"
#define MAILDIR_NEW "new"
#define MAILDIR_TMP "tmp"

static void new_mail(maildir_mail_t *mail, const char *fname, const char *dir);

int maildir_open(user_maildir_t *maildir, const char *username)
{
        if (maildir == NULL) {
                log(ERROR, "NULL maildir");
                return 1;
        }

        memset(maildir, 0, sizeof(user_maildir_t));

        if (username != NULL) {
                strcpy(maildir->user, username);
                sprintf(maildir->path, "%s/%s/%s/", MAILDIR_ROOT, maildir->user, MAILDIR_DIR_NAME);
                maildir->path_len = strlen(maildir->path);
        }

        return 0;
}

int maildir_set_username(user_maildir_t *maildir, const char *username)
{
        if (maildir == NULL) {
                log(ERROR, "NULL username");
                return 1;
        }

        strcpy(maildir->user, username);
        sprintf(maildir->path, "%s/%s/%s/", MAILDIR_ROOT, maildir->user, MAILDIR_DIR_NAME);
        maildir->path_len = strlen(maildir->path);

        return 0;
}

void maildir_close(user_maildir_t *maildir)
{
        if (maildir == NULL)
                return;

        maildir->user[0] = '\0';

        if (maildir->new.mails != NULL) {
                for (unsigned i = 0; i < maildir->new.len; i++) {
                        if (maildir->new.mails[i].fname != NULL)
                                free(maildir->new.mails[i].fname);

                        maildir->new.mails[i].flen = 0;
                        maildir->new.mails[i].size = 0;

                        if (maildir->new.mails[i].path != NULL)
                                free(maildir->new.mails[i].path);

                        maildir->new.mails[i].plen = 0;
                }

                free(maildir->new.mails);
        }
        maildir->new.len = 0;
}

// List all
maildir_mails_t *maildir_list_new(user_maildir_t *maildir)
{
        if (maildir == NULL) {
                log(ERROR, "No maildir.");
                return NULL;
        }

        if (maildir->new.mails != NULL) {
                return &maildir->new;
        }

        DIR *d = NULL;
        struct dirent *dir;
        unsigned nfiles = 0;

        // user/Maildir/new/
        strcpy(&maildir->path[maildir->path_len], MAILDIR_NEW);

        d = opendir(maildir->path);
        if (d == NULL)
                return NULL;

        while ((dir = readdir(d)) != NULL) {
                if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
                        // Skip this dir and parent
                        continue;
                }
                nfiles++;
        }

        maildir->new.mails = calloc(nfiles, sizeof(maildir_mail_t));
        if (maildir->new.mails == NULL) {
                log(FATAL, "Could not allocate memory.");
                return NULL;
        }
        maildir->new.len = nfiles;

        rewinddir(d);

        int i = 0;
        while ((dir = readdir(d)) != NULL) {
                if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
                        continue;

                new_mail(&maildir->new.mails[i++], dir->d_name, maildir->path);
        }

        closedir(d);

        maildir->new.ndel = 0;

        return &maildir->new;
}

bool maildir_is_read(maildir_mail_t *mail)
{
        if (mail == NULL) {
                log(ERROR, "No mail.");
                return false;
        }

        return mail->read;
}

// Read/Unread mail
void maildir_set_read(maildir_mail_t *mail, bool r)
{
        if (mail == NULL) {
                log(ERROR, "No mail.");
                return;
        }

        mail->read = r;
}

bool maildir_is_del(maildir_mail_t *mail)
{
        if (mail == NULL) {
                log(ERROR, "No mail.");
                return false;
        }

        return mail->del;
}

// Delete/Undelete mail. Returns true if marked as deleted
void maildir_set_del(maildir_mails_t *mails, unsigned n, bool d)
{
        if (mails == NULL) {
                log(ERROR, "No mails.");
                return;
        }
        if (n > mails->len) {
                log(ERROR, "Invalid mail number.");
                return;
        }

        maildir_mail_t *m = &mails->mails[n];

        if (m->del == false && d == true)
                mails->ndel++;
        else if (m->del == true && d == false)
                mails->ndel--;

        m->del = d;
}

static void new_mail(maildir_mail_t *mail, const char *fname, const char *dir)
{
        struct stat st;

        mail->fname = strdup(fname);
        mail->flen = strlen(fname);

        // dir + / + file + \0
        int plen = strlen(dir) + mail->flen + 2;
        char *buf = malloc(plen * sizeof(char));
        if (buf == NULL) {
                log(FATAL, "Could not allocate memory.");
                return;
        }

        sprintf(buf, "%s/%s", dir, fname);

        mail->path = buf;
        mail->plen = plen;

        if (stat(mail->path, &st)) {
                log(ERROR, "Could not find mail: %s", mail->path);
                return;
        }

        mail->size = st.st_size;

        mail->read = false;
        mail->del = false;
}
