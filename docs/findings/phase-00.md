# Phase 00: survey, translation, first run (M1)

macOS arm64 (Apple clang 21, CMake 4.2, Ninja), 2026-11-16. Windows, Android and Linux are not
built yet (M7).

## Identity

`dcdisc info` / `dcdisc inspect` on the owner's CHD (libchdr from the vendored submodule build):

| Field | Value |
|---|---|
| Title | HYDRO THUNDER, MIDWAY (maker SEGA ENTERPRISES) |
| Product | `T-9702N` `V1.020`, released 1999-10-04, region USA |
| Device | `5154 GD-ROM1/1`, IP.BIN CRC ok |
| Peripherals | `0x0799A10`: VGA box, Puru Puru pack, VMU, Start, A, B, D-pad, X, Y, analog L/R triggers, analog stick X/Y |
| Windows CE | no: peripherals bit 0 clear, no `0WINCEOS.BIN`. Katana SDK |
| Boot file | `1ST_READ.BIN`, 1,888,290 bytes, stored plain |
| Boot SHA-1 | `d8d54a28eac55cb4b63d9693951f9582a1b2505a` |
| SDK banners | Shinobi x2 (`syStart Ver 1.07 Build:Mar 02 1999`), KAMUI x3, AICA x2 |
| Tracks | 37: data 1, audio 2, HD data 3 (LBA 45000), audio 4-36 (33 CD-DA tracks), data 37 |
| Streaming media | none |
| Sound driver | `AUDIO64.DRV` with `.KAT` banks |

Test: `tests/test_identity.py` (CTest `hydrothunder.identity`) asserts product, version, Windows CE
flag clear and boot SHA-1 through dcdisc's `inspect_image`. Disc from `$HT_DISC`, else
`.dreamcomp/local.json`; exit 77 (CTest skip) when neither names a file. Verified passing, and
skipping with the local config moved away.

Open question for M3: `dcdisc inspect`'s code heuristic also flags `GRAV.R2` and `NILE.R2`
(16-17 MB each) as code. Both are referenced by name from the boot file, like every other `.R2`.
Probably track data with a pointer-heavy layout (inference, not checked).

## Extraction

`DCDISC_LIBCHDR=<build>/libchdr.dylib python3 dreamcomp/tools/dc.py setup . --disc <chd>` wrote
only `game/extracted/fs/1ST_READ.BIN` (SHA-1 above, git-ignored) and `.dreamcomp/local.json`.
`DCDISC_LIBCHDR` is needed because the vendored tree's `.gitmodules` files stop dcdisc's search for
the repository root at `dreamcomp/engine/`, which has no build directory.

## Translator report

`dream-translate game --config game/hydrothunder.toml` (seed: entry `0x8C010000` only, no
relocations, no extra seeds):

| Measure | Value |
|---|---|
| Functions | 3,990 (1 seed, 1,731 call, 1,497 pointer, 339 branch, 422 sweep); 0 named |
| Instructions emitted | 320,655 |
| Decode coverage | 608,716 of 694,324 bytes in function ranges (87.7%); 56,724 bytes in 555 runs of 16+ bytes neither code nor pool. 32.2% of the whole file is code |
| Literal-pool pointers | 3,546 of 11,745 candidates accepted as function entries |
| Unresolved indirect sites | 537: `no_constant` 474 (407 `jsr`, 67 `jmp`), `out_of_image` 63 (61 `jsr`, 2 `jmp`), `no_table` 0 |
| Switch tables | 37 found by discovery, 43 lowered by the emitter; 2 `braf r0` with no table emitted as tail calls |
| Calls | 7,493 `jsr`/`bsrf` resolved to direct calls; the 537 unresolved sites go through `call_indirect` (table lookup) |
| FP mode | 51,704 run-time PR/SZ tests; 1,452 entry-mode clones (65 tests left in them); 11 functions with unknown FPSCR effect; 0 entry modes inferred (3,990 ambiguous) |
| Patched pool words (not folded) | 131 |
| Not lowered | 51, all words that decode to no SH-4 instruction (`.word 0x0000`, `0x8c1e`, ...: data reached by discovery, mostly around `0x0c0e0e20..0x0c0e4dbc` and `0x0c1b05cc..0x0c1d47f8`). They compile to `unimplemented()`, a fatal runtime fault. 0 unsupported valid instructions |

The emitter now fails the translation on any valid instruction it cannot lower
(`dreamcomp/docs/engine-changes.md`, 2026-11-16).

The generated `build/game/gen/hydrothunder.cpp` (55 MB) includes only
`dream/runtime/gen_interface.h` (interface version 1, `static_assert`ed). It builds as the static
library `hydrothunder_gen` (29 MB, `-O1`), linked whole into `game/hydro-thunder` (headless: no
Vulkan SDK or `glslc` on this Mac, so `DREAM_RENDERER` is off; SDL3 audio is on). Full build from
scratch about 90 s.

## Engine tests on macOS

`ctest` in `build/` with `-DDREAM_BUILD_TESTS=ON`: 7 of 7 pass.

| Test | Result |
|---|---|
| `translator.unit` | 14 cases, 117 assertions |
| `runtime.unit` | 75 cases, 121,137 assertions |
| `render.unit` | 96 cases, 203,942 assertions (CPU-side; the Vulkan backend is not built) |
| `translator.programs` | 9 cases, 107,220 assertions. Golden replay: 51 Flycast-captured states replayed in 3 FP modes; 31 skipped (`ct_*`, they need Crazy Taxi's disc image, which is not here) |
| `translator.version` | pass |
| `dcdisc.pytest` | 26 passed, 3 skipped (need MAME `chdman`); needs `pytest` and `pycdlib` (in the ignored `.venv`) |
| `hydrothunder.identity` | pass |

The vendored tree has 82 golden files, not the 78 the plan quotes; 51 run without another game's
disc. The Flycast differential test (`translator.differential`) needs the oracle core and is M2.

## First headless run

```
build/game/hydro-thunder --config game/hydrothunder.toml --disc <chd> \
  --max-frames 1800 --rtc-seed 1 --unthrottled --report build/first-run-report.txt
```

Stop reason: `frame limit` (1,800 frames, 29.9 s guest time, 3.6 s host, 8.1x real time). No fault,
because the development interpreter (`DREAM_DEV_INTERPRETER=ON`) ran every untranslated target.
This is not a boot: nothing has been looked at on screen.

| Report line | Value |
|---|---|
| Untranslated call targets | 8,021 calls to 6 addresses: `0x8c000600` x5,973 (a 56-byte copy of image `0x0c1cbb78`, the interrupt vector), `0x8c0944bc` x1,816, `0x8c021020` x227, `0x8c094d14` x2, `0x8c0951a8` x2, `0x8c1c1024` x1 (the last five are in the image itself: missing seeds) |
| Candidate relocations | RAM `0x8c00007c` (image `0x0c1d4880`, 52 B), `0x8c000100` and `0x8c000400` (image `0x0c1cbb44`, 56 B), `0x8c000600` (image `0x0c1cbb78`, 56 B) |
| Unmapped memory accesses | 0 |
| Interrupts | 5,973 delivered, max nesting 1; 0 traps; 0 task-switching RTEs |
| GD-ROM | 5,227 sectors read; 2 CD-DA syscalls (`gdrom.cdda=2`) |
| PVR | 707 renders (706 done), 708 frame swaps, 1,627 CH2 DMA transfers, 0 Sort-DMA |
| Maple | 1,775 pad polls |
| AICA | ARM7 running `AUDIO64.DRV` (95.7 M instructions, 29,601 FIQs); mixer 0 key-ons, every sample zero; 0 SH-4 IRQ raises from the ARM |
| Self-modified image | 690 byte ranges changed (single bytes from `0x0c0af246`, likely data inside the image) |

Next action (M3): paste the five `[functions] extra` seeds and the four vector relocations from
`--suggest-config` into `game/hydrothunder.toml` after checking each against the disassembly, then
rerun.
