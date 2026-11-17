#!/usr/bin/env bash
# Cross-compile Hydro Thunder for Windows x86_64 from macOS/Linux with mingw-w64, then stage a
# portable folder under dist/windows/ that copies onto a USB stick.
#
# Primary Windows path (dreamcomp): clang-cl + VS 2022 Build Tools on a Windows host
# (see docs/findings/m7-windows-wheel.md). This script is the Mac fall-back.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
deps="${HT_DEPS:-$root/.deps}"
build="${HT_BUILD_WIN:-$root/build-windows}"
dist="${HT_DIST_WIN:-$root/dist/windows}"
jobs="${HT_JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || nproc)}"

need() { command -v "$1" >/dev/null || { echo "missing $1" >&2; exit 1; }; }
need x86_64-w64-mingw32-g++
need cmake
need ninja
need glslc
need curl
need tar

mkdir -p "$deps"
sdl_root="$deps/SDL3-3.4.20/x86_64-w64-mingw32"
if [[ ! -f "$sdl_root/lib/cmake/SDL3/SDL3Config.cmake" ]]; then
  echo "fetching SDL3 mingw devel…"
  curl -fsSL -o "$deps/SDL3-devel-mingw.tar.gz" \
    "https://github.com/libsdl-org/SDL/releases/download/release-3.4.20/SDL3-devel-3.4.20-mingw.tar.gz"
  tar -C "$deps" -xf "$deps/SDL3-devel-mingw.tar.gz"
fi
vk_root="$deps/mingw-vulkan/mingw64"
if [[ ! -f "$vk_root/lib/libvulkan-1.dll.a" ]]; then
  echo "fetching mingw Vulkan headers + loader…"
  mkdir -p "$deps/mingw-vulkan"
  curl -fsSL -o "$deps/vk-loader.pkg.tar.zst" \
    "https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-vulkan-loader-1~1.4.363.0-1-any.pkg.tar.zst"
  curl -fsSL -o "$deps/vk-headers.pkg.tar.zst" \
    "https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-vulkan-headers-1~1.4.363.0-1-any.pkg.tar.zst"
  tar -C "$deps/mingw-vulkan" -xf "$deps/vk-loader.pkg.tar.zst"
  tar -C "$deps/mingw-vulkan" -xf "$deps/vk-headers.pkg.tar.zst"
fi

host_translate="$root/build/dreamcomp/engine/translator/dream-translate"
if [[ ! -x "$host_translate" ]]; then
  echo "building host dream-translate (used during the cross build)…"
  cmake -S "$root" -B "$root/build" -G Ninja -DCMAKE_BUILD_TYPE=Release >/dev/null
  cmake --build "$root/build" --target dream-translate --parallel "$jobs"
fi

# Host glslc builds the SPIR-V headers; the cross link uses Windows Vulkan + SDL3.
# Host dream-translate emits the C++; mingw only compiles/links it.
cmake -S "$root" -B "$build" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$root/tools/cmake/mingw-w64-x86_64.cmake" \
  -DCMAKE_PREFIX_PATH="$sdl_root;$vk_root" \
  -DVulkan_INCLUDE_DIR="$vk_root/include" \
  -DVulkan_LIBRARY="$vk_root/lib/libvulkan-1.dll.a" \
  -DDREAM_TRANSLATE_EXECUTABLE="$host_translate" \
  -DDREAM_BUILD_TESTS=OFF \
  -DDREAMCOMP_FRONTEND=OFF \
  -DDREAM_WERROR=OFF
cmake --build "$build" --target hydrothunder_boot --parallel "$jobs"

exe="$build/game/hydro-thunder.exe"
[[ -f "$exe" ]] || { echo "no exe at $exe" >&2; exit 1; }

rm -rf "$dist"
mkdir -p "$dist"
cp "$exe" "$dist/hydro-thunder.exe"
cp "$sdl_root/bin/SDL3.dll" "$dist/"
cp "$vk_root/bin/vulkan-1.dll" "$dist/"
# Config the portable run uses; disc path is chosen at run time (--disc or the launcher).
cp "$root/game/hydrothunder.toml" "$dist/"
if [[ -d "$build/game/frontend" ]]; then
  cp -R "$build/game/frontend" "$dist/frontend"
fi

cat > "$dist/README.txt" <<'EOF'
Hydro Thunder (Dreamcast) — portable Windows build
==================================================

What to copy
------------
Copy this entire folder to a USB stick (or anywhere on the Windows PC).
You also need your own Hydro Thunder disc image (.chd / .gdi / .cue).
Do not redistribute the disc image.

Requirements on the Windows PC
------------------------------
- 64-bit Windows 10/11
- A GPU with a working Vulkan driver (almost every recent NVIDIA / AMD / Intel
  driver installs one). If the game says it cannot create a Vulkan device,
  update the GPU driver from the vendor.

How to run
----------
1. Open a Command Prompt in this folder.
2. Run (replace the path with your disc image):

     hydro-thunder.exe --config hydrothunder.toml --disc "D:\Games\Hydro Thunder.chd" --window

   Or double-click hydro-thunder.exe and pick the disc in the launcher when one
   is present.

Keyboard defaults (same as macOS)
---------------------------------
- Arrows: steer (also move through menus)
- Up / W: throttle    Down / Q: brake
- Space / Z: boost    C / S: camera
- Enter: Start        F1: controller bindings    Esc: quit / back

Steering wheel + pedals
-----------------------
1. Plug in the wheel, pedal box, and optional handbrake.
2. Press F1 to open CONTROLLER.
3. DEVICE -> JOYSTICK.
4. Set WHEEL / PEDALS / HANDBRAKE to the matching devices (left/right).
5. Bind:
     STICK LEFT / RIGHT  <- turn the wheel each way
     R TRIGGER           <- press gas (accelerate)
     L TRIGGER           <- press brake
     A                   <- press clutch (boost)
     A .2 (left/right on the A row, then Return) <- handbrake for boost, optional
6. CLOSE — bindings save under %APPDATA%\dreamcomp\hydrothunder\bindings.txt
   so you only set this up once on that PC.

Optional settings
-----------------
  --set unlock_all=true     unlock every track and boat
  --widescreen              wide races (menus stay 4:3)
  --no-launcher             skip the launcher window

Vulkan
------
This build talks to the system Vulkan loader (vulkan-1.dll beside the exe is a
fallback). Prefer the GPU vendor's driver; delete our vulkan-1.dll if it
conflicts with a newer system loader.
EOF

echo "staged: $dist"
ls -la "$dist"
