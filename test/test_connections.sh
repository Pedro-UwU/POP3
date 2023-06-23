#!/usr/bin/env bash
#
# MIT License - 2023
# Copyright 2023 - Lopez Guzman, Zahnd
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the “Software”), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# shellcheck disable=2155
readonly SCRIPT_DIR="$(dirname "${BASH_SOURCE[0]}")"
pushd "$SCRIPT_DIR" &> /dev/null || exit 1

populate_maildir=1   # 0 = YES; 1 = NO. Creates a lot of data.

ip="127.0.0.1"       # Replace with the desired IP address
port=60711           # Replace with the desired port number
timeout_duration=20  # Replace with the desired timeout duration in seconds
pop3_command=""      # See below. Will be replaced with desired command(s)

connections=1000

readonly CLIENT_BIN="../bin/client"

yes_no_question() {
    local question="$1"

    while true; do
        echo -n "$question [y/n] "
        read -r ans

        case "${ans,,}" in
            [y]*) return 0 ;;
            [n]*) return 1 ;;
            *) echo "Please enter 'y' or 'n'"
        esac
    done
}

# Function to connect to the IP address and send the command
connect_to_ip() {
    if echo -en "$pop3_command" | nc -w $timeout_duration $ip $port ; then
        echo "Command sent to $ip"
    else
        echo "Error connecting to $ip"
    fi
}

if [ ! -f "$CLIENT_BIN" ]; then
    echo "Could not find client executable. Did you call make?"
    exit 1
fi

if [ "$populate_maildir" -eq 0 ] && ! yes_no_question \
    "This script will create ~80 GiB of data. Are you sure you want to continue?"
then
    exit 0
fi

# Loop to initiate multiple connections
cmd=""
for ((i = 0; i < connections; i++)); do
    cmd+="ADD_USER TEST_USER$i TEST_PASS$i "
    if [ "$populate_maildir" -eq 0 ]; then
        cmd+="POPULATE_USER TEST_USER$i "
    fi
done

# shellcheck disable=2086
if ! $CLIENT_BIN $cmd; then # DO NOT quote
    exit 1
fi

echo "Drums..."

sleep 3

echo "CONNECT!"

for ((i = 0; i < connections; i++)); do
    pop3_command="USER TEST_USER$i\r\n"
    pop3_command+="PASS TEST_PASS$i\r\n"
    #
    # Append or replace pop3_command from here.
    #
    # Remember to add \r\n
    #
    pop3_command+="STAT\r\n"
    pop3_command+="LIST\r\n"
    pop3_command+="RETR 1\r\n"

    connect_to_ip &
done

wait

popd &> /dev/null || exit 1 # $SCRIPT_DIR
exit 0
