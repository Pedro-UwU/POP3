#!/bin/bash
#
# shellcheck disable=2155
readonly SCRIPT_DIR="$(dirname "${BASH_SOURCE[0]}")"
pushd "$SCRIPT_DIR" &> /dev/null || exit 1

ip="127.0.0.1"       # Replace with the desired IP address
port=60711           # Replace with the desired port number
timeout_duration=20  # Replace with the desired timeout duration in seconds
pop3_command=""      # Replace below with desired command(s)

readonly CLIENT_BIN="../bin/client"

add_user() {
    local cmd=""
    local i=0
    for arg in "$@"; do
        if [ $i -eq 0 ]; then
            cmd+=" "
            cmd+="ADD_USER "
            i=$((i + 1))

            cmd+="$arg"
        else
            cmd+=" "
            cmd+="$arg"
            i=0

        fi
    done

    # shellcheck disable=2086
    $CLIENT_BIN $cmd # DO NOT quote
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

# Loop to initiate multiple connections
uname=""
for ((i=0; i<500; i++)); do
    uname+="TEST_USER$i TEST_PASS$i "
done

# shellcheck disable=2086
add_user $uname # DO NOT quote

echo "Drums..."

sleep 3

echo "Now... CONNECT!"

for ((i=0; i<500; i++)); do
    pop3_command="USER TEST_USER$i\r\n"
    pop3_command+="PASS TEST_PASS$i\r\n"
    # Append or replace pop3_command from here.
    #
    # Remember to add \r\n
    pop3_command+="STAT\r\n"
    pop3_command+="LIST\r\n"

    connect_to_ip &
done

wait

popd &> /dev/null || exit 1 # $SCRIPT_DIR
exit 0
