#!/usr/bin/env/bash
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

readonly MAILDIRS="../bin/maildirs/"
readonly USERNAME="USER1"

random_number ()
{
    # Random number with 1 or 2 bytes
    local number=$(od \
            --address-radix=n \
            --format=d \
            --read-bytes=$((1 + RANDOM % 2)) \
            /dev/urandom | \
        tr -d ' ')

    echo "$number"
}

new_email()
{
    local dir="$MAILDIRS/$USERNAME/Maildir/new/"
    local filename=""

    # All printable (ASCII) chars
    local chars=' !"#$%&()*+,\-./0-9:;<=>?@A-Z[\\]^_`a-z{|}~'
    # Fixed size
    # local bytes="63998" # + 2 due to \r\n
    # Random size
    local bytes="$(random_number)"

    filename="$(mktemp --tmpdir="$dir" XXXXXXXXXX)"

    LC_ALL=C </dev/urandom \
        tr -dc "$chars" | \
        head -c "$bytes" > "$filename"

    echo "" >> "$filename"

    perl -pi -e 's/\n/\r\n/' "$filename"
}

mkdir -p "$MAILDIRS/$USERNAME/Maildir/"{cur,new,env}

for ((i = 0; i < 16; i++)) ; do
    new_email "$USERNAME"
done

popd &> /dev/null || exit 1 # $SCRIPT_DIR
