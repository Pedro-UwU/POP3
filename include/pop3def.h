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
#ifndef POP3_DEF
#define POP3_DEF

#define MAX_CMD_LEN 5
#define MAX_ARG_LEN 40
#define MAX_ARGS 5
#define MAX_RSP_LEN 32768
#define MAX_BYTES_SEND 256
#define USER_CMD "USER"
#define QUIT_CMD "QUIT"
#define TERMINATOR ".\r\n"
#define TERMINATOR_LEN 3

enum POP3ErrCodes {
        NO_ERROR = 0,
        LONG_PARAM,
        LONG_CMD,
        INVALID_CMD,
        INVALID_ARG,
        INVALID_USER,
        INVALID_CHAR,
        WRONG_PASSWORD,
        WRONG_TIME_PASS_CMD,
        USER_ALREADY_ONLINE,
        /* ... */
        UNKNOWN_ERROR,
};

#endif // !POP3_DEF
