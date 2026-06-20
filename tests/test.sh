#!/bin/bash

SERVER="127.0.0.1"
PORT="9090"
PASS=0
FAIL=0

normalize(){
    tr -d ' \t\r\n' |
    sed 's/^[[:space:]]*//;s/[[:space:]]*$//' |
    grep -v '^$'
}

send() { printf "%s\n" "$1" >&3; }
recv() { 
    local line
    while read -t0.5 -r line <&3; do
        echo "$line"
    done
}
test() {
    [ -f /tmp/got.txt ] && rm /tmp/got.txt
    [ -f /tmp/expected.txt ] && rm /tmp/expected.txt
    local isLoggedIn=$1
    local name="$2"
    local input="$3"
    local expected="$4"    
    
    exec 3<>/dev/tcp/$SERVER/$PORT

    if [ $isLoggedIn == true ]; then
        send "/login alex"
        recv
    fi

    send "$input"
    recv >> /tmp/got.txt
    echo $expected > /tmp/expected.txt

    exec 3>&- 3<&-
    
    normalize < /tmp/got.txt > /tmp/got_norm.txt
    normalize < /tmp/expected.txt > /tmp/expected_norm.txt

    if diff -q /tmp/expected_norm.txt /tmp/got_norm.txt > /dev/null; then
        echo "PASS: $name"
        PASS=$((PASS + 1))
    else 
    echo "FAIL: $name"
    echo " expected: 
        $(cat /tmp/expected_norm.txt)
        "
    echo " got: 
        $(cat /tmp/got_norm.txt)
        "
    FAIL=$((FAIL + 1))
    fi
}

    test false "Login" "/login aleks" "Type /login to login with your username User was logged in with username: aleks"
    test true "Get Details" "/details" "alex"
   # test true "Create Chatroom" "/create room" 
