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
#ifndef CLIENT_SPEC_H
#define CLIENT_SPEC_H

#define MONITOR_TERMINATOR "\r\n\r\n"
#define MONITOR_TERMINATOR_LEN 4

#define MONITOR_SEPARATOR " "
#define MONITOR_CMD_TERMINATOR "\r\n"
#define MONITOR_CMD_TERMINATOR_LEN 2

#define MONITOR_MAX_ARG_LEN 40

#define MONITOR_ANSWER_OK "OwO"
#define MONITOR_ANSWER_ERR "UwU"

typedef enum {
        QUIT,
        LOGIN,
        GET_USERS,
        GET_USER,
        GET_CURR_CONN,
        GET_TOTAL_CONN,
        GET_SENT_BYTES,
        ADD_USER,
        POPULATE_USER,
        DELETE_USER,
        COMMANDS,
        N_CMDS,
        CMD_INVALID,
} monitor_cmd;

static const int monitor_cmds_args[] = {
        0, // QUIT
        2, // LOGIN <user> <password>
        0, // GET_USERS
        1, // GET_USER <username>
        0, // GET_CURR_CONN
        0, // GET_TOTAL_CONN
        0, // GET_SENT_BYTES
        2, // ADD_USER <username> <password>
        1, // POPULATE_USER <username>
        1, // DELETE_USER <username>
        0, // COMMANDS
};

#endif // !CLIENT_SPEC_H
