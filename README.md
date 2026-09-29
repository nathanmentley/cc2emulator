# cc2

A work in progress compucolor II emulator. Currently compiles and runs on macOS, and ubuntu 20.10.

## Deps

meson

sfml

gtest

## building

meson setup build

cd build

meson compile

meson test

meson install

cd ../dist

./cc2_emu.sh
