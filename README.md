# cc2 — Compucolor II emulator

A Compucolor II emulator written in C++20. It boots the system ROM, drives the 384×256 CRT (hardware scroll, blink, plot graphics and an optional lowercase character set), accepts keyboard input, reads and writes `.ccvf` floppy images, and can emulate the optional SoundWare 1-bit audio add-on. It runs on Linux and macOS with SFML 3.

More detail is available in the [user guide](docs/USER_GUIDE.md) and the [architecture overview](docs/ARCHITECTURE.md).

## Features

- Intel 8080 CPU, TMS5501 timer/serial/I/O chip and SMC5027 CRT controller as runtime plugins; the CRT timing PROM is embedded in the Compucolor plugin and exposed through I/O ports `0x80–0x9F`.
- Two floppy drives (A and B) using `.ccvf` images. Disks can be mounted, ejected, saved and saved-as while running; unsaved changes are detected. The last mounted disks are restored at startup (`disks/chomp.ccvf` and `disks/life.ccvf` by default).
- Save/load complete emulator state to a JSON file chosen in the File → Save State… / Load State… browser, or to an explicit path via CLI flags.
- Record and replay live GUI sessions as versioned `.cc2replay.json` bundles with emulated-time `CompucolorIIKey` events. `Compucolor.Impl.App.Headless` remains the deterministic oracle for state hashes and JSONL traces.
- SFML window with resizable, aspect-correct display, optional smooth scaling and fullscreen. Window scale is 1x–8x (hotkeys and the View menu offer 1x–4x); the default for a fresh configuration is 4x.
- Full keyboard matrix: cursor keys, Home, Tab, Insert/Delete, function keys, Ctrl and PC symbols produce the matching Compucolor key or character chord (US layout). A mouse-driven on-screen keyboard exposes every emulated key for anything the host mapping cannot reach. `Alt` is reserved for host shortcuts and is never sent to the emulated keyboard; right Windows/Super is REPEAT and left Windows/Super is a one-shot AUTO.
- SoundWare audio from the TMS5501 TXD line, enabled by default at volume 100. Toggle with `Alt`+`A`, the Sound menu or Preferences. Only SoundWare-aware software produces audio.
- ImGui menu bar that scales with the window, ten display themes, optional CRT scanlines, fuzz, vignette and phosphor glow with per-layer intensity sliders, a floating disk panel, status messages, a built-in `.ccvf` file browser, keyboard help and About dialogs. The menu overlays the top of the window, is excluded from screenshots and can be hidden with `Alt`+`M`.
- Debug windows for CPU state, memory, I/O ports, a rolling instruction trace and TMS5501/SMC5027 status panels. Trace recording is off by default; filters and stop-on-match triggers can tame noisy streams, and traces can be recorded to newline-delimited JSON. Memory edits are enabled only while the machine is paused.
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

`cc2_emu.sh` changes to the `dist/` directory before starting the emulator. This is required: `app.config`, `plugins/`, `disks/` and relative state paths are resolved against the current working directory. The launcher forwards arguments to the binary. Both `--flag <path>` and `--flag=<path>` forms are accepted:

| Flag | Effect |
| --- | --- |
| `--trace-jsonl <path>` | Start JSONL debug recording immediately. |
| `--load-state <path>` | Load a saved state immediately after startup. |
| `--replay-bundle <path>` | Load a bundle's state and replay its input in the GUI. |
| `--record-replay <path>` | Capture startup state and record keyboard input until exit. |
| `--trace-filter <spec>` | Restrict which JSONL records are written. Repeatable; also accepts `--trace-filter=<spec>`. |

`--load-state` and `--replay-bundle` are mutually exclusive, as are `--record-replay` and `--replay-bundle`.

The headless replay CLI does not need a window or `app.config`:

```sh
./dist/cc2_replay.sh \
  --bundle replay.cc2replay.json \
  --trace-jsonl trace.jsonl \
  --trace-filter 'instr:text=MOV' \
  --trace-filter 'port:port=0x07,dir=write' \
  --expected-state-hash <sha256>
```

It loads the saved state, applies `input` events at their emulated timestamps and prints the final state hash and emulated time. `--trace-jsonl` and `--expected-state-hash` are optional. `--trace-filter` is repeatable and applies only to JSONL output; with no filters, the full stream is written. Filter specs are `kind[:key=value[,key=value...]]`, with kinds `cpu`, `instr`, `port`, `tms5501` and `smc5027`. See [`docs/REPLAY_PLAN.md`](docs/REPLAY_PLAN.md) for the full syntax, per-kind keys and phasing.

### GUI replay bundles

Use **File → Start Replay Recording…** to choose a bundle path. Recording captures the current machine state and every emulated key down/up transition, stamped with emulated time. A `REC` chip appears in the menu bar. **File → Stop Replay Recording** writes the bundle immediately; quitting while still recording writes it on exit.

To watch a bundle in the GUI:

```sh
./dist/cc2_emu.sh --replay-bundle replay.cc2replay.json
```

The bundle's state is loaded after startup, and its input events are applied on 1 ms machine-tick boundaries through the existing keyboard path. While replay is active, live keyboard input is ignored so the playback cannot be contaminated; normal input resumes when replay reaches its stop time. **Debug → Replay Status** shows current emulated time, the next input event time and the stop target. GUI replay is for visual confirmation; use `Compucolor.Impl.App.Headless` for deterministic final-state and trace comparisons.

## Controls

`Alt` is host-only: every Alt keydown, keyup and Alt chord is consumed by the UI and never reaches the emulated Compucolor keyboard. Shortcuts are suppressed while ImGui owns keyboard focus (for example, while typing in Preferences), but the Alt event is still swallowed.

| Keys | Action |
| --- | --- |
| `Alt`+`Enter` | Toggle fullscreen (recreates the window) |
| `Alt`+`1` … `Alt`+`4` | Set window scale to 1x–4x and leave fullscreen |
| `Alt`+`S` | Toggle smooth scaling |
| `Alt`+`Shift`+`F` | Toggle CRT effects |
| `Alt`+`A` | Toggle SoundWare audio |
| `Alt`+`M` | Show/hide the menu |
| `Alt`+`P` | Save a screenshot |
| `Alt`+`Shift`+`S` | Save state… |
| `Alt`+`Shift`+`L` | Load state… |
| `Alt`+`R` | Start/stop replay recording |
| `Alt`+`Shift`+`P` | Pause/resume |
| `Alt`+`Shift`+`R` | Toggle real speed |
| `Alt`+`D` | Show/hide the disk panel |
| `Alt`+`T` | Start/stop trace recording |
| `Alt`+`Shift`+`T` | Clear traces |
| `Alt`+`C` | Toggle the CPU debug window |
| `Alt`+`O` | Preferences… |
| `Alt`+`H` | Keyboard help |
| `Alt`+`I` | About |
| `Alt`+`Shift`+`Q` | Quit |

Help → Keyboard lists the emulated key mappings. In short: letters and punctuation are typed as character chords using a US layout, the number row and numpad map to digits, `F1`–`F15`, arrows, `Home`, `Tab`, `Insert`, `Delete`, `PgUp`/`PgDn`, `Esc` and `Ctrl` map to their Compucolor equivalents, `End` maps to `Home`, and `Pause` maps to `BREAK`. Physical Shift is folded into the character chord rather than sent separately. Held keys are released when the window loses focus or is recreated.

CRT effects are optional and off by default. Scanlines stay locked to the source rows, fuzz rerolls every rendered frame, and vignette/phosphor glow are also available; each layer can be toggled in Preferences. They affect only the emulated screen, never ImGui or saved screenshots.

### Menus

| Menu | Items |
| --- | --- |
| File | Save/Load State; Mount/Eject/Save/Save As for disks A and B; Save Screenshot; Quit |
| Machine | Reset (warm), Hard Reset, Pause, Real Speed |
| Debug | Record trace; CPU, Memory, Ports, Trace, TMS5501 and SMC5027 windows; Clear traces |
| Sound | Enable Sound, Volume slider, SoundWare tooltip |
| View | Scale 1x–4x, Fullscreen, Smooth Scaling, CRT Effects, Disk Panel, Hide Menu |
| Settings | Preferences… (display, theme, CRT effects, sound, character set, ROM version and speed) |
| Help | Keyboard reference, About |

Mount, Eject and Quit prompt before discarding unsaved disk changes. Save writes immediately; Save As writes to a new path and makes it the mounted path. Changing the system ROM in Preferences performs a hard reset. Unchecking Real Speed runs the CPU in bounded fast-forward bursts rather than at real-time speed.

Debug → Record trace turns on bounded CPU-instruction and I/O-port capture (the last 512 entries of each). The CPU window shows registers, flags, the next decoded instruction and Pause/Resume/Reset buttons; Memory shows a hex view with editable bytes when paused; Ports and Trace show the most recent events first. Both event windows have a Filters section: the Trace window can match a PC range and a disassembled mnemonic substring, and the Ports window can match a port range and read/write direction. With **Stop on match** enabled, the matching event is recorded and that stream freezes until **Rearm** or **Clear traces**. TMS5501 shows status, interrupts, timers and decoded I/O lines; SMC5027 shows cursor and scroll registers. No single-step, run-to-address or breakpoints are implemented yet.

`./cc2_emu.sh --trace-jsonl trace.jsonl` (or `--trace-jsonl=trace.jsonl`) writes one JSON object per line with `kind` values `cpu`, `instr`, `port`, `tms5501` and `smc5027`. Each line includes `v: 1` and a local increasing `seq`. The Debug → Record trace checkbox pauses/resumes both the in-memory buffers and the JSONL sink.

## Files and settings

| Item | Location |
| --- | --- |
| User settings | `$XDG_CONFIG_HOME/cc2emu/settings.config`, else `$HOME/.config/cc2emu/settings.config`, else `./settings.config` |
| Screenshots | `<directory containing settings.config>/screenshots/cc2-YYYYMMDD-HHMMSS.png` (by default `~/.config/cc2emu/screenshots/`) |
| Emulator state | Last path chosen in the state browser is remembered in `state_path` in `settings.config` |
| Plugin paths | `./app.config` (relative paths such as `plugins/libCompucolor.Impl.Floppy`) |
| Default disks | `./disks/chomp.ccvf` and `./disks/life.ccvf` |
| ROM images | Source copies in `data/roms/`; both system ROMs and fonts are compiled into `Roms.cpp` / `FontRom.cpp`, so the installed `roms/` directory is informational. The Compucolor ROM code is copyrighted; see the notice at the top of `src/Compucolor.Impl.Compucolor/Roms.cpp`. |
| Installed apps | `dist/bin/Compucolor.Impl.App.Sfml`, `dist/bin/Compucolor.Impl.App.Headless`, `dist/plugins/`, `dist/disks/`, `dist/cc2_emu.sh`, `dist/cc2_replay.sh` |

Settings keys and defaults: `window_scale=4`, `fullscreen=false`, `smooth_scaling=false`, `theme=0`, `crt_effects=false`, `crt_scanlines=true`, `crt_fuzz=true`, `crt_vignette=true`, `crt_glow=true`, `crt_scanline_intensity=35`, `crt_fuzz_intensity=35`, `crt_vignette_intensity=35`, `crt_glow_intensity=35`, `charset=standard`, `rom_version=v8.79`, `speed_regulated=true`, `disk_a_path=disks/chomp.ccvf`, `disk_b_path=disks/life.ccvf`, `last_disk_dir` empty, `state_path` empty, `sound_enabled=true`, `sound_volume=100`. `theme` is an index from 0–9: Standard, Synthwave, Gruvbox, WarmRetro, Amber Terminal, Green Phosphor, Commodore 64, CGA, Solarized Dark and Nord; it is applied at startup. `charset=lowercase` selects the alternate font; `rom_version=v6.78` selects the older ROM and its 3-phase floppy stepper. Settings are written on clean exit, not after every change. The older `crt_intensity` key is ignored in favour of the four per-layer intensity keys.

The `sound_enabled` and `sound_volume` lines in `data/app.config` are legacy and unused; use the Sound menu, Preferences or `settings.config`.

### Bundled disk images

`data/disks/` (copied to `dist/disks/`) contains 18 `.ccvf` images: `action_collection.ccvf`, `action_soundware.ccvf`, `airraid.ccvf`, `blackjack.ccvf`, `blank.ccvf`, `chess.ccvf`, `chip_33.ccvf`, `chomp.ccvf`, `cooltools.ccvf`, `demonstration.ccvf`, `hangman.ccvf`, `life.ccvf`, `lunar_lander.ccvf`, `othello.ccvf`, `sampler.ccvf`, `sharks.ccvf`, `startrek.ccvf` and `swarms.ccvf`. `chomp.ccvf`, `demonstration.ccvf` and `life.ccvf` are writable. In `data/disks/`, only `action_soundware.ccvf` is read-only; `meson install` currently installs all images writable by their owner. Use Save As if you want to preserve the original bundled image. Some games in `action_collection.ccvf` require the v8.79 ROM. There is no joystick emulation.

`.ccvf` is a text format with optional `Label ...` lines followed by `Track N` sections of 64-hex-character lines (32 bytes per line). Each image holds 41 tracks × 10 sectors × 192 bytes.

## Project layout

```text
include/Compucolor/       public interfaces, shared app/replay headers and plugin loader/manager
src/Compucolor.Impl.*/    emulator components plus split GUI/headless app implementations
test/                     GoogleTest suites and shared fakes/mocks
data/                     app.config, launcher scripts, ROMs and disk images
scripts/check_imports.py  build-time check that includes do not use ".."
docs/                     user guide, architecture overview and replay plan
```

Build outputs go to `build/`; `meson install` writes the runnable application to `dist/`. See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for the component graph, threading model, memory map and I/O port routing.

## Testing

`meson test` runs ten test executables (Memory, Intel8080, Smc5027, Tms5501, Crt, Floppy, Keyboard, Compucolor, App and Replay). A separate build target runs `scripts/check_imports.py`, which rejects `#include` paths containing `..`.

GitHub Actions runs the same build and test suite, plus `git diff --check`, on pushes and pull requests. The workflow builds the devcontainer image so CI uses the same SFML 3 toolchain as local development (`.github/workflows/ci.yml`).

The deterministic save-state and input-replay workflow is documented in [`docs/REPLAY_PLAN.md`](docs/REPLAY_PLAN.md).

## Credits and references

This project was built using the following reference points and resources:

- [jtbattle/ccemu](https://github.com/jtbattle/ccemu) — Compucolor emulator implementation used as a reference point.
- [Compucolor.org](https://www.compucolor.org) — documentation and community resources for the Compucolor II.
- [superzazu/8080](https://github.com/superzazu/8080) — Intel 8080 CPU emulation core used by this project, wrapped by the local Intel8080 plugin.
- [Dear ImGui](https://github.com/ocornut/imgui) and [ImGui-SFML](https://github.com/eliasdaler/imgui-sfml) — UI libraries used by the SFML frontend.

Thanks to their authors and maintainers for preserving and documenting the platform.

## Dev container

```sh
devpod up . --recreate
ssh cc2emulator.devpod
```

The container forwards the host display and PulseAudio socket, and runs `meson setup build --prefix=$PWD/dist` on creation.
