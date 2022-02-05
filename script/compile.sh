#!/bin/bash

gcc -o przypominacz przypominacz.c `pkg-config --cflags --libs gtk+-3.0 libnotify`

chmod +x przypominacz
