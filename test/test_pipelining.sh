#!/bin/bash

ip="127.0.0.1"
port=60711
# command="USER PEDRO\r\nUSER USER1\r\nPASS 123A\r\nUSER USER1\r\nPASS 12345\r\nQUIT\r\n"  # Replace with the command you want to send
command="USER USER1\r\nPASS 12345\r\RETR\r\n"

# Send the command through netcat
echo -e "$command" | nc -C $ip $port

if [ $? -eq 0 ]; then
    echo "Command sent to $ip:$port"
else
    echo "Error connecting to $ip:$port"
fi

