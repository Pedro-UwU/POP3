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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <netinet/in.h>
#include <server/monitor.h>
#include <server/parsers/authParser.h>
#include <server/parsers/monitorParser.h>
#include <server/parsers/transParser.h>
#include <server/pop3.h>
#include <server/serverUtils.h>
#include <server/selector.h>
#include <server/user.h>
#include <server/parsers/authParser.h>
#include <server/parsers/transParser.h>
#include <server/parsers/updateParser.h>
#include <utils/args.h>
#include <time.h>
#include <utils/logger.h>
#include <server/fileReader.h>

int serverRunning = 1;

// Signal handler function
void handleSignal(int signal)
{
        if (signal == SIGINT || signal == SIGTSTP) {
                log(INFO, "Signal Received. Stopping server");
                serverRunning = 0;
        }
}

void version()
{
        fprintf(stderr,
                "POP3 Server & Monitor v0.0.1\n"
                "\t60711 - Pedro J. Lopez Guzman\n"
                "\t60401 - Martin E. Zahnd\n"
                "\nMIT License - 2023\n"
                "Copyright 2023 - Lopez Guzman, Zahnd\n"
                "\n"
                "Permission is hereby granted, free of charge, to any person obtaining a copy of\n"
                "this software and associated documentation files (the “Software”), to deal in\n"
                "the Software without restriction, including without limitation the rights to\n"
                "use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies\n"
                "of the Software, and to permit persons to whom the Software is furnished to do\n"
                "so, subject to the following conditions:\n"
                "\n"
                "The above copyright notice and this permission notice shall be included in all\n"
                "copies or substantial portions of the Software.\n"
                "\n"
                "THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
                "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
                "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
                "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
                "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
                "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
                "SOFTWARE.\n");
}

void help(const char *name)
{
        fprintf(stderr,
                "Usage: %s [OPTION] COMAND [ARGUMENTS]...\n"
                "\n"
                "       -h              Print help\n"
                "       -V              Print version\n"
                "       -c <exec>       Run executable <exec> before RETR\n"
                "       -p <port>       POP3 server port\n"
                "       -l <ip>         POP3 server ip\n"
                "       -P <port>       Monitor port\n"
                "       -L <ip>         Monitor ip\n"
                "       -v              Set DEBUG level (default is INFO)\n"
                "\n",
                name);
}

int main(int argc, char **argv)
{
        args_t args = {};
        int argcmd = parse_args(argc, argv, &args);
        if (argcmd < 0) {
                help(argv[0]);
                return 1;
        }

        if (args.version == true) {
                version();
                return 1;
        }

        if (args.debug == true)
                setLogLevel(DEBUG);
        else
                setLogLevel(INFO);

        if (args.ext_cmd != NULL) {
                log(DEBUG, "External command for mails is: %s", args.ext_cmd);
                set_external_program(args.ext_cmd);
        }

        init_monitor();

        if (args.user != 0 && args.pass != 0)
                user_add(args.user, args.pass);
        else
                user_add("USER1", "12345");

        close(0); // Nothing to read from stdin

        const char *err_msg = NULL;
        selector_status ss = SELECTOR_SUCCESS;
        fd_selector selector = NULL;

        // Create master POP3 socket
        int masterSocket = createTCPSocketServer(args.server.port_s, AF_INET);
        if (masterSocket < 0) {
                log(FATAL, "Couln't create master socket");
                goto finally;
        }
        log(DEBUG, "POP3 IPv4 socket created.");

        // Create master POP3 socket
        int masterSocket6 = createTCPSocketServer(args.server.port_s, AF_INET6);
        if (masterSocket6 < 0) {
                log(FATAL, "Couln't create master socket");
                goto finally;
        }
        log(DEBUG, "POP3 IPv6 socket created.");

        // Create master Monitor Socket
        int monitorSocket = createTCPSocketServer(args.monitor.port_s, AF_INET);
        if (monitorSocket < 0) {
                log(FATAL, "Couldn't create monitor socket");
                goto finally;
        }
        log(DEBUG, "Monitor socket created.");

        // Register signal handlers
        signal(SIGINT, handleSignal);
        signal(SIGTSTP, handleSignal);

        if (selector_fd_set_nio(masterSocket) == -1) {
                err_msg = "getting server socket flags";
                goto finally;
        }

        if (selector_fd_set_nio(masterSocket6) == -1) {
                err_msg = "getting server socket flags";
                goto finally;
        }

        if (selector_fd_set_nio(monitorSocket) == -1) {
                err_msg = "getting server socket flags";
                goto finally;
        }

        const struct selector_init conf = {
            .signal = SIGALRM,
            .select_timeout = {
                .tv_sec = 10,
                .tv_nsec = 0,
            },
        };

        if (0 != selector_init(&conf)) {
                err_msg = "Error on selector_init";
                goto finally;
        }

        selector = selector_new(1024);
        if (selector == NULL) {
                err_msg = "Unable to create selector";
                goto finally;
        }

        const struct fd_handler masterHandler = {
                .handle_read = pop3AcceptPassive,
                .handle_write = NULL,
                .handle_close = NULL,
        };

        ss = selector_register(selector, masterSocket, &masterHandler, OP_READ, NULL);

        if (ss != SELECTOR_SUCCESS) {
                err_msg = "Error registering master handler";
                goto finally;
        }

        ss = selector_register(selector, masterSocket6, &masterHandler, OP_READ, NULL);

        if (ss != SELECTOR_SUCCESS) {
                err_msg = "Error registering master handler IPv6";
                goto finally;
        }

        const struct fd_handler masterMonitorHandler = {
                .handle_read = acceptMonitorConnection,
                .handle_write = NULL,
                .handle_close = NULL,
        };

        ss = selector_register(selector, monitorSocket, &masterMonitorHandler, OP_READ, NULL);

        if (ss != SELECTOR_SUCCESS) {
                err_msg = "Error registering master monitor handler";
                goto finally;
        }

        conf_auth_parser();
        conf_trans_parser();
        conf_update_parser();
        conf_monitor_parser();
        while (serverRunning) {
                err_msg = NULL;
                ss = selector_select(selector);
                if (ss != SELECTOR_SUCCESS) {
                        err_msg = "Error serving";
                        goto finally;
                }
        }

        int ret = 0;

finally:
        if (ss != SELECTOR_SUCCESS) {
                log(ERROR, "%s: %s", (err_msg == NULL) ? "" : err_msg,
                    ss == SELECTOR_IO ? strerror(errno) : selector_error(ss));
                ret = 2;
        } else if (err_msg) {
                log(ERROR, "%s", err_msg);
                ret = 1;
        }

        if (selector != NULL) {
                selector_destroy(selector);
        }

        if (masterSocket >= 0) {
                close(masterSocket);
        }
        if (masterSocket6 >= 0) {
                close(masterSocket6);
        }
        if (monitorSocket >= 0) {
                close(monitorSocket);
        }

        selector_close();

        free_auth_parser();
        free_trans_parser();
        free_update_parser();
        free_monitor_parser();

        return ret;
}
