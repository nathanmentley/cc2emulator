# cc2

A work in progress compucolor II emulator. Currently compiles and runs on macOS, and ubuntu 20.10.

## Deps

meson
gtk-3
gtest

## building

meson setup build
cd build
meson compile
meson test
./Compucolor.App.Impl.Gtk