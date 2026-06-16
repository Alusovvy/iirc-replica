#!/bin/bash

MESSAGE="My Test Message"

connect() {
    local i=$1
    echo "Test $i"
    
    if echo "$MESSAGE" | nc -w 3 127.0.0.1 9090 2>/dev/null; then
        echo "Terminal $i: Połączono pomyślnie"
    else
        echo "Terminal $i: Błąd połączenia"
   fi
}

for i in $(seq 1 5); do
    connect $i &
done

wait
echo "--- Wszystkie połączenia zakończone ---"
