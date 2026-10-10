# cc2

A Compucolor II emulator written in C++. It boots the system ROM, drives the CRT (hardware scroll, blink, double-height and plot characters), accepts keyboard input, and reads floppy disk images (`.ccvf`). It compiles and runs on macOS and Linux.

## Features

- Intel 8080 CPU, TMS5501 I/O/timer, SMC5027 CRT controller
- Floppy drives A and B with `.ccvf` images (see `data/disks`). Disks can be mounted, ejected and saved while running, and the last mounted disks are restored at startup (`disks/life.ccvf` and `disks/chess.ccvf` the first time).
- SFML window and keyboard input (PC `Shift`+`;` produces the Compucolor `:`)
- Full keyboard matrix: cursor keys, Home, Tab, Insert/Delete, function keys and Ctrl are mapped, and PC symbols produce the matching Compucolor characters (US layout). `Alt` acts as the REPEAT key.
- Resizable, aspect-correct window with optional smoothing and fullscreen. Settings persist in `~/.config/cc2emu/settings.config`.
- Menu bar (Dear ImGui) that scales with the window: mount, eject and save disks (with a prompt for unsaved changes), reset, hard reset, pause, real-speed toggle, ROM version (v6.78 or v8.79), character set and display preferences. It overlays the top of the screen and can be hidden.
- Screenshots saved as PNG to `~/.config/cc2emu/screenshots`.

## Hotkeys

| Keys | Action |
| --- | --- |
| `Alt`+`Enter` | Toggle fullscreen |
| `Alt`+`1` .. `Alt`+`4` | Window scale 1x to 4x |
| `Alt`+`S` | Toggle smooth scaling |
| `Alt`+`M` | Show or hide the menu |
| `Alt`+`P` | Save a screenshot |

## Deps

- meson
- sfml (3.x)
- gtest
- Dear ImGui and imgui-sfml (fetched automatically as meson subprojects)

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
