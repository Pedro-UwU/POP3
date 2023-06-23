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
#ifndef MONITOR_DEF_H
#define MONITOR_DEF_H

#define MONITOR_BUFFER_SIZE 32768
#define MONITOR_MAX_ARG_LEN 40
#define MONITOR_MAX_CMD_LEN 40

enum monitor_error_codes {
        MONITOR_NO_ERROR = 0,
        MONITOR_INVALID_CHAR,
        MONITOR_INVALID_CMD,
        MONITOR_INVALID_ARG,
        MONITOR_NOT_LOGGED,
        MONITOR_WRONG_LOGIN,
        MONITOR_INVALID_USER,
        MONITOR_ALREADY_LOGGED,
        MONITOR_NOT_USER_LIST,
        MONITOR_FULL_USERS,
        MONITOR_USER_EXISTS,
        MONITOR_USER_ONLINE,
        MONITOR_CANT_CREATE_MAILDIR,
        MONITOR_CANT_RM_MAILDIR,
        MONITOR_CMD_ERROR,
        MONITOR_POPULATE_ERROR,
        MONITOR_UNKNOWN_ERROR
};

#endif // !MONITOR_DEF_H
