#!/usr/bin/env bash

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
