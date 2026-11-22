# Architecture

This document describes the emulator as implemented in this repository. User-facing behavior is covered in the [user guide](USER_GUIDE.md).

## Big picture

The application is split into a host layer and an emulator layer connected through interfaces in `include/Compucolor/`. Most emulator components live in shared libraries loaded at startup.

```text
SFML events
  -> Program / KeyboardTranslator / AsciiKeyMap
  -> CompucolorEmulatorProcessor
  -> ICompucolorEmulator (CompucolorEmulator plugin)
  -> IKeyboardEmulator

Scheduler tick (1 ms)
  -> CompucolorEmulator::StepOnce()
  -> Intel8080Emulator::Step() -> optional IDebugSink instruction events
  -> CompucolorIntel8080Bus -> optional IDebugSink port events
       -> TMS5501     ports 0x00-0x1F
       -> SMC5027     ports 0x60-0x7F (writes)
       -> CRT timing PROM ports 0x80-0x9F (reads)
  -> VectorMemory
  -> one CPU/TMS5501/SMC5027 snapshot per tick -> IDebugSink
  -> DebugHub fan-out -> DebugMonitor (UI) and JsonlDebugSink (CLI)
  -> CRT refresh thread (10 ms) -> IDisplay -> SFML texture
  -> ImGui overlay / window / monitor snapshots

TMS5501 serial/floppy path
  -> FloppyEmulator A/B
  -> timestamped TXD edges -> AudioOutput / SoundSynth
```

## Source layout

| Path | Contents |
| --- | --- |
| `include/Compucolor/` | Public interfaces (`IMemory`, `IIntel8080Emulator`, `ITms5501Emulator`, `ICrtEmulator`, ...), `IEmulatable`, plugin loader/manager and `CompucolorEmulatorProcessor`. |
| `src/Compucolor.Impl.App.Sfml/` | Host application: `main`, `Program`, SFML `Display` and `AudioOutput`, `SoundSynth`, `KeyboardTranslator`, `AsciiKeyMap`, `Gui`, `Settings`, `Configuration`, `DebugMonitor`, `DebugFilter`, `DebugHub`, `IRecordableDebugSink`, `JsonlDebugSink` and `DebugDisassembler`. |
| `src/Compucolor.Impl.Compucolor/` | Top-level machine, 8080 bus, embedded ROMs and plugin factory. |
| `src/Compucolor.Impl.{Intel8080,Memory,Crt,Smc5027,Tms5501,Floppy,Keyboard,Scheduler,Logger}/` | Hardware and service implementations; each has a `PluginFactory.cpp` exporting `create`. |
| `test/` | Nine GoogleTest executables plus shared fakes/mocks in `Compucolor.Tests.Core/`. |
| `data/` | `app.config`, `cc2_emu.sh`, ROM files and bundled disk images. |
| `scripts/` | `check_imports.py`, run at build time to reject `#include` paths containing `..`. |

## Plugin system

`IPlugin` is a marker base class. Each plugin shared library exports a `create` symbol with the signature `std::shared_ptr<T>(IPluginManager*)` (registered with `BOOST_DLL_ALIAS`). `PluginLoader<T>` calls `boost::dll::import_alias(..., "create", append_decorations)` and caches the single instance it creates; `append_decorations` lets `app.config` omit the platform suffix.

`PluginManager` is the service locator. The host passes in the logger provider, display and audio output directly; all other services are resolved lazily through their own loaders. Drives A and B have separate loaders even though they use the same library, so they are independent `FloppyEmulator` instances. `app.config` in the working directory supplies the library paths; a missing key throws and `main()` prints the error before exiting.

The ten plugins are: Logger, Memory, Intel8080, Scheduler, Smc5027, Tms5501, Crt, Floppy, Keyboard and Compucolor (the whole machine).

## Startup and the main loop

`main.cpp` installs a SIGSEGV backtrace handler, parses the optional `--trace-jsonl` argument and calls `Program::Run()`.

`Program`'s constructor, in order: resolve the settings path and load `settings.config`, create the SFML window (384×256 scaled 1–8), create the console logger provider (Debug level), load `app.config`, create the display and audio host services, build the `PluginManager`, create the `DebugMonitor`, optional `JsonlDebugSink` and `DebugHub`, subscribe those sinks to the hub, wrap the machine in `CompucolorEmulatorProcessor` with the hub as its debug sink, initialize the ImGui `Gui` with the monitor and hub, apply startup settings and mount the saved disks.

`Program::Run()` starts the emulator, then loops:

1. Dispatch SFML events to `Program` and `Gui`. Hotkeys are handled before `Gui::WantsKeyboard()`; other keyboard events go to the emulator only when ImGui does not want them. Held keys are released on focus loss and window recreation.
2. Release one-shot keys whose key-up was swallowed by the desktop (left Super/AUTO has a 200 ms fallback).
3. Advance the GUI, render the emulated frame with the ImGui overlay, and flip the window buffer.
4. Apply any pending window change (fullscreen or scale) by shutting down ImGui, recreating the window and reinitializing ImGui.
5. On exit: release all keys, stop the emulator, close the display, shut down the GUI and save settings.

`CompucolorEmulatorProcessor` is a thin host-side façade. `Start()` launches one `std::thread` that calls `ICompucolorEmulator::Start()`; that call returns after scheduling work, so the actual emulation runs on scheduler threads. `Stop()` joins the thread and stops the machine. `SaveState()`/`LoadState()` serialize to/from `./compucolor-state.json`.

## Threads and timing

| Thread/work | Period | Purpose |
| --- | --- | --- |
| Machine tick (`ScheduledTask`) | 1 ms | CPU stepping and peripheral timing. |
| CRT refresh (`ScheduledTask`) | 10 ms | Render 64×32 cells from video RAM; every 16th refresh (160 ms) trigger the TMS5501 external sensor. |
| Scheduler `RunOnce` | on demand | `std::async` one-shot task; currently unused in production. |
| SFML UI thread | per frame | Events, ImGui, window presentation. |
| SFML audio stream | 44.1 kHz mono | Render queued SoundWare edges. |

`CompucolorEmulator` uses a 502 ns CPU cycle (~1.99 MHz). In regulated mode each 1 ms tick consumes wall-clock time from a budget capped at 50 ms to avoid catch-up spirals; in unregulated mode it executes a 4,096-step burst per tick. `StepOnce()` stamps the TMS5501 with emulated time before the CPU instruction, advances the emulated clock by elapsed cycles, clears acknowledged interrupts, calls `StepFloppy()` every 8 instructions and the TMS5501 timers every 128 instructions. Reset requests are applied at tick boundaries so they never race with a CPU step.

A `ScheduledTask` runs its callback immediately, then waits against a `steady_clock` deadline; a long callback therefore delays the next run. `Stop()` is idempotent and wakes the condition variable; `Join()` blocks; the destructor stops and joins.

## Debug instrumentation

`Debug::IDebugSink` is an optional observer installed by the host. `Program` constructs a `DebugHub` and passes it to `CompucolorEmulatorProcessor`; the machine forwards it to the CPU and its bus. `Intel8080Emulator::Step()` captures the pre-step CPU state and the next three opcode bytes, calls `SetCurrentPc()`, steps the core, then emits an `InstructionEvent` with the elapsed cycles. `CompucolorIntel8080Bus::Read()`/`Write()` emit `PortEvent`s attributed to that PC.

Once per 1 ms tick, both while stepping and while paused, the machine publishes a `CpuState` snapshot with emulated time plus `Tms5501State` and `Smc5027State` snapshots obtained from the device interfaces. Those snapshots are forwarded even while recording is off so the CPU/TMS5501/SMC5027 panels stay live. Snapshots are copies taken on the scheduler thread; ImGui never reads device internals directly.

`DebugHub` is a fan-out sink with a master enable gate. Its subscriber list is mutex-protected and copied before callbacks, so a listener can be added without blocking event delivery. State snapshots and the current PC are always forwarded; `OnInstruction`/`OnPort` are forwarded only while the hub is enabled. The hub stores the current PC atomically, so bus events can be attributed even when no listener records them. Two listeners are installed by the host: `DebugMonitor` for ImGui and, when `--trace-jsonl` is supplied, `JsonlDebugSink` for a CLI/AI-consumable file. `IRecordableDebugSink` is an optional capability implemented by the JSONL sink so the hub's master switch also pauses snapshot lines; the monitor intentionally does not implement it, because its panels need live snapshots while recording is off.

`DebugMonitor` gates its own recording on `IsEnabled()`, which the Debug menu toggles together with the hub. Instruction and port events go into separate 512-entry ring buffers guarded by a mutex; CPU and peripheral snapshots share that mutex and the current PC is atomic. ImGui only reads copied snapshots/vectors, so it never touches live emulator state while scheduler threads run. Memory edits from the editor also go through `DebugMonitor`; the UI disables them unless the machine is paused.

Each event stream has a `DebugFilter`. Filters are applied when the monitor records an event: instruction filters can match a PC range and a case-insensitive substring of the disassembled mnemonic; port filters can match a port range and read/write direction. With `stopOnMatch`, the first matching event is buffered and the stream freezes until `RearmTriggers()` or `Clear()`. Filters affect only the in-memory windows; the JSONL file receives the unfiltered stream.

`JsonlDebugSink` writes one JSON object per line with a version field and its own increasing sequence number. `kind` is `cpu`, `instr`, `port`, `tms5501` or `smc5027`; byte and register fields are uppercase `0x`-prefixed strings. Writes are mutex-serialized and the stream is flushed on destruction, which makes the file suitable for offline analysis or a future CLI/AI consumer.

`DebugDisassembler` decodes all 256 8080 opcodes into uppercase mnemonics with `$XX`/`$XXXX` immediates and instruction lengths. It has no dependency on the CPU core or SFML, so the same decoder is used by the Trace window, instruction filters and its tests. Execution control is limited to the existing `Pause()`, `Resume()`, `Reset(false)` and `Reset(true)` calls; single-step, step-N, run-to-address and breakpoints are not implemented.

## Memory and I/O

`IMemory` is a 64 KiB byte-addressed array (`VectorMemory` uses atomics). Addresses `0x6000–0x6FFF` alias video RAM at `0x7000–0x7FFF`. ROM writes below `0x4000` are ignored by the CPU wrapper.

System ROMs (v6.78 and v8.79), the CRT timing PROM and the UF6 fonts are compiled into `Roms.cpp` and `FontRom.cpp`; files under `data/roms/` are installed but not read at runtime.

| Port range | Direction | Device |
| --- | --- | --- |
| `0x00–0x1F` | read/write | TMS5501 (masked to `0x00–0x0F`); port 1 keyboard rows, port 2 interrupt acknowledge, port 7 outport, ports 9–D timers. |
| `0x60–0x7F` | write | SMC5027 (masked to 0x0F); 0x06/0x0B scroll, 0x0C/0x0D cursor. |
| `0x80–0x9F` | read | CRT timing PROM (`port & 0x1F`). |
| other | read | Returns the port number. |

`ISmc5027Emulator::Read()` exists but the production bus has no SMC read path.

## Video and audio pipeline

The CRT reads two bytes per cell from `0x7000–0x7FFF`: glyph code and attributes. Attribute bit 6 is blink, bit 7 is plot graphics (the glyph byte becomes a 2×3 bitmap), bits 0–2 are foreground and bits 3–5 background. Bit 7 of the glyph byte selects double-height text. The SMC5027 scroll register (`FirstDisplayRow()`) offsets rows; blink alternates every refresh. The cursor is currently a solid white cell. The display double-buffers a 384×256 RGBA frame: the CRT thread writes a back buffer and `Repaint()` publishes it under a mutex; the UI thread uploads it to an SFML texture with a letterbox view and draws the overlay in window coordinates. Screenshots save the published frame only.

SoundWare is a 1-bit TXD line: when the serial port is selected, changes in the TMS5501 command break bit toggle the line with an emulated-time timestamp. `SoundSynth` buffers up to 8,192 timestamped edges (dropping excess), applies a ~4 kHz RC low-pass and ~16 Hz DC-blocking filter, and renders 44.1 kHz mono samples. It targets 40 ms of latency and resynchronizes if audio rendering falls more than 200 ms behind emulated time. `AudioOutput` is an SFML `SoundStream`; muting stops the stream and suppresses edge processing.

## Floppy emulation

Each drive holds 41 tracks × 10 sectors × 192 bytes. `.ccvf` files are text: optional `Label ...` lines, `Track N` sections and 64-hex-character lines (32 bytes per line). Mounting parses into a scratch image so a malformed file leaves the current disk unchanged. The drive models 76.8 kbit serial encoding, motor coast, start/data/stop framing and track stepping. The ROM version selects a 3-phase stepper (v6.78, one step per track) or 4-phase stepper (v8.79, two steps per track). A recursive mutex protects drive state because the UI can mount/eject while the emulator thread steps the drive.

Floppy bytes deliberately do **not** raise the serial receive interrupt: the ROM's RST4 handler would put them in the console input buffer, where a stray byte could corrupt typed commands such as `RUN CHOMP`.

## Save state

`IEmulatable` defines `Start`, `Stop`, `SaveState()` (an owned JSON snapshot) and `LoadState(shared_ptr<json>)`. `CompucolorEmulator::SaveState()` pauses the machine, aggregates state from CRT, both floppies, CPU, keyboard, memory, SMC5027 and TMS5501, adds scheduling/config fields (`loopInterval`, `floppyCycleRemainder`, `emulatedNanoseconds`, `romVersion`, `speedRegulated`, pending `resetRequest`), and restores the previous pause state. `LoadState()` validates fields before mutating component state, restores memory first, then the other components, and restarts wall-clock pacing. `CompucolorEmulatorProcessor` reads/writes the whole document as `compucolor-state.json` in the current working directory.

## Build and test

`meson.build` defines the project (version 0.0.2), the ten plugin shared libraries installed under `dist/plugins`, the SFML executable under `dist/bin`, and nine test executables. It overrides the `sfml` dependency and forces ImGui subprojects to static libraries so the installed app does not need a separate `libimgui.so`. `meson install --skip-subprojects` also installs `app.config`, `cc2_emu.sh`, `include/`, `data/roms/` and `data/disks/`.
