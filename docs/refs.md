# Reference repos (`refs/`)

`refs/` is gitignored. Recreate it with `git clone --depth 1 <url> refs/<dir>`.

Licenses and what copying from each one means are in `.cursor/rules/non-negotiables.mdc`, section 5. Flag any copying before doing it.

## The base

### `dreamcomp` (GPL-2.0)
https://github.com/danielgomesvieira2000/dreamcomp

**The base this project builds on**, which makes this project GPL-2.0. A framework for native ports of Dreamcast games: SH-4 to C++ static recompiler, emulated hardware, Vulkan renderer, SDL3 window and audio. It replaces KatanaRecomp, which isn't public.

- `engine/` is a vendored copy of [dream-recomp](https://github.com/phobos665/dream-recomp) (phobos665). `engine/translator/` is the recompiler; `engine/runtime/` is the hardware (PVR2, AICA, GD-ROM, Maple), with portions taken from Flycast.
- `engine/docs/differential-harness.md` and `engine/tests/sh4/`: translator tests that compare against Flycast's SH-4 interpreter. This is the model for our translation tests.
- `engine/docs/runtime-*.md`: design notes per hardware block (interrupts, PVR, AICA, Maple, memory).
- `docs/PORTING-GUIDE.md`: bringing up a new game. `docs/playbook/README.md`: Dreamcast traps, listed by symptom.
- `docs/HOOKS.md`, `include/dreamcomp/`, `src/`: the port layer (hooks into game functions, settings, launcher).
- `docs/PLATFORMS.md`: Windows x64 is built and tested, Linux compiles, macOS ARM64 runs through MoltenVK, Android is only a plan. Our macOS and Android targets need work here.
- Reads `.chd`, `.gdi` and `.cue` disc images. The `libchdr` submodule isn't fetched.
- Each port is its own repo. The one public port is [soulcalibur-recomp](https://github.com/danielgomesvieira2000/soulcalibur-recomp) (not in `refs/`); `engine/games/crazytaxi/` is the engine's baseline game config. Nothing exists for Hydro Thunder.

## Other Dreamcast recompilers (compare approaches)

### `chuchu` (MIT)
https://github.com/sp00nznet/chuchu

ChuChu Rocket! recomp, SH-4 to C. Read `README.md` first: it's a map of the runtime problems in the order you hit them (boot, flash, GD-ROM, Holly interrupts, AICA and its ARM7, TA lists, rendering). Early state: no textures, sound or input.

The recompiler itself is the `dcrecomp` submodule, which isn't fetched, so `chuchu/dcrecomp/` is empty. Read `refs/dcrecomp` instead.

### `dcrecomp` (GPL-2.0)
https://github.com/sp00nznet/dcrecomp

The framework under `chuchu`: SH-4 to C for Dreamcast and Naomi, with an SDL2 and OpenGL 3.3 runtime. Alpha, with no conformance tests. The recompiler is Python (`tools/static_recompile.py`, `tools/sh4_disasm.py` for function finding), which makes it the quickest one to read end to end. `src/hal/` is a small hardware layer written from reading Flycast (see `NOTICE`); `tools/extract_gdi.py` extracts a disc.

The clone is the latest commit, which is newer than the one `chuchu` pins (`90ee221`).

### `sfa3-recomp` (GPL-3.0)
https://github.com/irnutsmurt/Street-Alpha-3-Dreamcast-Recomp

Street Fighter Alpha 3 recomp, a third independent approach: SH-4 to C, Vulkan renderer, SDL3 input and audio, Linux and Windows. See `runtime/` for the hardware layer (plain C, one file per block: `sh4.c`, `pvr.c`, `aica.c`, `irq.c`, `maple.c`), `tools/recomp/` for the recompiler, `tools/chd2gdi.c` for CHD reading, `sfa3port.py` for the user-facing setup (verify disc, recompile, build), and `docs/dreamcast.md`.

### `N64Recomp` (MIT)
https://github.com/N64Recomp/N64Recomp

The architecture all of the above descend from: a per-game config drives a tool that turns each function into C, and a separate runtime supplies the hardware. Different CPU (MIPS), so use it for structure, not instruction semantics: function discovery, jump tables, overlays, config format, the generated-code-to-runtime interface, and `LiveRecomp/` for code that can't be translated ahead of time.

## Oracles and hardware references

### `flycast` (GPL-2.0-or-later)
https://github.com/flyinghead/flycast

The reference for hardware behavior and the debugging oracle. When recompiled code and Flycast disagree, Flycast is right until proven otherwise.

- `core/hw/sh4/`: CPU interpreter, the oracle for translation tests.
- `core/hw/pvr/` and `core/rend/`: PVR2 tile accelerator and the renderers.
- `core/hw/aica/`: sound chip and its ARM7.
- `core/hw/holly/`: interrupt controller and system bus. `core/hw/gdrom/`, `core/hw/maple/`: disc and controllers.
- `core/debug/`: GDB server, for stepping the real game.

### `tbg-decomp` (no license)
https://github.com/lhsazevedo/tokyo-bus-guide-decomp

Tokyo Bus Guide decompilation. Useful for how it tests: each decompiled function is run against the original in an SH-4 simulator. See `tests/` and `tests/dsl-reference.md` for the test style, and `src/` for what Dreamcast game code built with the Hitachi compiler looks like in C.

The simulator is a separate project, `refs/sh4objtest`.

### `sh4objtest` (MIT)
https://github.com/lhsazevedo/sh4objtest

The "tbg SH-4 simulator" the rules refer to. An SH-4 simulator and test framework written in PHP 8.3 (`src/Simulator/Simulator.php`). A test sets up registers and memory, mocks the functions the code calls, runs it, and asserts on memory writes and calls.

Limits as a translation oracle:

- Its input is Hitachi compiler object files (SysROF `.obj`), not raw binaries, ELF, or a disc's `1ST_READ.BIN` (`src/ObjectParser.php`).
- Instruction coverage is partial. `tbg-decomp` notes at least one opcode it lacks (`MOV.W @Rm+,Rn`), and the source has TODOs for float and system registers.
- Needs PHP 8.3 and Composer.

So Flycast is the oracle for instruction semantics. This is the reference for function-level tests with mocked callees.

### `KallistiOS` (BSD-style KOS License)
https://github.com/KallistiOS/KallistiOS

Open-source Dreamcast SDK. The most readable source for hardware register definitions and how drivers use them.

- `kernel/arch/dreamcast/include/dc/`: register and structure definitions (PVR, AICA, Maple, GD-ROM, ASIC interrupts).
- `kernel/arch/dreamcast/hardware/`: driver code that uses them.
- `examples/dreamcast/`: small programs that can be compiled into SH-4 test cases.

Hydro Thunder was built with Sega's Katana SDK, not KOS, so this documents the hardware, not the game's own library calls.

### `SDL` (Zlib)
https://github.com/libsdl-org/SDL

SDL3 source, so API usage is checked against the real headers. `include/SDL3/SDL_gpu.h` for the GPU API; `include/SDL3/` for everything else; `docs/` for per-platform notes, including Android and macOS.
