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
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#elif _POSIX_C_SOURCE < 200809L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h> // NULL
#include <dirent.h> // DIR
#include <string.h> // strcpy
#include <sys/stat.h> // stat, mkdir
#include <stdlib.h> // mkstemp
#include <unistd.h> // close
#include <fcntl.h> // open
#include <time.h> // time

#include <utils/logger.h>
#include <utils/maildir.h>

#define MAILDIR_ROOT "bin/maildirs"

#define MAILDIR_DIR_NAME "Maildir"
#define MAILDIR_CUR "cur"
#define MAILDIR_NEW "new"
#define MAILDIR_TMP "tmp"

inline static void set_path_folder_md(user_maildir_t *md, char *folder);
inline static void set_path_folder(char *path, unsigned len, char *folder);
static void new_mail(maildir_mail_t *mail, const char *fname, const char *dir);
static int create_dir(char *path);
static char *append_flags(char *flags, char *filename, unsigned *len);
static char *move_and_set_flags(bool *success, maildir_mail_t *mail, char *flags);

int maildir_open(user_maildir_t *maildir, const char *username)
{
        if (maildir == NULL) {
                log(ERROR, "NULL maildir");
                return 1;
        }

        memset(maildir, 0, sizeof(user_maildir_t));

        if (username != NULL)
                maildir_set_username(maildir, username);

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

        if (create_dir(maildir->path) == -1)
                return -1;

        if (maildir_build(maildir) == -1)
                return -1;

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

char *maildir_get_path(user_maildir_t *maildir)
{
        if (maildir == NULL) {
                log(ERROR, "NULL maildir");
                return NULL;
        }

        maildir->path[maildir->path_len] = '\0';

        return strdup(maildir->path);
}

int maildir_build(user_maildir_t *maildir)
{
        if (maildir == NULL) {
                log(ERROR, "NULL maildir");
                return -2;
        }
        if (maildir->user[0] == '\0') {
                log(ERROR, "NULL user. Forgot to call maildir_set_username?");
                return -2;
        }

        // user/Maildir/cur/
        set_path_folder_md(maildir, MAILDIR_CUR);
        if (create_dir(maildir->path) == -1) {
                return -1;
        }

        // user/Maildir/new/
        set_path_folder_md(maildir, MAILDIR_NEW);
        if (create_dir(maildir->path) == -1) {
                return -1;
        }

        // user/Maildir/tmp/
        set_path_folder_md(maildir, MAILDIR_TMP);
        if (create_dir(maildir->path) == -1) {
                return -1;
        }
        return 0;
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
        set_path_folder_md(maildir, MAILDIR_NEW);

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

bool maildir_move_as_read(maildir_mail_t *mail)
{
        if (mail == NULL) {
                log(ERROR, "NULL mail");
                return false;
        }

        bool success = false;
        char *np = move_and_set_flags(&success, mail, "S");
        if (np != NULL)
                free(np);

        return success;
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

bool maildir_del_permanentely(maildir_mail_t *mail)
{
        if (mail == NULL) {
                log(ERROR, "NULL mail");
                return false;
        }

        char *np = NULL;
        bool success = false;

        // To comply with Maildir, it should be flagged as Trashed (T) first
        if (maildir_is_read(mail) == true)
                np = move_and_set_flags(&success, mail, "ST");
        else
                np = move_and_set_flags(&success, mail, "T");

        remove(np);
        free(np);

        return success;
}

inline static void set_path_folder_md(user_maildir_t *md, char *folder)
{
        strcpy(&md->path[md->path_len], folder);
}

inline static void set_path_folder(char *path, unsigned len, char *folder)
{
        memcpy(&path[len], folder, strlen(folder));
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
        mail->plen = plen - 1; // - '\0'

        if (stat(mail->path, &st)) {
                log(ERROR, "Could not find mail: %s", mail->path);
                return;
        }

        mail->size = st.st_size;

        mail->read = false;
        mail->del = false;
}

static int create_dir(char *path)
{
        struct stat st;
        long len = strlen(path);
        char *p = path;

        // Recursively create dir with parents
        do {
                p++;

                char *tmp = strchr(p, '/');
                if (tmp == NULL) {
                        p = path + len + 1;
                } else {
                        p = tmp;
                        *p = '\0';
                }

                if (stat(path, &st) != 0) {
                        if (mkdir(path, 0777) != 0) {
                                log(ERROR, "Could not create dir '%s'", path);
                                return -1;
                        }
                }

                if (p - path <= len)
                        *p = '/';

        } while (p - path < len);

        return 0;
}

static char *append_flags(char *flags, char *filename, unsigned *len)
{
        // ":2," + flags
        unsigned flags_len = strlen(flags) + 3;

        char *buf = realloc(filename, sizeof(char) * (*len + flags_len + 1));
        if (buf == NULL) {
                log(FATAL, "Could not allocate memory");
                goto finally;
        }

        sprintf(buf + *len, ":2,%s", flags);
        *len = strlen(buf);

finally:
        return buf;
}

static char *move_and_set_flags(bool *success, maildir_mail_t *mail, char *flags)
{
        unsigned np_len = mail->plen;
        char *new_path = strdup(mail->path);
        if (new_path == NULL) {
                log(FATAL, "Could not allocate memory");
                *success = false;
                return NULL;
        }

        // 4 = "new" + /
        set_path_folder(new_path, mail->plen - mail->flen - 4, MAILDIR_CUR);

        char *with_flags = append_flags(flags, new_path, &np_len);
        if (with_flags == NULL) {
                free(new_path);
                *success = false;
                return NULL;
        }

        rename(mail->path, with_flags);

        *success = true;
        return with_flags;
}

int maildir_destroy(user_maildir_t *maildir)
{
        if (maildir == NULL) {
                log(ERROR, "NULL maildir");
                return -2;
        }
        if (maildir->user[0] == '\0') {
                log(ERROR, "NULL user. Forgot to call maildir_set_username?");
                return -2;
        }
        DIR *dir = opendir(maildir->path);
        if (dir != NULL) {
                closedir(dir);
                int remove_success = remove(maildir->path);
                return remove_success;
        }
        free(maildir->new.mails);
        return -3;
}
