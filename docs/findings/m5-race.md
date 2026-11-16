# M5: a race on the keyboard

Date 2026-11-16, macOS 27.2, Apple M4 Pro. Windows, Android and Linux are not built yet (M7).

## Route into a race (guest frames, `--rtc-seed 1`)

Title, Start (724) to the main menu (GAME / CONFIG / OPTION), A (924) on GAME to Choose Track
(Thunder Park, Easy, first), A (1324) to Choose Boat, A (1874) to "Get ready", race starts about
2,570. `game/race.press` holds that plus the throttle from 2474 for 7,200 frames and a boost tap
every 30 frames. With no steering the boat reaches the final lap and the clock runs out ("TIME
EXPIRED", 8th of 16), then results and back to attract.

## Control mapping

Measured by holding each input for 300 frames mid-race against a run without it (mean pixel
difference over the next 250 frames), and confirmed by the game's own Configure Controller screen
(CONFIG): Brake, Accelerate, Steer, Camera, Boost, Pause.

| Pad input | In a race | Evidence |
|---|---|---|
| R trigger | accelerate | 16th to 12th while held |
| L trigger | brake | drops to 16th |
| A | boost (needs pickups; none at the start) | 0.0 difference at the start, 33-44 after a pickup |
| Y | camera | first-person view |
| B, X | nothing | 0.0 |
| stick or d-pad left/right | steer | identical frames for either |
| d-pad up/down | nothing | 0.0 |

## Keyboard defaults (`src/keyboard.h`)

| Key | Control | Also |
|---|---|---|
| Up arrow | accelerate (R trigger) and d-pad up for menus | W |
| Down arrow | brake (L trigger) and d-pad down | Q |
| Left / Right | steer (stick and d-pad) | |
| Space | boost (A), menu confirm | Z |
| C | camera (Y) | S |
| X | B (menu back) | |
| Return | Start (pause) | |

Stick up/down are unbound on the keyboard (they were the arrows). A port supplies the layout as
`PortInfo::default_bindings`; the frontend writes it once to
`~/Library/Application Support/dreamcomp/hydrothunder/bindings.txt`, which the Controls screen
then edits. Delete that file to get the defaults back. `hydrothunder.keyboard` (ctest) parses the
layout with the engine's parser and checks every binding above. Keys bound to the triggers ramp
over ~150 ms (engine default); steering is still full lock: a ramp waits for play.

## VMU

Every boot reads 15 blocks (root, FAT, 13 directory blocks) from the card in port A1 and finds no
save. Nothing tried here writes: a full race (8th, time expired), the Music and SFX screen with a
change, the controller screen. The likely trigger is an unlock (top three on Easy opens Medium) or
a record, which scripted play without steering does not reach. To check after playing: the
`memory card:` line of the run's report, or the size and date of `vmu_a1.bin` beside the settings.

## Translation

One seed: `0x0C01BBA0`, a function reached through a pointer table (`jsr @r12` at `0x0C061A4C`,
as `0x8C01BBA0`), right after the previous function's `rts` and pool, standard prologue. No wrong
translation seen.

## Exit check

| Check | Result |
|---|---|
| Race scenario (`gate.py capture --scenarios gameplay --gameplay-press "$(cat game/race.press)" --max-seconds 180`) | 10,788 frames, 0 untranslated targets, 0 unmapped accesses; two captures identical (`216661ec86e35b64`) |
| Attract gate, M4 vs M5 | identical (`473c728e4f77ab04`) |
| `ctest --test-dir build` | 10/10 |
| You play a race on the keyboard | pending |
