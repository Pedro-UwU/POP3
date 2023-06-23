# POP3 Server & Monitor

Version: 0.0.1

- 60711 - Pedro J. Lopez Guzman
- 60401 - Martin E. Zahnd

## Other documents

[Monitor Protocol](docs/protocol.md)

## Requirements

### Necessary

- Linux (tested on Ubuntu 22.04 and Arch Linux)
- GNU Make >= 4.3
- Bash >= 5.1.16 (for running tests)
- [curl](https://curl.se/) >= 7.81.0 (for tests)
- [OpenBSD netcat](https://manpages.ubuntu.com/manpages/impish/man1/nc.traditional.1.html)

### Optional

- clang-format if you want to contribute (for `make format`).
- [Bear](https://github.com/rizsotto/Bear): compilation database for clang tooling (usually not
  needed).

# Compilation

Run

```
make
```

in the project root to compile the sources.

Note that `make utils`, `make server` and `make client` are also available, but must be run from the
project's root directory.

# Execution

Both the client and server executables are created inside `./bin` directory.

## Server

Running the server is as simple as executing `./bin/server`.
The following options are also available:

```
Usage: ./bin/server [OPTION] COMAND [ARGUMENTS]...

       -h                      Print help
       -V                      Print version
       -u <user>:<pass>        Set <user> as the default user
       -c <exec>               Run executable <exec> before RETR
       -p <port>               POP3 server port
       -l <ip>                 POP3 server ip
       -P <port>               Monitor port
       -L <ip>                 Monitor ip
       -v                      Set DEBUG level (default is INFO)
```

Argument `-c` is intended mainly for filtering: it will make the server answer `RETR` commands with
the output of the given executable.

For example, if the server is started with `./bin/server -c md5sum`, when USER1 `RETR`s his fifth mail
it will receive

```
7901f5e491ae6a4f14361337ae3d7410  bin/maildirs/USER1/Maildir/new/1687475064_mail5

.
```

as output.

## Client

For running the client, type `./bin/client`. (Make sure the server is up first!)

This should print the following output before exiting

```
OwO Successfully logged

```

The client requires all commands to be provided as arguments **in the exact order they should be
run**.
It will connect to the monitor, execute the commands in order, and finish the monitor session.

Executing `./bin/client COMMANDS` will output all available commands that the monitor supports.
Note that `LOGIN` and `QUIT` refer to the monitor's commands, not the POP3 ones (in the case of
`QUIT`), and are automatically called at the beginning and end of the connection.

The following options are available:

```
Usage: ./bin/client [OPTION] COMAND [ARGUMENTS]...

       -h              Print help
       -V              Print version
       -P <port>       Monitor port
       -L <ip>         Monitor ip

 Available COMMANDs:
       GET_USERS                       List all user's status (online, offline)
       GET_USER <username>             Show user status (online, offline, non existent)
       GET_CURR_CONN                   Total number of active connections
       GET_TOTAL_CONN                  Total number of connections since server went up
       GET_SENT_BYTES                  Total number of sent bytes since server went up
       ADD_USER <username> <password>  Add <username> and set <password> for it
       POPULATE_USER <username>        Fill <username> Maildir with random mails
       DELETE_USER <username>          Remove user <username>
       COMMANDS                        List all commands
```

### Example execution

```bash
## $
./bin/client GET_USERS ADD_USER pepe 60401 ADD_USER pdihax 60711 POPULATE_USER pepe POPULATE_USER pdihax GET_USER pepe GET_USERS
#
## Output:
#
OwO Successfully logged

OwO
USER1 OFFLINE

OwO User pepe added

OwO User pdihax added

OwO User populated

OwO User populated

OwO pepe OFFLINE

OwO
USER1 OFFLINE
pepe OFFLINE
pdihax OFFLINE

```

This execution:

1. Lists all users in the server.
2. Adds user 'pepe' with password '60401'.
3. Adds user 'pdihax' with password '60711'.
4. Fills (populates) both users 'new' folder in their Maildir with mails containing random ASCII
   characters.
5. Asks for the current state of 'pepe'.
6. Asks for the state of all users.

# Test

In `./test` there are some Bash scripts used for testing the server and monitor functionality:

## `create_maildir.sh`

Creates files with unique names and random size (between 1 and 65535 bytes) in `USERNAME`'s `new` folder.

## `test_500_users.sh`

Adds 500 different users using the client and connects to them with curl.

## `test_connections.sh`

> **WARNING**
>
> This test grows the Maildir folder (located under `./bin/maildirs/`) to roughly 80 GiB.
>
> This can be disabled by changing the value of `populate_maildir` in the first lines of the
> script.

Adds 1000 different users, populates their Maildir and connects to all of them using netcat.
Once connected, all users run the following commands: `STAT`, `LIST`, `RETR 1`, in that order.

## `test_pipelining.sh`

Tests that a single call to the server executes all commands. The default command is

```bash
# Using -e -n with echo is important (!)
echo -en "USER USER1\r\nPASS 12345\r\nLIST\r\nSTAT\r\nQUIT\r\n" | nc -C 127.0.0.1 60711
```

## License

> MIT License - 2023
> Copyright 2023 - Lopez Guzman, Zahnd

See [LICENSE](/LICENSE) for details.
