# M4: picture on macOS (MoltenVK), compared against Flycast

Date 2026-11-16. macOS 27.2, Apple M4 Pro, 75 Hz external display. Windows, Android and Linux are
not built yet (M7); nothing here uses a platform API outside SDL3 and Vulkan.

## Stack

Homebrew: molten-vk 1.4.2, vulkan-loader 1.4.363.0, vulkan-headers 1.4.363.0, shaderc 2026.4
(`glslc`), SDL3 3.4.20. `cmake -S . -B build -DDREAM_RENDERER=ON` finds them unaided
(`dream::render: Vulkan backend enabled`). The window presents through the Vulkan swapchain on
MoltenVK.

## Recomp screenshots

```sh
build/venv/bin/python dreamcomp/tools/dc.py shots . \
  --at 60,300,600,900,1200,1500,1800,2400,3000,3600,4800,6000 --out build/m4/ours
```

(`build/venv`: a local venv with Pillow; `dc.py` falls back to PPM without it.) Presented frames
60 to 6000: legal screen, Midway/Hydro Thunder title with blinking PRESS START, attract-mode demo
races (Team Hydro Racing overlay, canyon, waterfall and mountain courses), back to the title, next
demo. 7,620 guest frames, 0 untranslated targets, 0 unmapped accesses.

## Flycast reference screenshots

A local clone of `refs/flycast` (`3a5267a`, GPL-2.0) in ignored `build-flycast/src`, with
`tools/flycast-lua-screenshot.patch`: Lua turned on for macOS (upstream excludes Apple) and
`flycast.emulator.screenshot(path)`, which writes Flycast's own `getScreenshot` PNG on the UI
thread. Build:

```sh
git clone refs/flycast build-flycast/src && cd build-flycast/src && git checkout 3a5267a
git submodule update --init --recursive --depth 1 && git apply ../../tools/flycast-lua-screenshot.patch
cd .. && printf 'enable_language(OBJC)\nenable_language(OBJCXX)\n' > objc.cmake
cmake -S src -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DUSE_BREAKPAD=OFF -DUSE_HOST_LIBZIP=OFF -DUSE_LUA=ON -DCMAKE_PROJECT_flycast_INCLUDE=$PWD/objc.cmake \
  -DMOLTENVK_DYLIB=/opt/homebrew/opt/molten-vk/lib/libMoltenVK.dylib \
  -DZLIB_LIBRARY="$(xcrun --show-sdk-path)/usr/lib/libz.tbd"
cmake --build build -j 10
```

The extra flags work around Flycast's Xcode-generator assumptions under Ninja: SDL and Syphon
enable Objective-C only in their subdirectories, `ZLIB_LIBRARY` is preset to `-lz`, the default is
a universal binary while Homebrew's MoltenVK, Lua and libpng are arm64 only.

```sh
build/venv/bin/python tools/flycast_shots.py --at 60,300,600,900,1200,1500,1800,2400,3000,3600,4800,6000
```

Runs Flycast (HLE BIOS, no audio, private `HOME` under `build-flycast/`) with a generated Lua
script that captures at guest frame = presented frame + 24 (the recomp's lead at these frames),
stops it once every PNG exists, and writes `build/m4/flycast/compare.png` (recomp left, Flycast
right). Exits 77 without Flycast or a disc.

## Comparison

Legal screen and title: same layout, fonts, logo and colours. Attract mode: the two engines drift
apart in time once the demos start (different disc timing), so most rows show different moments;
where the same scene is on screen (frames 1500, 3600, 6000) the courses, sky, water, boats, Team
Hydro Racing overlay and PRESS START match. No missing geometry, texture or transparency fault
seen. A per-pixel number is not meaningful here: Flycast's frame is 640x479 and scaled
differently, so even the static title differs by ~20/255 on average from resampling alone.

The user judges the final picture from `build/m4/ours/sheet.png` and `build/m4/flycast/compare.png`
(both ignored: derived from the disc).

## Engine fixes this milestone (`dreamcomp/docs/engine-changes.md`)

| Fix | Found by |
|---|---|
| Function table `end` is the emitted extent, not discovery's | Past frame 1,800 two dev-interpreter resumes at 0x0c1cc4be / 0x0c1cc4c8: a branch fragment ending at an unrecovered `braf` claimed its host's switch cases. Now 0 untranslated targets; unit test |
| `fps = auto` blends frames only on 120 Hz+ displays | Windowed run at 0.8x real time on the 75 Hz display (two vsynced presents per frame). Now frame mean 16.70 ms, 39.9 s guest in 40.1 s host |
| `dc.py` takes only an executable as the launcher | `dc.py shots` ran `hydrothunder_port_version.cpp` on macOS |

## Exit check

| Check | Result |
|---|---|
| `ctest --test-dir build` | 9/9 pass |
| Regression gate `attract`, M3 vs M4 | identical (write hash `473c728e4f77ab04`) |
| Screenshots before vs after the emitter fix | 12/12 pixel-identical |
| Headless unthrottled | 8.5x real time |
| Windowed at the guest clock | 1.0x (mean frame 16.70 ms) |
