#!/bin/bash

pid=$(pgrep -f "python.*matrix.py")
[ -n "$pid" ] && kill -s SIGHUP $pid
