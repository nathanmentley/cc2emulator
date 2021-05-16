# cc2

## Deps
meson
gtk
googletest

## building

meson setup build
cd build
meson compile
meson test
./Compucolor.App.Impl.Gtk