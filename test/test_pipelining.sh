#!/bin/bash
#
# shellcheck disable=2155
readonly SCRIPT_DIR="$(dirname "${BASH_SOURCE[0]}")"
pushd "$SCRIPT_DIR" &> /dev/null || exit 1

ip="127.0.0.1"
port=60711
pop3_command="USER USER1\r\nPASS 12345\r\nLIST\r\nSTAT\r\nQUIT\r\n"

exit_val=0

# Send the command through netcat
if echo -en "$pop3_command" | nc -C $ip $port ; then
    echo "Command sent to $ip:$port"
else
    echo "Error connecting to $ip:$port"
    exit_val=1
fi

popd &> /dev/null || exit 1 # $SCRIPT_DIR
exit $exit_val
