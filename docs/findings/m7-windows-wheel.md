# M7: Windows portable build + steering wheel / pedals

## Exit criteria (plan)

- Windows plays a race on the keyboard and on a wheel with pedals, from a disc chosen at run time
- Linux builds (engine tests)
- Rule 2 shim progress; keyboard defaults unchanged; unlock_all / widescreen stay shared

## Portable Windows folder

Produced on this Mac via mingw-w64 cross-compile (`tools/build_windows_mingw.sh`):

```
dist/windows/
  Play Hydro Thunder.bat   # double-click this
  hydro-thunder.exe        # launcher built in (disc picker)
  SDL3.dll
  hydrothunder.toml
  frontend/                # launcher UI + fonts
  README.txt
```

No `vulkan-1.dll` in the folder — Windows uses the GPU vendor loader from System32
(important for an RTX 4070 Ti with stock NVIDIA drivers).

Copy the whole folder to a USB stick. Disc image is **not** included (rule 1).
Double-click `Play Hydro Thunder.bat` → pick disc in the launcher → Start Game.
Disc path persists in `%APPDATA%\dreamcomp\hydrothunder\settings.ini`.

Rebuild: `./tools/build_windows_mingw.sh`

## Keyboard defaults

Same Hydro Thunder layout as macOS (`src/keyboard.h` via `PortInfo::default_bindings`). Bindings
file uses SDL key names, so Windows sees the same defaults. Seeded once to
`%APPDATA%\dreamcomp\hydrothunder\bindings.txt`.

## Wheel / pedals / handbrake

Raw SDL joysticks (not the gamepad API):

1. F1 → CONTROLLER → DEVICE → JOYSTICK
2. Assign WHEEL / PEDALS / HANDBRAKE (left/right cycles plugged-in sticks by SDL GUID)
3. Bind axes/buttons by pressing them:
   - STICK LEFT/RIGHT ← wheel
   - R TRIGGER ← gas, L TRIGGER ← brake (full-range pedal axes)
   - A ← clutch (boost); left/right on A selects slot `.2` for an optional handbrake boost
4. CLOSE — written to the bindings file (roles + `jaxis`/`jbutton` lines)

`unlock_all` and widescreen remain port hooks in `src/` (not Mac-only).

## Linux gate

```sh
./tools/build_linux_docker.sh
```

Headless Ubuntu 24.04 container: `runtime.unit` + `render.unit` green; `hydro-thunder` links.
Windowed Linux still needs distro SDL3 + Vulkan + glslc (24.04 default archive has no
`libsdl3-dev` yet).

## Rule 2

- New `dreamcomp/engine/platform/` (backtrace; boot no longer includes `<execinfo.h>`)
- `DREAM_DXGI` off by default (Vulkan present path only)
- `DREAM_TRANSLATE_EXECUTABLE` for cross builds
