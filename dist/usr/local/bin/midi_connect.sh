#!/bin/bash

card_number=$(cat /sys/$1/device/number)
name=$(head -n1 /proc/asound/card${card_number}/midi0)
aconnect=/usr/bin/aconnect 

pid=$(pgrep -x "midi-server")
[ -n "$pid" ] && kill -s SIGHUP $pid
