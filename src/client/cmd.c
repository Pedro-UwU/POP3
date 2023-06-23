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
#include <stdio.h> // perror, snprintf
#include <stdlib.h> // malloc
#include <string.h> // strcmp
#include <sys/socket.h> // send
#include <client/spec.h>

#include <client/cmd.h>

#define ARR_LEN(x) (sizeof(x) / sizeof(*x))
#define BUFFER_SIZE (32 * 1024)

static const char *monitor_cmds[] = {

        "QUIT",          "LOGIN",          "GET_USERS",      "GET_USER",
        "GET_CURR_CONN", "GET_TOTAL_CONN", "GET_SENT_BYTES", "ADD_USER",
        "POPULATE_USER", "DELETE_USER",    "COMMANDS"

};

typedef struct buffer {
        char b[BUFFER_SIZE];
        int r;
        int w;

        char last[MONITOR_TERMINATOR_LEN];
} buffer_t;

static int send_cmd(int fd, monitor_cmd cmd, char *args);
static int recv_cmd(int fd);
static void buffer_write(buffer_t *b, char c);
static char buffer_read(buffer_t *b);
static bool has_terminator(buffer_t *b);

monitor_cmd get_cmd_index(const char *cmd)
{
        for (int i = 0; i < N_CMDS; i++) {
                if (0 == strcmp(cmd, monitor_cmds[i])) {
                        return i;
                }
        }

        return CMD_INVALID;
}

bool valid_cmd_argc(const monitor_cmd cmd, const int argc)
{
        if (cmd < 0 || cmd > N_CMDS)
                return false;

        return (argc == monitor_cmds_args[cmd]) ? true : false;
}

int cmd_exec(int fd, monitor_cmd cmd, char **args)
{
        switch (cmd) {
        case QUIT:
                cmd_quit(fd);
                break;

        case LOGIN:
                cmd_login(fd, args[0], args[1]);
                break;

        case GET_USERS:
                cmd_get_users(fd);
                break;

        case GET_USER:
                cmd_get_user(fd, args[0]);
                break;

        case GET_CURR_CONN:
                cmd_get_curr_conn(fd);
                break;

        case GET_TOTAL_CONN:
                cmd_get_total_conn(fd);
                break;

        case GET_SENT_BYTES:
                cmd_get_sent_bytes(fd);
                break;

        case ADD_USER:
                cmd_add_user(fd, args[0], args[1]);
                break;

        case POPULATE_USER:
                cmd_populate_user(fd, args[0]);
                break;

        case DELETE_USER:
                cmd_delete_user(fd, args[0]);
                break;

        case COMMANDS:
                cmd_commands(fd);
                break;

        default:
                return 1;
                break;
        }

        recv_cmd(fd);

        return 0;
}

int cmd_quit(int fd)
{
        return send_cmd(fd, QUIT, NULL);
}

int cmd_login(int fd, const char *user, const char *pass)
{
        char buf[1 + MONITOR_MAX_ARG_LEN * 2];
        snprintf(buf, 1 + MONITOR_MAX_ARG_LEN * 2, "%s%s%s", user, MONITOR_SEPARATOR, pass);

        return send_cmd(fd, LOGIN, buf);
}

int cmd_get_users(int fd)
{
        return send_cmd(fd, GET_USERS, NULL);
}

int cmd_get_user(int fd, const char *user)
{
        char buf[1 + MONITOR_MAX_ARG_LEN];
        snprintf(buf, MONITOR_MAX_ARG_LEN, "%s", user);

        return send_cmd(fd, GET_USER, buf);
}

int cmd_get_curr_conn(int fd)
{
        return send_cmd(fd, GET_CURR_CONN, NULL);
}

int cmd_get_total_conn(int fd)
{
        return send_cmd(fd, GET_TOTAL_CONN, NULL);
}

int cmd_get_sent_bytes(int fd)
{
        return send_cmd(fd, GET_SENT_BYTES, NULL);
}

int cmd_add_user(int fd, const char *user, const char *pass)
{
        char buf[1 + MONITOR_MAX_ARG_LEN * 2];
        snprintf(buf, MONITOR_MAX_ARG_LEN, "%s%s%s", user, MONITOR_SEPARATOR, pass);

        return send_cmd(fd, ADD_USER, buf);
}

int cmd_populate_user(int fd, const char *user)
{
        char buf[1 + MONITOR_MAX_ARG_LEN];
        snprintf(buf, MONITOR_MAX_ARG_LEN, "%s", user);

        return send_cmd(fd, POPULATE_USER, buf);
}

int cmd_delete_user(int fd, const char *user)
{
        char buf[1 + MONITOR_MAX_ARG_LEN];
        snprintf(buf, MONITOR_MAX_ARG_LEN, "%s", user);

        return send_cmd(fd, DELETE_USER, buf);
}

int cmd_commands(int fd)
{
        return send_cmd(fd, COMMANDS, NULL);
}

static int send_cmd(int fd, monitor_cmd cmd, char *args)
{
        size_t clen = strlen(monitor_cmds[cmd]);
        size_t send_len = clen + MONITOR_CMD_TERMINATOR_LEN + 1; // + '\0'

        char *buf = NULL;

        if (args != NULL) {
                send_len += strlen(args) + 1; // + MONITOR_SEPARATOR
        }

        buf = malloc(sizeof(char) * send_len);
        if (buf == NULL) {
                perror("malloc");
                return -1;
        }

        if (args == NULL)
                snprintf(buf, send_len, "%s%s", monitor_cmds[cmd], MONITOR_CMD_TERMINATOR);
        else
                snprintf(buf, send_len, "%s%s%s%s", monitor_cmds[cmd], MONITOR_SEPARATOR, args,
                         MONITOR_CMD_TERMINATOR);

        int ans = send(fd, buf, send_len - 1, 0); // Do not send '\0'
        if (ans < 0) {
                perror("send");
                fprintf(stderr, "Error while talking to monitor.\n");
        }

        free(buf);

        return ans;
}

static int recv_cmd(int fd)
{
        // Is this fast? No. Does it work? Yes
        buffer_t response = { 0 };
        char buf[BUFFER_SIZE] = { 0 };

        int ans = 0;

        while (ans >= 0 && has_terminator(&response) == false) {
                memset(buf, 0, BUFFER_SIZE * sizeof(char));

                ans = recv(fd, buf, BUFFER_SIZE - 1, 0);
                if (ans < 0) {
                        perror("recv");
                        fprintf(stderr, "\nError reading from fd %d\n", fd);
                        return -1;
                }

                for (ssize_t i = 0; i < ans; i++) {
                        buffer_write(&response, buf[i]);
                }

                while (response.w != response.r)
                        putchar(buffer_read(&response));
        }

        return 0;
}

static bool has_terminator(buffer_t *b)
{
        for (int i = 0; i < MONITOR_TERMINATOR_LEN; i++) {
                if (b->last[i] != MONITOR_TERMINATOR[i])
                        return false;
        }

        return true;
}

static void buffer_write(buffer_t *b, char c)
{
        b->b[b->w] = c;
        b->w = (b->w + 1) % BUFFER_SIZE;

        for (int i = 0; i < MONITOR_TERMINATOR_LEN - 1; i++) {
                b->last[i] = b->last[i + 1];
        }
        b->last[MONITOR_TERMINATOR_LEN - 1] = c;
}

static char buffer_read(buffer_t *b)
{
        char c = b->b[b->r];
        b->r = (b->r + 1) % BUFFER_SIZE;
        return c;
}
