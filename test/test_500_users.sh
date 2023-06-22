#!/bin/bash

# Function to generate ADD_USER commands
generate_add_user_commands() {
  for ((i=1; i<=500; i++)); do
    echo "ADD_USER user$i 1234"
  done
}

# Prepare the command sequence
commands="LOGIN admin admin\r\n$(generate_add_user_commands)\r\n"

# Execute the commands using nc
echo -e "$commands" | nc -w 5 -C 127.0.0.1 60401

for ((i=1; i<=500; i++)); do
    curl --url 'pop3://127.0.0.1:60711/' -u "user$i:1234" > /dev/null 2> results/user$i.txt &
done

