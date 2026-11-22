# cc2 — Compucolor II emulator

A Compucolor II emulator written in C++20. It boots the system ROM, drives the 384×256 CRT (hardware scroll, blink, plot graphics and an optional lowercase character set), accepts keyboard input, reads and writes `.ccvf` floppy images, and can emulate the optional SoundWare 1-bit audio add-on. It runs on Linux and macOS with SFML 3.

More detail is available in the [user guide](docs/USER_GUIDE.md) and the [architecture overview](docs/ARCHITECTURE.md).

## Features

- Intel 8080 CPU, TMS5501 timer/serial/I/O chip and SMC5027 CRT controller as runtime plugins; the CRT timing PROM is embedded in the Compucolor plugin and exposed through I/O ports `0x80–0x9F`.
- Two floppy drives (A and B) using `.ccvf` images. Disks can be mounted, ejected, saved and saved-as while running; unsaved changes are detected. The last mounted disks are restored at startup (`disks/chomp.ccvf` and `disks/life.ccvf` by default).
- Save/load complete emulator state to `compucolor-state.json` (File → Save State / Load State).
- SFML window with resizable, aspect-correct display, optional smooth scaling and fullscreen. Window scale is 1x–8x (hotkeys and the View menu offer 1x–4x); the default for a fresh configuration is 4x.
- Full keyboard matrix: cursor keys, Home, Tab, Insert/Delete, function keys, Ctrl and PC symbols produce the matching Compucolor key or character chord (US layout). `Alt` is mapped to REPEAT when it is not part of a hotkey; right Windows/Super is also REPEAT and left Windows/Super is a one-shot AUTO.
- SoundWare audio from the TMS5501 TXD line, enabled by default at volume 100. Toggle with `Alt`+`A`, the Sound menu or Preferences. Only SoundWare-aware software produces audio.
- ImGui menu bar that scales with the window, a floating disk panel, status messages, a built-in `.ccvf` file browser, keyboard help and About dialogs. The menu overlays the top of the window, is excluded from screenshots and can be hidden with `Alt`+`M`.
- PNG screenshots of the raw 384×256 emulated frame (no menu overlay or letterboxing), saved to `<config-dir>/screenshots/`.
- Modular design: CPU, memory, scheduler, CRT, keyboard, floppy, SMC5027, TMS5501 and the whole machine are loaded from shared libraries listed in `app.config`.

## Quick start

Dependencies:

- Meson and a C++20 compiler
- SFML 3 (graphics and audio)
- Boost (`filesystem`, `json`, `dll`)
- GoogleTest and GoogleMock (tests)
- Python 3 (build-time include check)
- Dear ImGui and imgui-sfml (Meson subprojects in `subprojects/`; no separate install needed)

The install prefix is hard-coded to `dist/` inside the project by `meson.build`, so a Meson `--prefix` flag has no effect.

```sh
meson setup build
cd build
meson compile
meson test
meson install --skip-subprojects
cd ../dist
./cc2_emu.sh
```

`cc2_emu.sh` changes to the `dist/` directory before starting the emulator. This is required: `app.config`, `plugins/`, `disks/` and `compucolor-state.json` are all resolved relative to the current working directory. Command-line arguments are forwarded to the binary but are currently ignored.

## Controls

Hotkeys (checked before the menu, so they also work while a menu or dialog has focus):

| Keys | Action |
| --- | --- |
| `Alt`+`Enter` | Toggle fullscreen (recreates the window) |
| `Alt`+`1` … `Alt`+`4` | Set window scale to 1x–4x and leave fullscreen |
| `Alt`+`S` | Toggle smooth scaling |
| `Alt`+`A` | Toggle SoundWare audio |
| `Alt`+`M` | Show/hide the menu |
| `Alt`+`P` | Save a screenshot |

Help → Keyboard lists the emulated key mappings. In short: letters and punctuation are typed as character chords using a US layout, the number row and numpad map to digits, `F1`–`F15`, arrows, `Home`, `Tab`, `Insert`, `Delete`, `PgUp`/`PgDn`, `Esc` and `Ctrl` map to their Compucolor equivalents, `End` maps to `Home`, and `Pause` maps to `BREAK`. Physical Shift is folded into the character chord rather than sent separately. Held keys are released when the window loses focus or is recreated.

### Menus

| Menu | Items |
| --- | --- |
| File | Save/Load State; Mount/Eject/Save/Save As for disks A and B; Save Screenshot; Quit |
| Machine | Reset (warm), Hard Reset, Pause, Real Speed |
| Sound | Enable Sound, Volume slider, SoundWare tooltip |
| View | Scale 1x–4x, Fullscreen, Smooth Scaling, Disk Panel, Hide Menu |
| Settings | Preferences… (display, sound, character set, ROM version and speed) |
| Help | Keyboard reference, About |

Mount, Eject and Quit prompt before discarding unsaved disk changes. Save writes immediately; Save As writes to a new path and makes it the mounted path. Changing the system ROM in Preferences performs a hard reset. Unchecking Real Speed runs the CPU in bounded fast-forward bursts rather than at real-time speed.

## Files and settings

| Item | Location |
| --- | --- |
| User settings | `$XDG_CONFIG_HOME/cc2emu/settings.config`, else `$HOME/.config/cc2emu/settings.config`, else `./settings.config` |
| Screenshots | `<directory containing settings.config>/screenshots/cc2-YYYYMMDD-HHMMSS.png` (by default `~/.config/cc2emu/screenshots/`) |
| Emulator state | `./compucolor-state.json` (current working directory, normally `dist/`) |
| Plugin paths | `./app.config` (relative paths such as `plugins/libCompucolor.Impl.Floppy`) |
| Default disks | `./disks/chomp.ccvf` and `./disks/life.ccvf` |
| ROM images | Source copies in `data/roms/`; both system ROMs and fonts are compiled into `Roms.cpp` / `FontRom.cpp`, so the installed `roms/` directory is informational |
| Installed app | `dist/bin/Compucolor.Impl.App.Sfml`, `dist/plugins/`, `dist/disks/`, `dist/cc2_emu.sh` |

Settings keys and defaults: `window_scale=4`, `fullscreen=false`, `smooth_scaling=false`, `charset=standard`, `rom_version=v8.79`, `speed_regulated=true`, `disk_a_path=disks/chomp.ccvf`, `disk_b_path=disks/life.ccvf`, `last_disk_dir` empty, `sound_enabled=true`, `sound_volume=100`. `charset=lowercase` selects the alternate font; `rom_version=v6.78` selects the older ROM and its 3-phase floppy stepper. Settings are written on clean exit, not after every change.

The `sound_enabled` and `sound_volume` lines in `data/app.config` are legacy and unused; use the Sound menu, Preferences or `settings.config`.

### Bundled disk images

`data/disks/` (copied to `dist/disks/`) contains 17 `.ccvf` images: `action_collection.ccvf`, `action_soundware.ccvf`, `airraid.ccvf`, `blackjack.ccvf`, `blank.ccvf`, `chess.ccvf`, `chip_33.ccvf`, `chomp.ccvf`, `cooltools.ccvf`, `hangman.ccvf`, `life.ccvf`, `lunar_lander.ccvf`, `othello.ccvf`, `sampler.ccvf`, `sharks.ccvf`, `startrek.ccvf` and `swarms.ccvf`. `chomp.ccvf` and `life.ccvf` are writable. In `data/disks/`, only `action_soundware.ccvf` is read-only; `meson install` currently installs all images writable by their owner. Use Save As if you want to preserve the original bundled image. Some games in `action_collection.ccvf` require the v8.79 ROM. There is no joystick emulation.

`.ccvf` is a text format with optional `Label ...` lines followed by `Track N` sections of 64-hex-character lines (32 bytes per line). Each image holds 41 tracks × 10 sectors × 192 bytes.

## Project layout

```text
include/Compucolor/       public interfaces and plugin loader/manager
src/Compucolor.Impl.*/    emulator components and the SFML/ImGui application
test/                     GoogleTest suites and shared fakes/mocks
data/                     app.config, launcher script, ROMs and disk images
scripts/check_imports.py  build-time check that includes do not use ".."
docs/                     user guide and architecture overview
```

Build outputs go to `build/`; `meson install` writes the runnable application to `dist/`. See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for the component graph, threading model, memory map and I/O port routing.

## Testing

`meson test` runs nine test executables (Memory, Intel8080, Smc5027, Tms5501, Crt, Floppy, Keyboard, Compucolor and App). A separate build target runs `scripts/check_imports.py`, which rejects `#include` paths containing `..`.

## Dev container

```sh
devpod up . --recreate
ssh cc2emulator.devpod
```

The container forwards the host display and PulseAudio socket, and runs `meson setup build --prefix=$PWD/dist` on creation.
