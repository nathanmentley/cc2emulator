# User guide

This guide covers running the emulator and using its interface. See the [README](../README.md) for build instructions and the [architecture overview](ARCHITECTURE.md) for implementation details.

## Starting and stopping

Build and install as described in the README, then run the launcher:

```sh
cd dist
./cc2_emu.sh
```

The launcher changes to `dist/` because `app.config`, plugin libraries, disk images and the saved emulator state are resolved relative to the current working directory. Running the binary directly from another directory produces a "Missing setting ... in app.config" error. Both `--flag <path>` and `--flag=<path>` forms are accepted. The separate headless replay tool is described under [Headless replay](#headless-replay).

| Flag | Effect |
| --- | --- |
| `--trace-jsonl <path>` | Start JSONL debug recording as soon as the emulator starts. |
| `--load-state <path>` | Load a saved state immediately after startup. |
| `--replay-bundle <path>` | Load a bundle's state and replay its input in the GUI. |
| `--record-replay <path>` | Capture startup state and record keyboard input until exit. |
| `--trace-filter <spec>` | Restrict JSONL records; repeatable and also accepted as `--trace-filter=<spec>`. |

`--load-state` cannot be combined with `--replay-bundle`; `--record-replay` cannot be combined with `--replay-bundle`.

The window title is "Compucolor II Emulator". On first launch the default disks are `chomp.ccvf` in drive A and `life.ccvf` in drive B; the fresh default window scale is 4x (384×256 scaled to 1536×1024). Existing settings files are unchanged by code updates.

## Keyboard

Most keys are translated into Compucolor key presses. The emulator uses a US-layout mapping based on the character a key produces:

- Digits and punctuation follow US-layout symbols; for example `Shift+;` produces `:`. The Compucolor uses typewriter case: an unshifted PC letter produces an uppercase Compucolor letter, and `Shift`+letter produces a lowercase one.
- Numpad digits, `F1`–`F15`, `Esc`, both `Ctrl` keys, cursor keys, `Home`, `Tab`, `Insert`, `Delete`, `PageUp`, `PageDown` and `End` map to emulated keys. `End` maps to `Home`; `Pause` maps to `BREAK`.
- `Alt` is reserved for host shortcuts and is never sent to the emulated keyboard. Right Windows/Super is REPEAT. Left Windows/Super maps to AUTO and is released automatically after 200 ms because some desktops swallow its key-up.
- Shift is folded into the produced character chord and is not sent as a separate emulated press.
- Multiple physical keys can hold the same emulated key; it is released only after all of them are released. All emulated keys are released when the window loses focus, closes or is recreated.

If the PC layout differs from US, the character-based mapping may not match the printed symbols. Every `Alt` keydown, keyup and Alt chord is consumed by the host UI, including unbound chords. While ImGui needs the keyboard (for example, when typing in Preferences), shortcuts are suppressed but the Alt event is still swallowed.

### Hotkeys

| Keys | Action |
| --- | --- |
| `Alt`+`Enter` | Toggle fullscreen (recreates the window) |
| `Alt`+`1` … `Alt`+`4` | Window scale 1x–4x; also exits fullscreen |
| `Alt`+`S` | Toggle smooth scaling |
| `Alt`+`Shift`+`F` | Toggle CRT effects |
| `Alt`+`A` | Toggle SoundWare audio |
| `Alt`+`M` | Show/hide the menu |
| `Alt`+`P` | Save a screenshot |
| `Alt`+`Shift`+`S` | Save state… (opens the file browser) |
| `Alt`+`Shift`+`L` | Load state… (opens the file browser) |
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

The menu bar scales with window height and overlays the top of the window; it is excluded from screenshots. Hide it with `Alt`+`M` if it gets in the way; a short status message reminds you how to bring it back. Help → Keyboard shows the same mapping reference in the application.

## Menus

### File

- **Save State…** / **Load State…** — open the built-in JSON file browser to choose where to write/read emulator state. The chosen path is remembered in `state_path` for the next browser session. State includes CPU, memory, both drives, keyboard, display, timers and machine configuration.
- **Start Replay Recording…** — opens a save browser for a `.cc2replay.json` path, then captures the current state and starts recording emulated key down/up events. A `REC` chip appears in the menu bar.
- **Stop Replay Recording** — writes the bundle to the chosen path immediately. Enabled only while recording. Quitting while still recording also writes the bundle.
- **Mount Disk A/B…** — opens the built-in `.ccvf` browser. If the current disk has unsaved changes, you can save, discard or cancel first.
- **Eject Disk A/B** — unmounts the drive (with the same unsaved-changes prompt) and clears the saved disk path.
- **Save Disk A/B** — writes the mounted image in place immediately. It does not prompt; a failed write (for example, a read-only file) shows an Error dialog.
- **Save Disk A/B As…** — writes to a new path, adds `.ccvf` if the name has no extension, and makes that path the mounted path.
- **Save Screenshot** — writes a PNG of the raw 384×256 emulated screen to the `screenshots/` directory next to `settings.config` (by default `~/.config/cc2emu/screenshots/`). The menu overlay, window letterboxing and CRT effects are not included.
- **Quit** — checks both drives for unsaved changes before closing. Settings are saved on this clean exit.

The file browser starts in the last used disk directory, otherwise `disks/`, otherwise the working directory. Disk actions list directories and `.ccvf` files only; replay recording lists directories and `.json` files. Use `Up` to navigate, double-click or Enter to accept a file, and Cancel to close.

### Machine

- **Reset** — warm reset: resets CPU, timers, SMC5027 and keyboard but keeps memory.
- **Hard Reset** — also reloads the selected system ROM and clears the BASIC power-up flag for a cold start. Changing the ROM version performs a hard reset.
- **Pause** — stops CPU stepping; pending resets are still applied.
- **Real Speed** — when enabled (default), the CPU is paced to roughly 2 MHz with a 50 ms catch-up cap. When disabled, it runs in fast-forward bursts.

### Debug

- **Record trace** — toggles the master recording gate for CPU instructions, I/O-port accesses and the optional JSONL sink. Recording is off by default unless `--trace-jsonl` was supplied; each in-memory buffer keeps the most recent 512 events.
- **CPU** — shows the current register/flag snapshot, emulated time, the decoded instruction at PC, and Pause/Resume/Reset and Hard Reset buttons.
- **Memory** — a compact hexadecimal viewer/editor showing 64 bytes at a time. Enter a hexadecimal address (or use the ±64 buttons) to move the view; byte cells are editable only while the machine is paused. `0x0000–0x3FFF` is ROM, `0x6000–0x6FFF` aliases video RAM at `0x7000–0x7FFF`.
- **Ports** — shows recorded reads and writes with PC, port number, value, bit pattern and decoded device (TMS5501, SMC5027, CRT timing PROM or unmapped). Its **Filters** section can limit events by port range and to reads or writes.
- **Trace** — shows recorded instruction events with sequence number, PC, raw opcode bytes, decoded mnemonic, cycle count and flags. Its **Filters** section can limit events by PC range and by a case-insensitive substring of the mnemonic (for example `MVI` or `CALL`).
- **TMS5501** — shows the chip status byte, interrupt mask/pending bits, the next RST vector, five timer counters, serial data registers, decoded select lines, audio TXD level and the discrete outport.
- **SMC5027** — shows cursor X/Y, the first visible display row and a 32-row scroll-origin ruler.
- **Replay Status** — shows the current emulated time, next input event time and stop target while recording or replaying. It opens automatically when either action is active.
- **Clear traces** — empties both event buffers; CPU and memory state are unaffected.

Both the Ports and Trace windows also have a Record checkbox and Clear button. In either Filters section, **Enabled** turns filtering on; when it is off, all events of that kind are recorded. **Stop on match** records the first matching event and then freezes that stream until **Rearm** (or **Clear traces**). The other stream keeps recording independently. There are no single-step, step-N, run-to-address or breakpoint controls yet; use Machine → Pause (or the CPU window) to inspect memory safely.

### Recording to JSONL

Start the launcher with `--trace-jsonl trace.jsonl` or `--trace-jsonl=trace.jsonl` to write a newline-delimited JSON record for every CPU snapshot, instruction, port access and peripheral snapshot. The file is truncated when opened. Every line contains `v: 1` and a local increasing `seq`; `kind` is one of:

- `cpu` — register/flag snapshot with `emulatedNanoseconds`
- `instr` — `pc`, opcode `bytes` and `cycles`
- `port` — `pc`, `port`, `value` and `isWrite`
- `tms5501` — status/interrupt/timer/serial fields and decoded line flags
- `smc5027` — cursor and first display row (hex strings)

Hex fields are uppercase `0x`-prefixed strings. The Debug → Record trace checkbox pauses and resumes JSONL recording as well as the in-memory buffers; when a CLI trace is requested, recording starts enabled. Filters configured in the Debug windows apply only to those in-memory windows. JSONL filtering is configured separately with `--trace-filter`; without it, the JSONL file contains the full stream. A `text=` filter decodes the instruction for matching but does not add `text` to the JSONL record.

#### CLI JSONL filters

`--trace-filter <spec>` (or `--trace-filter=<spec>`) uses `kind[:key=value[,key=value...]]`. Repeat the flag to OR multiple filters; a record is written when a filter for its kind matches. If no filter mentions a kind, that kind is omitted. Invalid specs fail at startup with an error naming the spec. `seq` stays dense over the records actually written.

| Kind | Keys |
| --- | --- |
| `cpu` | `pc` |
| `instr` | `pc`, `text` |
| `port` | `pc`, `port`, `dir` |
| `tms5501` | none (bare kind only) |
| `smc5027` | none (bare kind only) |

`pc` and `port` take an inclusive hex range (`0x0100-0x01ff`) or a single hex value (`0x0100`); `port` values are 0–255. `dir` is `read`, `write` or `any`. `text` is a case-insensitive substring of the disassembled instruction.

```sh
./dist/cc2_emu.sh --trace-jsonl trace.jsonl --trace-filter instr:text=MOV
./dist/cc2_replay.sh --bundle replay.cc2replay.json \
  --trace-jsonl trace.jsonl \
  --trace-filter 'instr:pc=0x0100-0x01ff' \
  --trace-filter 'port:port=0x07,dir=write'
```

### GUI recording and replay

Start **File → Start Replay Recording…**, choose a `.cc2replay.json` path, and interact with the machine normally. The recorder stores the state at the moment recording starts plus each `CompucolorIIKey` down/up event with its emulated timestamp. **File → Stop Replay Recording** writes the bundle; quitting while recording writes it automatically.

Start the GUI from a bundle with:

```sh
./dist/cc2_emu.sh --replay-bundle replay.cc2replay.json
```

The GUI loads the bundle state after the machine starts and applies input on 1 ms tick boundaries through the same keyboard path used by live input. While replay is active, live keyboard input is ignored so the playback cannot be contaminated; normal input resumes automatically when the replay reaches its stop time. Host controls, menus and Pause/Resume still work. A `REPLAY` chip appears in the menu bar, and **Debug → Replay Status** reports current/next/stop times in emulated microseconds. This mode is for visual confirmation; use `Compucolor.Impl.App.Headless` when exact state hashes or trace comparisons matter.

`--load-state <path>` starts the GUI from a saved state without replaying input. It is rejected together with `--replay-bundle`.

### Headless replay

`dist/cc2_replay.sh` launches `dist/bin/Compucolor.Impl.App.Headless` to replay a saved state without opening a window or reading `app.config`. The launcher changes to `dist/` and points `LD_LIBRARY_PATH` at `plugins/`, because the headless executable links the hardware libraries directly:

```sh
./dist/cc2_replay.sh --bundle replay.cc2replay.json
./dist/cc2_replay.sh --bundle replay.cc2replay.json --trace-jsonl trace.jsonl
./dist/cc2_replay.sh --bundle replay.cc2replay.json \
  --trace-jsonl trace.jsonl \
  --trace-filter instr:text=MOV
./dist/cc2_replay.sh --bundle replay.cc2replay.json \
  --expected-state-hash ea9677ef6c0a044abdf846275cf992384c68744124fa3d4ea4fe59d475019553
```

Both `--flag value` and `--flag=value` forms are accepted. A version 1 bundle is JSON with `version`, optional `createdBy`, the complete saved `state`, a time-sorted `input` array and `stop.emulatedNanoseconds`. Each input entry is `{"t": <emulated nanoseconds>, "key": "<CompucolorIIKey name>", "down": true|false}`. Names are exact `CompucolorIIKey` enum names such as `A`, `Shift`, `F1` and `CurUp`.

The runner applies input on 1 ms machine-tick boundaries. An event may therefore take effect slightly after its timestamp, and replay stops on the first tick that reaches or passes `stop.emulatedNanoseconds`. The final SHA-256 of the canonicalized state is printed as `stateHash`; `--expected-state-hash` fails the run if it differs. CLI trace filters are described under [CLI JSONL filters](#cli-jsonl-filters); the GUI recording/replay workflow is described above. Filtering does not change the final state hash.

### Sound

SoundWare is an optional hardware add-on that produces audio by toggling a serial line. The emulator emulates that line, so only SoundWare-aware disks (for example `life.ccvf` / "Life with Sound" and `action_soundware.ccvf`) make sound. Use **Enable Sound** (`Alt`+`A`) and the **Volume** slider (0–100%), or set them in Preferences. The settings are stored in `settings.config` as `sound_enabled` and `sound_volume`.

### View

Scale 1x–4x, Fullscreen, Smooth Scaling, CRT Effects, Disk Panel and Hide Menu. CRT Effects adds scanlines, animated grain, vignette and phosphor glow to the emulated screen only; ImGui and screenshots stay clean. Scanlines stay locked to the source rows, while fuzz rerolls every rendered frame. The Disk Panel is a floating window showing each drive's filename, a `*` for unsaved changes, and Mount/Eject/Save buttons. The menu bar also shows small status chips such as `PAUSED`, `UNREGULATED`, `SOUND OFF` and modified-disk markers when there is room.

### Settings → Preferences

- **Display**: fullscreen, window scale 1–8, smooth scaling, a CRT effects master switch with individual Scanlines / Fuzz / Vignette / Phosphor glow toggles and intensity sliders, and a Theme combo (10 palettes for the eight hardware colors). The wider scale range is only available here or by editing `settings.config`. The saved theme is applied at startup. CRT effects are separate from the palette themes and do not affect screenshots.
- **Sound**: enable sound and volume, disabled when sound is off.
- **Machine**: character set (Standard or Lowercase), system ROM (v6.78 or v8.79) and Real Speed. Lowercase changes the font; the ROM choice also changes the floppy stepper geometry and triggers a hard reset.

## Settings file

Settings live in `$XDG_CONFIG_HOME/cc2emu/settings.config`, otherwise `$HOME/.config/cc2emu/settings.config`, otherwise `./settings.config`. The file is plain `key=value` lines; unknown keys are preserved. It is read at startup (including `theme`, which selects the display palette) and written only on clean exit, so a crash can lose changes made during a session.

| Key | Default | Effect |
| --- | --- | --- |
| `window_scale` | `4` | 384×256 × scale, clamped to 1–8 |
| `fullscreen` | `false` | Use the desktop video mode |
| `smooth_scaling` | `false` | Texture smoothing for the emulated frame |
| `crt_effects` | `false` | Master switch for the CRT post-process |
| `crt_scanlines` | `true` | Darken alternating source rows |
| `crt_fuzz` | `true` | Animated low-amplitude luminance grain |
| `crt_vignette` | `true` | Darken the corners and edges of the emulated screen |
| `crt_glow` | `true` | Bleed bright pixels into their neighbours |
| `crt_scanline_intensity` | `35` | Scanline strength, clamped to 0–100 |
| `crt_fuzz_intensity` | `35` | Fuzz strength, clamped to 0–100 |
| `crt_vignette_intensity` | `35` | Vignette strength, clamped to 0–100 |
| `crt_glow_intensity` | `35` | Phosphor glow strength, clamped to 0–100 |
| `theme` | `0` | Palette index 0–9: Standard, Synthwave, Gruvbox, WarmRetro, Amber Terminal, Green Phosphor, Commodore 64, CGA, Solarized Dark, Nord |
| `charset` | `standard` | `lowercase` selects the alternate font ROM |
| `rom_version` | `v8.79` | `v6.78` is the other supported value |
| `speed_regulated` | `true` | Real-speed pacing |
| `disk_a_path` | `disks/chomp.ccvf` | Drive A image |
| `disk_b_path` | `disks/life.ccvf` | Drive B image |
| `last_disk_dir` | empty | Starting directory for the disk browser |
| `sound_enabled` | `true` | SoundWare audio |
| `sound_volume` | `100` | Volume, clamped to 0–100 |

The older `crt_intensity` key is ignored; use the four per-layer intensity keys instead.

`app.config` (in `dist/`) lists plugin library paths and is not where user preferences go. Its `sound_enabled`/`sound_volume` lines are legacy and have no effect.

## Disks

The bundled `.ccvf` images are: `action_collection`, `action_soundware`, `airraid`, `blackjack`, `blank`, `chess`, `chip_33`, `chomp`, `cooltools`, `demonstration`, `hangman`, `life`, `lunar_lander`, `othello`, `sampler`, `sharks`, `startrek` and `swarms`. `chomp`, `demonstration` and `life` are writable. In `data/disks/`, only `action_soundware` is read-only; `meson install` currently installs all images writable by their owner. Use Save As if you want to preserve the original bundled image. Some titles in `action_collection` need the v8.79 ROM. Joystick games in the library cannot be controlled: there is no joystick emulation.

A `.ccvf` image is a text file: optional `Label ...` lines, then `Track N` sections with 64-hex-character data lines (32 bytes per line). Each image contains 41 tracks of 10 sectors of 192 bytes.

## Troubleshooting

- **"Missing setting '...' in app.config"** — the emulator was started from the wrong directory. Use `dist/cc2_emu.sh` or change into `dist/` first.
- **Load State… reports an error** — the path came from the file browser. A zero-byte or corrupt file (for example after a crash during saving) must be removed or overwritten with **Save State…**, or choose a different file with **Load State…**.
- **No sound** — only SoundWare-aware software produces audio; check that sound is enabled and that the volume is above zero.
- **Settings did not persist** — they are saved only on clean exit via File → Quit; closing the window by other means should also exit cleanly, but a crash will not save them.
- **A disk save fails** — the image may be read-only. Use Save As to write to a writable location.
