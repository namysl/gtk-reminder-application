#!/bin/bash

gcc -o $1 $1.c `pkg-config --cflags --libs gtk+-3.0 libnotify`

echo ; echo
chmod +x $1

echo END of bash script
