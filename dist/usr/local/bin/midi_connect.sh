#!/bin/bash

card_number=$(cat /sys/$1/device/number)
name=$(head -n1 /proc/asound/card${card_number}/midi0)
aconnect=/usr/bin/aconnect 

keys="JUNO-DS"
clock="Deluge"
bcr="BCR2000"

msed="s/.*: '\(.*\)' .*/\1/g"
others=$($aconnect -o | /bin/grep '^client' | /bin/grep 'card=' | /bin/sed "$msed" | grep -v "$name")
SAVEIFS=$IFS
IFS=$'\n'
others=($others)
IFS=$SAVEIFS

if [ "$name" == "$keys" ]; then
	for (( i=0; i<${#others[@]}; i++ )); do
		other="${others[$i]}"
		$aconnect "$keys:0" "$other:0"
	done
else
	$aconnect "$keys:0" "$name:0"
fi

if [ "$name" == "$clock" ]; then
	for (( i=0; i<${#others[@]}; i++ )); do
		other="${others[$i]}"
		$aconnect "$clock:0" "$other:0"
		$aconnect "$other:0" "$clock:0"
	done
else
	$aconnect "$clock:0" "$name:0"
	$aconnect "$name:0" "$clock:0"
fi

if [ "$name" == "$bcr" ]; then
	for (( i=0; i<${#others[@]}; i++ )); do
		other="${others[$i]}"
		$aconnect "$bcr:0" "$other:0"
		$aconnect "$other:0" "$bcr:0"
	done
else
	$aconnect "$bcr:0" "$name:0"
	$aconnect "$name:0" "$bcr:0"
fi

pid=$(pgrep -f "python.*matrix.py")
[ -n "$pid" ] && kill -s SIGHUP $pid
