#!/bin/bash

ip="127.0.0.1"  # Replace with the desired IP address
port=60711 # Replace with the desired port number
timeout_duration=20  # Replace with the desired timeout duration in seconds
command="USER ASDJH"  # Replace with the command you want to send

# Function to connect to the IP address and send the command
connect_to_ip() {
    echo "$command" | nc -w $timeout_duration $ip $port
    if [ $? -eq 0 ]; then
        echo "Command sent to $ip"
    else
        echo "Error connecting to $ip"
    fi
}

# Loop to initiate multiple connections
for ((i=0; i<500; i++)); do
    connect_to_ip &
done

wait

