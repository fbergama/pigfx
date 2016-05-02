#!/bin/bash
tmux new-window -n "OPENOCD" "openocd -f interface/jlink.cfg -f raspi.cfg"
sleep 1
tmux new-window -n "OPENOCD-TELNET" "telnet localhost 4444"
