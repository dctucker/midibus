#!/bin/bash

pid=$(pgrep -x "midi-server")
[ -n "$pid" ] && kill -s SIGHUP $pid
