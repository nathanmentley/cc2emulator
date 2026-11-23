# Deterministic Save-State and Replay Plan

> Status: Phases 1–3 are implemented. This document keeps the original plan structure; current launchers are `dist/cc2_emu.sh` for the GUI and `dist/cc2_replay.sh` for the headless runner.

## Goal

Add a reproducible debugging and integration-test workflow:

1. Start the emulator from a saved state.
2. Replay timestamped emulated input.
3. Capture deterministic output, initially JSONL trace and final state.
4. Use the same bundle as a regression test fixture.

The intended result is that a bug report can be reduced to:

```text
bundle file + expected trace/state
```

instead of a long manual sequence of GUI actions.

---

## What already exists

The project already has most of the hard infrastructure:

- `ICompucolorEmulator::SaveState()` / `LoadState()`
  - Captures CPU, memory, both floppies (including disk contents), keyboard, CRT, SMC5027,
    TMS5501, emulated time, and configuration.
  - `LoadState()` intentionally resets wall-clock pacing state on restore.
- `CompucolorEmulatorProcessor`
  - No fixed-path overload remains; the GUI and CLI always pass explicit paths.
- `DebugHub`, `DebugMonitor`, and `JsonlDebugSink`
  - Can capture CPU, instruction, port, TMS5501, and SMC5027 records without the GUI.
- `FakeScheduler` and injected clock in `Compucolor.Impl.Tests.Compucolor`
  - Demonstrate that emulator pacing can be made deterministic in tests.
- `CompucolorIIKey`
  - The right vocabulary for replay input. Replay must not store host/SFML key codes.

The missing pieces are a replay bundle format, a deterministic runner, input recording, and
test fixtures.

---

## Design overview

### Replay bundle

Use a single versioned JSON file. Keep the existing save-state JSON nested inside it rather
than changing the state format.

Proposed filename extension: `.cc2replay.json`.

```json
{
  "version": 1,
  "createdBy": "0.0.2",
  "state": {
    "...": "existing CompucolorEmulator::SaveState() document"
  },
  "input": [
    { "t": 123456789, "key": "A", "down": true },
    { "t": 123500000, "key": "A", "down": false }
  ],
  "stop": {
    "emulatedNanoseconds": 2000000000
  }
}
```

Rules:

- `t` is **absolute emulated nanoseconds**, never wall-clock time.
- Events must be sorted by `t` and non-decreasing.
- Events with `t < state.emulatedNanoseconds` are invalid and rejected at parse time.
- `key` is the string form of `CompucolorIIKey`.
- `stop` is exclusive: replay continues while emulated time is less than the target.
- Multiple events at the same `t` are applied in file order.

Do not embed the trace in the bundle initially. Keep trace output as a separate JSONL file so
filters and recording modes can change without invalidating bundles.

### Determinism rules

A replay run must:

- construct real emulator components without SFML or a GUI
- use a manual scheduler whose tick is driven explicitly by the runner; the CRT's 10 ms refresh
  task is registered on the same scheduler so its phase cannot depend on thread timing
- call `Start()` first, because `Start()` loads the ROM, then call `LoadState(bundle.state)`
- call `SetSpeedRegulated(false)` after loading state, so wall-clock pacing cannot affect step count
- enable the debug hub only after the state is loaded, so startup work does not enter the trace
- inject input using emulated time, not host time
- stop at the bundle’s `stop.emulatedNanoseconds`

With speed regulation off, each manual scheduler tick runs a fixed number of CPU steps, so the
same bundle should always produce the same instruction sequence.

---

## CLI surface

The CLI is the contract for all three capabilities. The GUI should mirror these flags where it
makes sense.

### GUI app (`cc2_emu.sh` / `Compucolor.Impl.App.Sfml`)

| Flag | Purpose |
| --- | --- |
| `--load-state <path>` | Start from a saved state JSON file. |
| `--replay-bundle <path>` | Start from a bundle’s state and replay its input events. |
| `--record-replay <path>` | Record a replay bundle for the session. |
| `--trace-jsonl <path>` | Write JSONL trace, filtered by repeatable `--trace-filter` when supplied. |
| `--trace-filter <spec>` | Restrict which records are written to JSONL. Repeatable. Added in Phase 3. |

Rules:

- `--load-state` and `--replay-bundle` are mutually exclusive.
- `Start()` still runs first because it loads the ROM and starts devices; the saved state is
  loaded immediately afterward.
- For `--replay-bundle`, input is injected from emulated time. GUI replay is for visual
  confirmation; the headless runner remains the deterministic oracle.
- For deterministic capture, prefer the headless runner. The GUI app remains wall-clock paced
  unless speed regulation is explicitly disabled.

### Headless runner (`Compucolor.Impl.App.Headless`)

| Flag | Purpose |
| --- | --- |
| `--bundle <path>` | Required replay bundle. |
| `--trace-jsonl <path>` | Write JSONL trace for the replay. |
| `--trace-filter <spec>` | Same filter syntax as the GUI app (Phase 3). |
| `--expected-state-hash <sha256>` | Fail if the final canonical state hash differs. |

---

## CLI trace filter syntax

Filters apply **only** to `JsonlDebugSink`. They do not affect `DebugMonitor`, the GUI trace
windows, or the emulator itself. With no filters supplied, JSONL stays fully unfiltered.

Supported forms:

```sh
--trace-filter <spec>
--trace-filter=<spec>
```

Spec format:

```text
kind[:key=value[,key=value...]]
```

Kinds:

- `cpu`
- `instr`
- `port`
- `tms5501`
- `smc5027`

Keys (applicability is strict; an invalid key/kind pair fails at startup):

- `pc=from-to` — hex range, inclusive; also accepts a single address. Valid for `cpu`, `instr` and `port`.
- `port=from-to` — hex range, inclusive; also accepts a single port (0–255). Valid for `port`.
- `dir=read|write|any` — port direction. Valid for `port`.
- `text=substring` — case-insensitive substring of the disassembled instruction. Valid for `instr`.

`tms5501` and `smc5027` accept no keys and are used as bare kinds. `text` is computed only for matching and is not written to JSONL records, so the no-filter output shape is unchanged.

Examples:

```sh
--trace-filter instr:pc=0x0100-0x01ff
--trace-filter instr:text=MOV
--trace-filter port:port=0x80-0x8f,dir=write
--trace-filter cpu --trace-filter smc5027
```

Semantics:

- Multiple filters are ORed.
- A record is written if at least one supplied filter for its kind matches.
- If no filter mentions a kind, that kind is not written.
- Invalid specs fail at startup with an error naming the bad filter.
- `seq` numbers remain dense over the records actually written.

This keeps the existing “JSONL is unfiltered by default” behavior while making large traces
manageable when explicitly requested.

---

## Proposed file layout

| Path | Purpose |
| --- | --- |
| `include/Compucolor/App/Replay/ReplayBundle.h` + `src/Compucolor.Impl.App/ReplayBundle.cpp` | Bundle parse, validation, serialization |
| `include/Compucolor/App/Replay/ReplayKeyNames.h` + `src/Compucolor.Impl.App/ReplayKeyNames.cpp` | `CompucolorIIKey` ↔ string mapping |
| `include/Compucolor/App/Replay/InputTimeline.h` + `src/Compucolor.Impl.App/InputTimeline.cpp` | Sorted input events and due-event lookup |
| `include/Compucolor/App/Replay/ManualScheduler.h` + `src/Compucolor.Impl.App.Headless/ManualScheduler.cpp` | Production `IScheduler` with explicit `Tick()` |
| `include/Compucolor/App/Replay/InputRecorder.h` + `src/Compucolor.Impl.App/InputRecorder.cpp` | Records timestamped emulated key events |
| `include/Compucolor/App/Replay/ReplayRunner.h` + `src/Compucolor.Impl.App.Headless/ReplayRunner.cpp` | Loads state, drives ticks, emits trace |
| `include/Compucolor/App/TraceFilterConfig.h` + `src/Compucolor.Impl.App/TraceFilterConfig.cpp` | CLI JSONL filter parsing and matching |
| `include/Compucolor/App/DebugDisassembler.h` + `src/Compucolor.Impl.App/DebugDisassembler.cpp` | Instruction text for GUI trace windows and `text=` filters |
| `src/Compucolor.Impl.App.Headless/replay_main.cpp` | Headless CLI entry point |
| `test/Compucolor.Impl.Tests.Replay/` | Unit and golden replay tests |
| `test/Compucolor.Impl.Tests.Replay/fixtures/` | Small `.cc2replay.json` fixtures |

All new sources should be compiled into the replay runner and the replay test target. Only the
files that need SFML should remain in the GUI executable.

---

## Phase 1 — Headless replay foundation

This phase does not record bundles yet. It proves that a bundle can be replayed deterministically.

### Tasks

1. **Add replay types**
   - `ReplayEvent`
   - `ReplayBundle`
   - `ReplayKeyNames`
   - `InputTimeline`

2. **Implement bundle parsing and validation**
   - Reject unknown `version`.
   - Reject non-monotonic event times.
   - Reject unknown key names.
   - Reject missing state or stop condition.
   - Produce clear error messages including the offending field.

3. **Add `ManualScheduler`**
   - Implements `IScheduler`.
   - Stores the recurring tick callback.
   - Exposes `Tick()` and `Interval()`.
   - Can be reused by tests instead of duplicating `FakeScheduler`.

4. **Add `ReplayRunner`**
   - Accepts real emulator components, a `DebugHub`, and a replay bundle.
   - Starts the emulator.
   - Loads the saved state.
   - Disables speed regulation.
   - Runs manual ticks until `stop.emulatedNanoseconds`.
   - Applies input events due at each tick boundary.
   - Stops the emulator cleanly.

5. **Add headless CLI**
   - Proposed executable: `Compucolor.Impl.App.Headless`.
   - Flags:
     - `--bundle <path>`
     - `--trace-jsonl <path>`
     - `--expected-state-hash <sha256>` (optional)
   - No SFML window, audio device, or GUI dependency.

6. **Add golden tests**
   - Parse/validate tests for `ReplayBundle`.
   - Key-name round-trip tests for every `CompucolorIIKey`.
   - One golden scenario that creates a small seed state at runtime, replays a few keys, and emits
     a trace. A checked-in state JSON would be large because it contains both 78 KB floppy images,
     so the test generates the state and pins the canonical hash instead.
   - Repeat the same replay twice and assert identical trace bytes.
   - Assert a canonical final-state hash.

### Acceptance criteria

- `Compucolor.Impl.App.Headless --bundle fixture.cc2replay.json --trace-jsonl out.jsonl` works without a display.
- Running it twice produces byte-identical JSONL output.
- Invalid bundles fail with actionable error messages.
- The replay test runs in CI.

---

## Phase 2 — Input recording in the GUI app

Once replay works, add the ability to produce bundles from a live session.

### Tasks

1. **Expose emulated time**
   - Add `GetEmulatedNanoseconds()` to `ICompucolorEmulator`.
   - Implement it in `CompucolorEmulator`.
   - Forward it through `CompucolorEmulatorProcessor`.
   - Already landed with Phase 1 because the headless runner needs the same clock.

2. **Add `InputRecorder`**
   - Enabled explicitly by CLI flag or GUI action.
   - Records `CompucolorIIKey` down/up events with current emulated time.
   - Protects the event list with a mutex because input arrives on the UI thread.
   - Enforces monotonic timestamps.

3. **Wrap input in `Program`**
   - `PressPhysicalKeys()` and `ReleasePhysicalKey()` are the natural hook points.
   - Record each emulated key in a chord, not the physical key.
   - Continue forwarding to the emulator exactly as today.

4. **Add named state paths**
   - Add overloads:
     - `CompucolorEmulatorProcessor::SaveState(path)`
     - `CompucolorEmulatorProcessor::LoadState(path)`
   - Keep the existing fixed-path methods for GUI compatibility.

5. **Add recording CLI**
   - Proposed flag: `--record-replay <path>`.
   - Start recording at launch.
   - Capture the initial state immediately after startup.
   - On exit, write a bundle containing that state, the recorded input, and the final emulated time as `stop`.

6. **Add GUI action**
   - File → Start Replay Recording.
   - Capture state at the moment recording starts.
   - Stop recording writes the bundle to a chosen path.

7. **Add GUI startup flags**
   - `--load-state <path>` calls `LoadState(path)` immediately after `Start()`.
   - `--replay-bundle <path>` loads the bundle state, then feeds the bundle’s input events through
     `InputTimeline` during normal ticks.
   - Reject `--load-state` and `--replay-bundle` together.

8. **Expose replay progress in the GUI**
   - Show current emulated time, next input event time, and stop target.
   - This makes GUI replay useful for watching a bug, while the headless runner remains the source
     of truth for golden comparisons.

### Acceptance criteria

- A session recorded with `--record-replay` can be replayed by `Compucolor.Impl.App.Headless`.
- `./dist/cc2_emu.sh --load-state <path>` starts the GUI app from the saved state.
- `./dist/cc2_emu.sh --replay-bundle <path>` starts the GUI app from the bundle state and replays its input.
- Replay produces the same final memory/state hash as the recorded run.
- Replay produces the same instruction/port trace when both runs enable the same trace settings.

### Implementation status

Phase 2 is implemented. Notes on the delivered shape:

- `InputRecorder` lives in the SFML-free shared app library. It captures the startup/current state as a JSON value, records `CompucolorIIKey` down/up transitions with emulated timestamps, clamps timestamps to be non-decreasing, and builds a version 1 bundle with `createdBy = "cc2emu"`. A zero-length GUI recording gets `stop = state.time + 1` so the bundle stays valid.
- `Program` records at the existing emulated-key transition points, including `ReleaseAllKeys()` on focus loss and shutdown. Live input forwarding is unchanged.
- `CompucolorEmulatorProcessor` now exposes `GetState()`, `SaveState(path)`, `LoadState(path)` and `LoadState(value)`. The fixed `compucolor-state.json` methods have been removed.
- `ProgramArguments` parses `--trace-jsonl`, `--load-state`, `--replay-bundle` and `--record-replay` in both `--flag value` and `--flag=value` forms, rejects duplicates/unknown flags, and enforces the mutual exclusions.
- GUI replay uses a `ReplayInputSink` debug sink. An opt-in `IReplayTickSink` boundary fires at the start of each 1 ms machine tick, before CPU work, so a replayed key contact is visible to the ROM's keyboard scan even when tracing makes ticks coarse. CPU snapshots still update progress reporting. The Debug → Replay Status window reads host-safe progress snapshots and reports emulated time only.
- `CompucolorEmulatorProcessor::Start()` now waits for emulator initialization before returning. This prevents the startup ROM load/reset from overwriting a replay state that `Program` loads immediately afterward, a race that tracing could make visible.
- The GUI master trace switch still controls both `DebugMonitor` and the JSONL sink; JSONL recording stays off unless `--trace-jsonl` is supplied.
- GUI replay is a visual confirmation path. The headless runner remains the deterministic oracle for final state hashes and instruction/port traces.
- While a GUI replay is active, `Program` ignores live keyboard events instead of forwarding them as emulated key events. Replay is the only producer of emulated keyboard input until its stop time is reached. Host hotkeys, menus and Pause/Resume remain available.
- CLI trace filters are implemented in Phase 3; the GUI master switch still gates the JSONL sink, and `--trace-filter` restricts which records that sink writes.

---

## Phase 3 — Replay-aware trace filters

With deterministic replay fixtures available, trace filtering becomes much easier to validate.

### Tasks

- Add `TraceFilterConfig` and the parser for the syntax documented above.
- Apply filters in `JsonlDebugSink` before `WriteLine()`.
- Expose `--trace-filter` in both `cc2_emu.sh` and `Compucolor.Impl.App.Headless`.
- Keep no-filter behavior unchanged.
- Use replay fixtures to assert that filtered traces contain exactly the expected records.

### Acceptance criteria

- Full trace and filtered trace are both reproducible from the same bundle.
- Filters affect only JSONL output; GUI trace windows are unchanged.
- Invalid filter specs fail at startup with actionable errors.
- Filter behavior is covered by golden tests using replay fixtures.

### Implementation status

Phase 3 is implemented. Notes on the delivered shape:

- `TraceFilterConfig` is SFML-free and compiled into `libCompucolor.Impl.App.a`. It parses bare kinds, inclusive hex ranges/single values, port direction and case-insensitive instruction text, rejects unknown or inapplicable keys at startup, and ORs repeated filters.
- `JsonlDebugSink` applies filters before `WriteLine()`, so `seq` remains dense over written records. No `text` field is added to JSONL records; `DebugDisassembler.cpp` is part of the shared app library so `text=` can decode instruction bytes for matching.
- Debug callbacks only copy small snapshots onto a queue. Filtering, formatting and file I/O run on a background writer thread, and `Flush()` drains that queue, so tracing does not skew the GUI replay's wall-clock pacing.
- `DebugMonitor`'s UI-only instruction/port buffers are captured only while the Trace or Ports debug window is visible. Enabling JSONL tracing no longer pays for buffer maintenance that nothing displays.
- Debug → Replay Status reports applied/total replay input events so input delivery can be distinguished from display or pacing issues.
- `--trace-filter <spec>` and `--trace-filter=<spec>` are accepted by both the GUI app and `Compucolor.Impl.App.Headless`. GUI filters in the Debug windows remain independent of JSONL filtering.
- The headless CLI explicitly flushes the sink after replay so small filtered traces are completely written.
- Tests cover parser validation and matching, sink filtering with dense `seq`, GUI argument parsing, and a replay golden trace that compares filtered output with the expected records from the full trace.

---

## Phase 4 — Optional refinements

These are useful but not required for the first useful version.

- **Instruction-level input injection**
  - Tick-boundary injection is simpler but slightly coarse.
  - Add a pre-step hook in `CompucolorEmulator` if a bug depends on exact instruction timing.

- **Compact bundle storage**
  - Floppy images are currently stored as JSON byte arrays, which is verbose.
  - Consider base64 or external disk references later.

- **Reset/pause/mount events**
  - Record resets, pause/resume, disk mount changes, ROM changes, and configuration changes.

- **CRT image output**
  - Store a hash of character/display RAM from final memory state, or render a headless framebuffer checksum.

- **Bundle metadata**
  - Emulator version, creation time, host OS, and notes field.

---

## Testing strategy

### Unit tests

- Bundle parsing and validation.
- Key-name round trips.
- InputTimeline ordering and due-event lookup.
- InputRecorder timestamp monotonicity.
- Canonical state hashing.

### Integration tests

- Load real save state, replay input, emit trace.
- Repeatability: same bundle → identical trace.
- Golden trace comparison.
- Golden final-state hash comparison.
- Invalid bundle rejection through the CLI.

### CI

Add the replay tests to the existing Meson test suite. CI already builds the devcontainer image
with SFML 3, so the replay tests can reuse that environment even though replay itself should not
need SFML.

---

## Risks and decisions to confirm

- **Wall-clock pacing**: must be disabled during replay; otherwise results will drift.
- **Input granularity**: start with tick boundaries; add instruction boundaries only if needed.
- **State size**: bundles containing disk images will be large but self-contained.
- **Versioning**: reject incompatible state/version combinations instead of guessing.
- **Host-only bugs**: GUI, SFML, physical key mapping, and audio-device issues may need a live capture rather than a replay bundle.
- **ROM reload ordering**: `Start()` loads ROM, so `LoadState()` must happen afterward.

---

## Definition of done

The first useful version is complete when:

1. A `.cc2replay.json` bundle can be loaded by a headless runner.
2. Emulated key events are replayed at deterministic times.
3. `./dist/cc2_emu.sh --load-state <path>` starts the GUI app from a previous state.
4. `./dist/cc2_emu.sh --replay-bundle <path>` starts the GUI app and replays controlled input.
5. A JSONL trace can be captured from both GUI and headless runs.
6. `--trace-filter` can reduce the JSONL output without changing GUI trace behavior.
7. The same bundle produces byte-identical trace output on repeated headless runs.
8. A recorded bundle from the GUI can be replayed headlessly.
9. At least one replay scenario runs in CI as a regression test.
