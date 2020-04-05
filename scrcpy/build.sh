#!/bin/zsh
# Script that Robbert wrote to bundle three commands
rm -rf x
meson x --buildtype release --strip -Db_lto=true -Dprebuilt_server=/home/robbert/Desktop/soccer-gamer/prebuilt/scrcpy-server-v1.12.1
ninja -Cx

