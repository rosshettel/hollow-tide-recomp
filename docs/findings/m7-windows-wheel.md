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
  SDL3.dll                 # official SDL 3.4.20 win32-x64 runtime
  hydrothunder.toml
  frontend/                # launcher UI + fonts
  README.txt

dist/hydro-thunder-windows-x64.zip   # same contents; prefer unzip on Windows
```

MinGW runtimes (`libgcc` / `libstdc++` / `winpthread`) are statically linked —
no `lib*.dll` beside the exe (avoids Win11 Smart App Control `0xc0e90002` on
unsigned MinGW DLLs). No `vulkan-1.dll` — use the GPU vendor loader in System32.

Delete any older folder on the USB first, then copy `dist/windows/` or unzip the
zip on the Windows PC. Disc image is **not** included (rule 1).
Double-click `Play Hydro Thunder.bat` → pick disc in the launcher → Start Game.
Disc path persists in `%APPDATA%\dreamcomp\hydrothunder\settings.ini`.

If Windows says `SDL3.dll` is “not designed to run on Windows”, the copy is
corrupt or mixed with an old build — wipe the folder and re-copy/unzip. Needs
64-bit Windows 10/11.

Rebuild: `./tools/build_windows_mingw.sh`

## Keyboard defaults

Same Hydro Thunder layout as macOS (`src/keyboard.h` via `PortInfo::default_bindings`). Bindings
file uses SDL key names, so Windows sees the same defaults. Seeded once to
`%APPDATA%\dreamcomp\hydrothunder\bindings.txt`.

## Wheel / pedals / handbrake

Configure in dreamcomp’s **Controls** tab (launcher before Start, or Escape in-game).
F1 opens the HUD editor; it does not open bindings.

1. Controls → **Wheel / pedals** → Edit Profile (or footer toggle to **W**)
2. Click WHEEL / PEDALS / HANDBRAKE to assign each physical device
3. Bind by clicking a box then using the hardware:
   - Analog Left/Right ← wheel
   - R Trigger ← gas, L Trigger ← brake
   - A ← clutch (boost); A’s second box ← optional handbrake boost
4. Saved immediately to `%APPDATA%\dreamcomp\hydrothunder\bindings.txt`

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
