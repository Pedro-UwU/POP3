#include <stdbool.h>
#include <stdio.h>
#include <client/cmd.h>
#include <client/connect.h>
#include <utils/args.h>

void version()
{
        fprintf(stderr,
                "Monitor Protocol v0.0.1\n"
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
                "       -v              Print version\n"
                //                "       -p <port>       POP3 server port"
                //                "       -l <ip>         POP3 server ip"
                "       -P <port>       Monitor port\n"
                "       -L <ip>         Monitor ip\n"
                "\n"
                " Available COMMANDs:\n"
                "       LOGIN <user> <password>\n"
                "       GET_USERS\n"
                "       GET_USER <username>\n"
                "       GET_CURR_CONN\n"
                "       GET_TOTAL_CONN\n"
                "       GET_SENT_BYTES\n"
                "       ADD_USER <username> <password>\n"
                "       POPULATE_USER <username>\n"
                "       DELETE_USER <username>\n"
                "       COMMANDS\n",
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

        printf("OK\n");
        printf("\nargs->version = %d", args.version);
        printf("\nargs->monitor.port = %u", args.monitor.port);
        printf("\nargs->monitor.ip = %s", args.monitor.ip);
        printf("\nargs-server.port = %u", args.server.port);
        printf("\nargs-server.ip = %s", args.server.ip);
        printf("\nargs->user = %s", args.user);
        printf("\nargs->pass = %s", args.pass);
        for (int i = argcmd; i < argc; i++) {
                printf("\nCOMMAND %d: %s", i, argv[i]);
        }
        printf("\n");

        int socket_fd = connection_open(&args);

        if (cmd_login(socket_fd, args.user, args.pass) != 0) {
                fprintf(stderr, "Could not login.");
                return 1;
        }

        while (argc > argcmd) {
                monitor_cmd cmd = get_cmd_index(argv[argcmd]);
                if (cmd == CMD_INVALID) {
                        fprintf(stderr, "Invalid command: %s\n", argv[argcmd]);
                        return 1;
                }

                int cmd_argc = monitor_cmds_args[cmd];
                if (argc - argcmd < cmd_argc + 1) { // Remember to count COMMAND
                        fprintf(stderr, "Missing argument for command %s\n", argv[argcmd]);
                        return 1;
                }

                cmd_exec(socket_fd, cmd, argv + argcmd + 1);

                argcmd += 1 + cmd_argc;
        }

        cmd_quit(socket_fd);

        connection_close(socket_fd);

        return 0;
}
