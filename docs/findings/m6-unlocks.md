# M6: unlock all tracks and boats

Date 2026-11-16. Port setting only; no infinite boost or other cheats.

## Setting

| Key | Default | Effect |
|---|---|---|
| `unlock_all` | off (`false`) | All tracks (Easy/Medium/Hard/Bonus) and all boats including bonus/hidden |

Enable for one run: `--set unlock_all=true`. Persist: add `unlock_all = true` under the
player's `settings.ini`, or pass `--save-settings` with `--set`.

Shared path: `PortInfo::on_vblank` in `src/unlocks.cpp` (not Mac-only). Off does no guest writes.

## How it is stored

USA V1.020 CodeBreaker "Enable All Tracks And Boats" (Sage): six 16-bit writes of `1` at

`0x8C26B8D0`, `+4`, `+8`, `+C`, `+10`, `+14`.

The boot file references `0x8C26B8D0` at four sites (`hydrothunder.unlocks` checks that). Each
vblank re-asserts the six words when the setting is on.

## Checks

| Check | Result |
|---|---|
| Track select screenshot, unlocks on | Medium/Hard/Bonus named (Greek Isles … Hydro Speedway) |
| Boat select screenshot, unlocks on | Bonus row: Tiny Titanic, Armed Response, Blowfish, fishing boat |
| Gate `aspect=original`, unlocks off | attract `473c728e4f77ab04`, gameplay `216661ec86e35b64` (match M5) |
| `ctest -R hydrothunder\.(identity\|keyboard\|unlocks)` | pass |

## Play

```zsh
DISC="$(python3 -c 'import json; print(json.load(open(".dreamcomp/local.json"))["disc"])')"
./build/game/hydro-thunder --config game/hydrothunder.toml --disc "$DISC" \
  --window --no-launcher --set unlock_all=true
```
