# M3: headless boot (macOS)

macOS arm64, headless launcher (`DREAM_RENDERER=OFF`), 2026-11-16. Windows, Android and Linux are
not built yet (M7).

## Config changes (one `--suggest-config` round)

From the M1 first run, each checked against the disassembly (`dream-translate disasm`) and the
discovered function list before pasting. Details and reasons are in `game/hydrothunder.toml`.

| Suggestion | Taken? | Why |
|---|---|---|
| `[functions] extra` 0x0C021020, 0x0C0944BC, 0x0C094D14, 0x0C0951A8, 0x0C1C1024 | yes | Each in a discovery gap right after a function's `rts`, delay slot and pool, opening with `mov.l r14,@-r15`: real entries, so seeding truncates nothing |
| relocation image 0x0C1CBB44 -> 0x8C000100, and -> 0x8C000400 | yes, size 0x34 not 0x38, `no_fold` | General-exception and TLB-miss stubs (VBR = 0x8C000000): save r0 into their own pool, read EXPEVT, jump to the common handler 0x8C1CBBAC |
| relocation image 0x0C1CBB78 -> 0x8C000600 | yes, size 0x34, `no_fold` | Interrupt stub, same shape, reads INTEVT |
| relocation image 0x0C1D4880 -> 0x8C00007C | no | Data (zeros then 0xFFFF fill); a coincidental byte match, never executed |

Second run's `--suggest-config` offered only the rejected data region. Result: 3,998 functions plus
three one-function vector units; still 0 unsupported instructions.

## Exit check (1,800 frames, `--rtc-seed 1`)

| Report line | Value |
|---|---|
| stop | frame limit, no fault |
| untranslated call targets | 0 (was 8,021 calls to 6 addresses) |
| unmapped memory accesses | 0 |
| PVR | 707 renders (706 done), 708 frame swaps |
| GD-ROM sectors read | 5,227 |
| Two runs, `--write-hash` | identical (1,800 frames; final line `1800 5979406948 cd17f078a3918253 157954971`) |

Over time (same seed):

| Frames | Renders | Sectors read |
|---|---|---|
| 300 | 96 | 1,070 |
| 600 | 223 | 1,665 |
| 900 | 373 | 1,665 |
| 1,200 | 407 | 5,227 |
| 1,800 | 707 | 5,227 |
| 3,600 | 1,607 | 5,227 |

From frame 1,200 on the game renders exactly 150 frames per 300 (30 fps), reads nothing more from
the disc through frame 3,600 and polls the pad every frame. That is the shape of a title or menu
loop waiting for input. Inference: nothing has been looked at on screen; M4 decides what it is.

Also seen: 0 traps, 0 Sort-DMA starts, 2 CD-DA syscalls (boot does not wait on CD audio), ARM7
running `AUDIO64.DRV` but the mixer has 0 key-ons (M6).

## Regression gate change, understood

`gate.py compare` between the M2 attract capture (`31e876e5...`) and now (`473c728e...`): counters
identical except `untranslated call targets` (13,100 -> 0); write hashes equal through frame 1,346.

- Hiding the eight newly translated functions (`DREAM_INTERP_FUNCS`) reproduces the M2 hash exactly.
- With `--mask-segment` both builds give identical hashes over all 2,406 frames.
- The differing stores are return addresses pushed inside `fn_0c021020`, which the game calls
  through its P1 alias: the interpreter pushes `0x8c0210xx`, translated code `0x0c0210xx`. Values are
  otherwise equal. This is the segment case the engine documents for `--mask-segment`
  (`dreamcomp/engine/docs/differential-harness.md`); not a translation fault.
