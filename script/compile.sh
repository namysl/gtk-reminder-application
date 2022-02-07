#!/bin/bash

gcc -o reminder reminder_app.c `pkg-config --cflags --libs gtk+-3.0 libnotify`

chmod +x reminder
