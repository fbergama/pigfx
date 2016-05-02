#!/bin/bash
tmux new-window -n "GDB" "arm-none-eabi-gdb --tui --command gdb_commands.txt"

