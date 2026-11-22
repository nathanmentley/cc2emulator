# User guide

This guide covers running the emulator and using its interface. See the [README](../README.md) for build instructions and the [architecture overview](ARCHITECTURE.md) for implementation details.

## Starting and stopping

Build and install as described in the README, then run the launcher:

```sh
cd dist
./cc2_emu.sh
```

The launcher changes to `dist/` because `app.config`, plugin libraries, disk images and the saved emulator state are resolved relative to the current working directory. Running the binary directly from another directory produces a "Missing setting ... in app.config" error. There are no command-line options; arguments are forwarded but ignored.

The window title is "Compucolor II Emulator". On first launch the default disks are `chomp.ccvf` in drive A and `life.ccvf` in drive B; the fresh default window scale is 4x (384×256 scaled to 1536×1024). Existing settings files are unchanged by code updates.

## Keyboard

Most keys are translated into Compucolor key presses. The emulator uses a US-layout mapping based on the character a key produces:

- Digits and punctuation follow US-layout symbols; for example `Shift+;` produces `:`. The Compucolor uses typewriter case: an unshifted PC letter produces an uppercase Compucolor letter, and `Shift`+letter produces a lowercase one.
- Numpad digits, `F1`–`F15`, `Esc`, both `Ctrl` keys, cursor keys, `Home`, `Tab`, `Insert`, `Delete`, `PageUp`, `PageDown` and `End` map to emulated keys. `End` maps to `Home`; `Pause` maps to `BREAK`.
- `Alt` maps to REPEAT unless it is part of one of the hotkeys below. Right Windows/Super is also REPEAT. Left Windows/Super maps to AUTO and is released automatically after 200 ms because some desktops swallow its key-up.
- Shift is folded into the produced character chord and is not sent as a separate emulated press.
- Multiple physical keys can hold the same emulated key; it is released only after all of them are released. All emulated keys are released when the window loses focus, closes or is recreated.

If the PC layout differs from US, the character-based mapping may not match the printed symbols. `Alt` combinations and keyboard input are ignored while ImGui needs the keyboard except for the hotkeys, which are always checked first.

### Hotkeys

| Keys | Action |
| --- | --- |
| `Alt`+`Enter` | Toggle fullscreen (recreates the window) |
| `Alt`+`1` … `Alt`+`4` | Window scale 1x–4x; also exits fullscreen |
| `Alt`+`S` | Toggle smooth scaling |
| `Alt`+`A` | Toggle SoundWare audio |
| `Alt`+`M` | Show/hide the menu |
| `Alt`+`P` | Save a screenshot |

The menu bar scales with window height and overlays the top of the window; it is excluded from screenshots. Hide it with `Alt`+`M` if it gets in the way; a short status message reminds you how to bring it back. Help → Keyboard shows the same mapping reference in the application.

## Menus

### File

- **Save State** / **Load State** — write/read `compucolor-state.json` in the working directory. Load State is enabled only when that file exists. State includes CPU, memory, both drives, keyboard, display, timers and machine configuration.
- **Mount Disk A/B…** — opens the built-in `.ccvf` browser. If the current disk has unsaved changes, you can save, discard or cancel first.
- **Eject Disk A/B** — unmounts the drive (with the same unsaved-changes prompt) and clears the saved disk path.
- **Save Disk A/B** — writes the mounted image in place immediately. It does not prompt; a failed write (for example, a read-only file) shows an Error dialog.
- **Save Disk A/B As…** — writes to a new path, adds `.ccvf` if the name has no extension, and makes that path the mounted path.
- **Save Screenshot** — writes a PNG of the raw 384×256 emulated screen to the `screenshots/` directory next to `settings.config` (by default `~/.config/cc2emu/screenshots/`). The menu overlay and window letterboxing are not included.
- **Quit** — checks both drives for unsaved changes before closing. Settings are saved on this clean exit.

The disk browser starts in the last used disk directory, otherwise `disks/`, otherwise the working directory. It lists directories and `.ccvf` files only; use `Up` to navigate, double-click or Enter to accept a file, and Cancel to close.

### Machine

- **Reset** — warm reset: resets CPU, timers, SMC5027 and keyboard but keeps memory.
- **Hard Reset** — also reloads the selected system ROM and clears the BASIC power-up flag for a cold start. Changing the ROM version performs a hard reset.
- **Pause** — stops CPU stepping; pending resets are still applied.
- **Real Speed** — when enabled (default), the CPU is paced to roughly 2 MHz with a 50 ms catch-up cap. When disabled, it runs in fast-forward bursts.

### Sound

SoundWare is an optional hardware add-on that produces audio by toggling a serial line. The emulator emulates that line, so only SoundWare-aware disks (for example `life.ccvf` / "Life with Sound" and `action_soundware.ccvf`) make sound. Use **Enable Sound** (`Alt`+`A`) and the **Volume** slider (0–100%), or set them in Preferences. The settings are stored in `settings.config` as `sound_enabled` and `sound_volume`.

### View

Scale 1x–4x, Fullscreen, Smooth Scaling, Disk Panel and Hide Menu. The Disk Panel is a floating window showing each drive's filename, a `*` for unsaved changes, and Mount/Eject/Save buttons. The menu bar also shows small status chips such as `PAUSED`, `UNREGULATED`, `SOUND OFF` and modified-disk markers when there is room.

### Settings → Preferences

- **Display**: fullscreen, window scale 1–8 and smooth scaling. The wider scale range is only available here or by editing `settings.config`.
- **Sound**: enable sound and volume, disabled when sound is off.
- **Machine**: character set (Standard or Lowercase), system ROM (v6.78 or v8.79) and Real Speed. Lowercase changes the font; the ROM choice also changes the floppy stepper geometry and triggers a hard reset.

## Settings file

Settings live in `$XDG_CONFIG_HOME/cc2emu/settings.config`, otherwise `$HOME/.config/cc2emu/settings.config`, otherwise `./settings.config`. The file is plain `key=value` lines; unknown keys are preserved. It is read at startup and written only on clean exit, so a crash can lose changes made during a session.

| Key | Default | Effect |
| --- | --- | --- |
| `window_scale` | `4` | 384×256 × scale, clamped to 1–8 |
| `fullscreen` | `false` | Use the desktop video mode |
| `smooth_scaling` | `false` | Texture smoothing for the emulated frame |
| `charset` | `standard` | `lowercase` selects the alternate font ROM |
| `rom_version` | `v8.79` | `v6.78` is the other supported value |
| `speed_regulated` | `true` | Real-speed pacing |
| `disk_a_path` | `disks/chomp.ccvf` | Drive A image |
| `disk_b_path` | `disks/life.ccvf` | Drive B image |
| `last_disk_dir` | empty | Starting directory for the disk browser |
| `sound_enabled` | `true` | SoundWare audio |
| `sound_volume` | `100` | Volume, clamped to 0–100 |

`app.config` (in `dist/`) lists plugin library paths and is not where user preferences go. Its `sound_enabled`/`sound_volume` lines are legacy and have no effect.

## Disks

The bundled `.ccvf` images are: `action_collection`, `action_soundware`, `airraid`, `blackjack`, `blank`, `chess`, `chip_33`, `chomp`, `cooltools`, `hangman`, `life`, `lunar_lander`, `othello`, `sampler`, `sharks`, `startrek` and `swarms`. `chomp` and `life` are writable. In `data/disks/`, only `action_soundware` is read-only; `meson install` currently installs all images writable by their owner. Use Save As if you want to preserve the original bundled image. Some titles in `action_collection` need the v8.79 ROM. Joystick games in the library cannot be controlled: there is no joystick emulation.

A `.ccvf` image is a text file: optional `Label ...` lines, then `Track N` sections with 64-hex-character data lines (32 bytes per line). Each image contains 41 tracks of 10 sectors of 192 bytes.

## Troubleshooting

- **"Missing setting '...' in app.config"** — the emulator was started from the wrong directory. Use `dist/cc2_emu.sh` or change into `dist/` first.
- **Load State reports an error** — `Load State` is enabled by the existence of `compucolor-state.json`. A zero-byte or corrupt file (for example after a crash during saving) must be removed or overwritten with Save State.
- **No sound** — only SoundWare-aware software produces audio; check that sound is enabled and that the volume is above zero.
- **Settings did not persist** — they are saved only on clean exit via File → Quit; closing the window by other means should also exit cleanly, but a crash will not save them.
- **A disk save fails** — the image may be read-only. Use Save As to write to a writable location.
