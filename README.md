# cc2

A Compucolor II emulator written in C++. It boots the system ROM, drives the CRT (hardware scroll, blink, double-height and plot characters), accepts keyboard input, and reads floppy disk images (`.ccvf`). It compiles and runs on macOS and Linux.

## Features

- Intel 8080 CPU, TMS5501 IO timer, SMC5027 CRT controller
- Floppy drives A and B with `.ccvf` images (see `data/disks`). Disks can be mounted, ejected and saved while running, and the last mounted disks are restored at startup (`disks/chomp.ccvf` and `disks/life.ccvf` the first time).
- SFML window and keyboard input 
- Full keyboard matrix: cursor keys, Home, Tab, Insert/Delete, function keys and Ctrl are mapped, and PC symbols produce the matching Compucolor characters (US layout). `Alt` acts as the REPEAT key.
- Resizable, aspect-correct window with optional smoothing and fullscreen. Settings persist in `~/.config/cc2emu/settings.config`.
- Menu bar that scales with the window: mount, eject and save disks (with a prompt for unsaved changes), reset, hard reset, pause, real-speed toggle, ROM version (v6.78 or v8.79), character set, display and sound preferences. It overlays the top of the screen and can be hidden (Alt-M).
- SoundWare add-on audio (the 1-bit sound used by disks such as "Life with Sound"). It is on by default; use the Sound menu, Preferences or `Alt`+`A` to toggle it and set the volume. The `sound_enabled` and `sound_volume=0..100` keys in `~/.config/cc2emu/settings.config` are updated automatically.
- Screenshots saved as PNG to `~/.config/cc2emu/screenshots`.

## Hotkeys

| Keys | Action |
| --- | --- |
| `Alt`+`Enter` | Toggle fullscreen |
| `Alt`+`1` .. `Alt`+`4` | Window scale 1x to 4x |
| `Alt`+`S` | Toggle smooth scaling |
| `Alt`+`A` | Toggle SoundWare audio |
| `Alt`+`M` | Show or hide the menu |
| `Alt`+`P` | Save a screenshot |

## Deps

- meson
- sfml (3.x)
- gtest

## Building

```sh
meson setup build --prefix="$PWD/dist"
cd build
meson compile
meson test
meson install --skip-subprojects
cd ../dist
./cc2_emu.sh
```

ROMs, disk images and configuration live in `data/`.

## Dev container

```sh
devpod up . --recreate
ssh cc2emulator.devpod
```
