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

ip="127.0.0.1"
port=60711
user="INTEGRITY1"
pass="12345"
monitor_command="ADD_USER $user $pass "

readonly USER_MAILDIR="../bin/maildirs/INTEGRITY1/Maildir/"

readonly CLIENT_BIN="../bin/client"

if [ ! -f "$CLIENT_BIN" ]; then
    echo "Could not find client executable. Did you call make?"
    exit 1
fi

EMAIL_FILENAME=""

new_email() {
    local filename=""

    # All printable (ASCII) chars
    local chars=' !"#$%&()*+,\-./0-9:;<=>?@A-Z[\\]^_`a-z{|}~'
    # Fixed size
    # 2 GiB
    local bytes="2147483648" # + 2 due to \r\n

    filename="$(mktemp --tmpdir="$USER_MAILDIR/new" XXXXXXXXXX)"

    LC_ALL=C </dev/urandom \
        tr -dc "$chars" | \
        head -c "$bytes" > "$filename"

    echo "" >> "$filename"

    perl -pi -e 's/\n/\r\n/' "$filename"

    EMAIL_FILENAME="$filename"
}

hash_email() {
    sha256sum "$1" | cut -d " " -f 1
}

# Create user in server

echo -e "\e[34mCreating user in POP3 server...\e[0m"
# shellcheck disable=2086
if ! $CLIENT_BIN $monitor_command; then # DO NOT quote
    echo -e "\e[31mError\e[0m"
    exit 1
fi

# Manually clean and populate its Maildir, so we have control over it
mkdir -p "$USER_MAILDIR/"{cur,new,env}

echo -en "\e[34mCreating 2 GiB email... \e[0m"
new_email
echo -e "\e[32mOK\e[0m"

EXCPECTED_HASH="$(hash_email "$EMAIL_FILENAME")"

OUTPUT_FILENAME="integrity_output.txt"

echo -e "\e[34mFetching file with curl... \e[0m"

if ! curl --url "pop3://$ip:$port/1" -u "$user:$pass" > "$OUTPUT_FILENAME"; then
    echo -e "\e[31mError\e[0m"
fi

echo -e "\e[32mOK\e[0m"

RECEIVED_HASH="$(hash_email "$OUTPUT_FILENAME")"

echo ""
echo -e "\e[33mExpected:\e[0m\t$EXCPECTED_HASH"
echo -e "\e[33mReceived:\e[0m\t$RECEIVED_HASH"

if [ "$EXCPECTED_HASH" = "$RECEIVED_HASH" ]; then
    echo -e "\e[32mWe received the exact same file!\e[0m"
fi

popd &> /dev/null || exit 1 # $SCRIPT_DIR
exit 0
