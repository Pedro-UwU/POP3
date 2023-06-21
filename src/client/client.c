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
                "       -V              Print version\n"
                "       -P <port>       Monitor port\n"
                "       -L <ip>         Monitor ip\n"
                "\n"
                " Available COMMANDs:\n"
                "       GET_USERS                       List all user's status (online, offline)\n"
                "       GET_USER <username>             Show user status (online, offline, non existent)\n"
                "       GET_CURR_CONN                   Total number of active connections\n"
                "       GET_TOTAL_CONN                  Total number of connections since server went up\n"
                "       GET_SENT_BYTES                  Total number of sent bytes since server went up\n"
                "       ADD_USER <username> <password>  Add <username> and set <password> for it\n"
                "       POPULATE_USER <username>        Fill <username> Maildir with random mails\n"
                "       DELETE_USER <username>          Remove user <username>\n"
                "       COMMANDS                        List all commands\n",
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

        // Defaults
        if (args.user == 0)
                args.user = "admin";
        if (args.pass == 0)
                args.pass = "admin";

        int socket_fd = connection_open(&args);
        if (socket_fd < 0) {
                fprintf(stderr, "Could not connect to server.\n");
                return 1;
        }

        char *auth[2];
        auth[0] = args.user;
        auth[1] = args.pass;
        if (cmd_exec(socket_fd, LOGIN, auth) < 0) {
                fprintf(stderr, "Could not login.\n");
                connection_close(socket_fd);
                return 1;
        }

        while (argc > argcmd) {
                monitor_cmd cmd = get_cmd_index(argv[argcmd]);
                if (cmd == CMD_INVALID) {
                        fprintf(stderr, "Invalid command: %s\n", argv[argcmd]);
                        goto finally;
                }

                int cmd_argc = monitor_cmds_args[cmd];
                if (argc - argcmd < cmd_argc + 1) { // Take into account cmd too
                        fprintf(stderr, "Missing argument for command %s\n", argv[argcmd]);
                        goto finally;
                }

                cmd_exec(socket_fd, cmd, argv + argcmd + 1);

                argcmd += 1 + cmd_argc;
        }

finally:
        cmd_quit(socket_fd);

        connection_close(socket_fd);

        return 0;
}
