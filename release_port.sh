# !/usr/bin/bash
set -e
set -x

ps aux | grep psw2v | grep -v 'grep'  | cut -d ' ' -f 14 | xargs kill
