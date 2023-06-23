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

dir_results="results/"

ip="127.0.0.1"       # Replace with the desired IP address
port=60401           # Replace with the desired port number
timeout_duration=5   # Replace with the desired timeout duration in seconds

# Function to generate ADD_USER commands
generate_add_user_commands() {
    for ((i=1; i<=500; i++)); do
        echo "ADD_USER user$i 1234 "
    done
}

# Prepare the command sequence
commands="LOGIN admin admin\r\n$(generate_add_user_commands)\r\n"

# Execute the commands using nc
if ! echo -en "$commands" | nc -w "$timeout_duration" -C "$ip" "$port" ; then
    echo "Error while trying to connect (nc)"
    exit 1
fi

mkdir -p "$dir_results"

for ((i=1; i<=500; i++)); do
    curl --url 'pop3://127.0.0.1:60711/' -u "user$i:1234" > /dev/null 2> "$dir_results"/"user$i.txt" &
done

echo "See curl output in: $SCRIPT_DIR/$dir_results"
