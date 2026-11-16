# M2: reference tests (macOS)

macOS arm64, 2026-11-16. Windows, Android and Linux are not built yet (M7).

## Oracle

Flycast libretro core built by a separate agent into `build-oracle/flycast-oracle/flycast_libretro.dylib`
from `build-oracle/flycast-src` at `0abac3465dc9547dca5f30f3352fee10b67e34b2`, the commit pinned in
`dreamcomp/engine/tools/flycast/oracle/build_oracle.sh`. Configure with
`-DDREAM_FLYCAST_CORE=<that dylib>`.

## Results

`ctest` in `build/`: 9 of 9 pass.

| Suite | Result |
|---|---|
| `translator.differential` (engine cases, `tools/oracle/cases.json`) | 52 / 52 agree with Flycast |
| `hydrothunder.differential` (Hydro Thunder slice, `game/oracle-cases.json`) | 30 / 30 agree with Flycast |
| `translator.programs` golden replay | 81 states replayed in 3 modes (translated, interpreted, hybrid): 51 engine + 30 Hydro Thunder; 31 Crazy Taxi states skipped (that disc is not here) |
| other engine tests, dcdisc pytest, identity | pass (see `phase-00.md`) |

A Hydro Thunder golden with one FR bit flipped fails the replay; restored, it passes.

## Hydro Thunder slice

`tools/oracle/pick_slice.py` on `build/game/gen/hydrothunder.cpp` (after fixing it for the current
emitter, see `dreamcomp/docs/engine-changes.md`): the top 20 pure-FPU leaves and top 30
memory-writing leaves, 50 candidates. Same register and fill scheme as the Crazy Taxi slice:
pointers into a 4 KB filled region at `0x8c100000`, eight float arguments in fr4-fr11, the region
hashed afterwards for memory-writing functions.

| Outcome | Count |
|---|---|
| Agree with Flycast | 30 (16 pure, 14 memory-writing) |
| Disagree | 0 |
| Pruned: oracle never reached the stop address | 5 (`ht_050454`, `ht_05040c`, `ht_083a5c`, `ht_03d444`, `ht_026e56`) |
| Pruned: runner faulted accessing outside RAM (pointers loaded from data the harness cannot seed) | 15 |

The 20 pruned cases ran on at most one side, so they say nothing about the emitter. FPU operations
covered by the 30: `fmov` (single), `fmul`, `fsub`, `fmac`, `fadd`, `fcmp/gt`, `fcmp/eq`, `fldi`,
`fsqrt`, `fneg`, `float`, `fdiv`, `ftrc`, `fsts`, `fipr`. Not covered by this slice: `ftrv`,
`fsca`, `fschg` pair moves (covered by the engine cases).

Files: `game/slice.cmake` and `game/oracle-cases.json` (addresses and inputs only, committed);
`game/extracted/golden/*.golden` (captured states, git-ignored). Regenerate the goldens:

```sh
python3 dreamcomp/engine/tools/oracle/differential.py --runner build/dreamcomp/engine/tests/sh4/dream_sh4_runner \
  --lib $DREAM_FLYCAST_CORE --cases build/hydrothunder-oracle-cases.json --keep build/ht_slice/dumps
python3 dreamcomp/engine/tools/oracle/write_golden.py --dumps build/ht_slice/dumps \
  --out game/extracted/golden --cases build/hydrothunder-oracle-cases.json
```

## Regression gate

```sh
python3 dreamcomp/engine/tools/regress/gate.py capture <label> --store .regress --scenarios attract \
  --exe build/game/hydro-thunder --config "$PWD/game/hydrothunder.toml" --disc <image>
python3 dreamcomp/engine/tools/regress/gate.py compare <before> <after> --store .regress --scenarios attract
```

`attract`: no input, `--rtc-seed 1 --no-audio --max-seconds 40`. Two captures of the current build:
2,407 frames each, write hash `31e876e503dc4e7a` both times, counters identical, `compare` exits 0.
The `gameplay` scenario's presses are Crazy Taxi's; a Hydro Thunder race scenario replaces it in M5.
