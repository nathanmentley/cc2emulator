# cc2

A Compucolor II emulator written in C++. It boots the system ROM, drives the CRT (hardware scroll, blink, double-height and plot characters), accepts keyboard input, and reads floppy disk images (`.ccvf`). It compiles and runs on macOS and Linux.

## Features

- Intel 8080 CPU, TMS5501 I/O/timer, SMC5027 CRT controller
- Floppy drive emulation with `.ccvf` images (see `data/disks`)
- SFML window and keyboard input (PC `Shift`+`;` produces the Compucolor `:`)

## Deps

- meson
- sfml
- gtest

## Building

```sh
meson setup build
cd build
meson compile
meson test
meson install
cd ../dist
./cc2_emu.sh
```

ROMs, disk images and configuration live in `data/`.

## Dev container

```sh
devpod up . --recreate
ssh cc2emulator.devpod
```
