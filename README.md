# gtk-reminder-application
A simple task reminder that allows you to add new events, modify or delete them. 

![reminder_demo](img/reminder_demo.jpg?raw=true "reminder_demo.jpg")

## Used libraries:
* libgtk-3 (v3.24.20)
* libnotify (v0.7.9)

You can install them by running the script *libs.sh*.

## Compilation: 
```
gcc -o reminder reminder_app.c `pkg-config --cflags --libs gtk+-3.0 libnotify`
```
or by using *compile.sh* script.
