# M7: Windows portable build + steering wheel / pedals

## Exit criteria (plan)

- Windows plays a race on the keyboard and on a wheel with pedals, from a disc chosen at run time
- Linux builds (engine tests)
- Rule 2 shim progress; keyboard defaults unchanged; unlock_all / widescreen stay shared

## Portable Windows folder

Produced on this Mac via mingw-w64 cross-compile (`tools/build_windows_mingw.sh`):

```
dist/windows/
  hydro-thunder.exe
  SDL3.dll
  vulkan-1.dll          # fallback loader; prefer the GPU vendor's Vulkan
  hydrothunder.toml
  README.txt            # copy / run / wheel setup
```

Copy the whole folder to a USB stick. Disc image is **not** included (rule 1).

Primary dreamcomp Windows path remains clang-cl + VS 2022 Build Tools on a Windows host
(`dreamcomp/docs/PLATFORMS.md`). The mingw script is the Mac fall-back that still yields a
USB-ready layout. Rebuild:

```sh
./tools/build_windows_mingw.sh
```

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
