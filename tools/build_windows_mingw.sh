#!/usr/bin/env bash
# Cross-compile Hollow Tide for Windows x86_64 from macOS/Linux with mingw-w64, then stage a
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
# Redistributable Windows x64 SDL3.dll (MSVC-built). Prefer this over the mingw
# devel copy next to the exe — same C ABI, fewer "not designed to run on Windows"
# reports from bad USB copies / wrong-arch mixups.
sdl_runtime_zip="$deps/SDL3-3.4.20-win32-x64.zip"
sdl_runtime_dll="$deps/SDL3-win32-x64/SDL3.dll"
if [[ ! -f "$sdl_runtime_dll" ]]; then
  echo "fetching SDL3 win32-x64 runtime…"
  curl -fsSL -o "$sdl_runtime_zip" \
    "https://github.com/libsdl-org/SDL/releases/download/release-3.4.20/SDL3-3.4.20-win32-x64.zip"
  rm -rf "$deps/SDL3-win32-x64"
  mkdir -p "$deps/SDL3-win32-x64"
  tar -C "$deps/SDL3-win32-x64" -xf "$sdl_runtime_zip" 2>/dev/null ||
    unzip -o -d "$deps/SDL3-win32-x64" "$sdl_runtime_zip"
fi
[[ -f "$sdl_runtime_dll" ]] || { echo "missing $sdl_runtime_dll" >&2; exit 1; }
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
# Frontend ON so a double-click opens the disc picker (no command line).
# Force STANDARD_LIBRARIES: Windows-GNU overwrites toolchain *_INIT, and
# -Bstatic must trail the link line so the driver's -lpthread stays static.
mingw_winlibs="-lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32"
mingw_stdlibs="$mingw_winlibs -Wl,-Bstatic -lwinpthread -lpthread"
cmake -S "$root" -B "$build" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$root/tools/cmake/mingw-w64-x86_64.cmake" \
  -DCMAKE_PREFIX_PATH="$sdl_root;$vk_root" \
  -DVulkan_INCLUDE_DIR="$vk_root/include" \
  -DVulkan_LIBRARY="$vk_root/lib/libvulkan-1.dll.a" \
  -DCMAKE_C_STANDARD_LIBRARIES="$mingw_stdlibs" \
  -DCMAKE_CXX_STANDARD_LIBRARIES="$mingw_stdlibs" \
  -DDREAM_TRANSLATE_EXECUTABLE="$host_translate" \
  -DDREAM_BUILD_TESTS=OFF \
  -DDREAMCOMP_FRONTEND=ON \
  -DDREAM_WERROR=OFF
cmake --build "$build" --target hydrothunder_boot --parallel "$jobs"

exe="$build/game/hollow-tide.exe"
[[ -f "$exe" ]] || { echo "no exe at $exe" >&2; exit 1; }
[[ -d "$build/game/frontend" ]] || { echo "frontend assets missing beside the exe" >&2; exit 1; }

rm -rf "$dist"
mkdir -p "$dist"
cp "$exe" "$dist/hollow-tide.exe"
cp "$sdl_runtime_dll" "$dist/SDL3.dll"
# MinGW runtimes (libgcc/libstdc++/winpthread) are statically linked — see toolchain.
# Do NOT ship vulkan-1.dll next to the exe: Windows loads the app-dir copy first and that
# would shadow the GPU vendor's loader (what a 4070 Ti already has in System32).
cp "$root/game/hydrothunder.toml" "$dist/"
cp -R "$build/game/frontend" "$dist/frontend"
# Optional: unlock_all / widescreen toggles live in the launcher settings after first run.

# Refuse to stage a non-Windows / wrong-arch SDL3.dll (the "not designed to run on
# Windows" dialog). file(1) must report PE32+ x86-64.
if command -v file >/dev/null; then
  sdl_file="$(file -b "$dist/SDL3.dll")"
  echo "SDL3.dll: $sdl_file"
  [[ "$sdl_file" == *"PE32+"* && "$sdl_file" == *"x86-64"* ]] || {
    echo "refusing to stage non-x64 Windows SDL3.dll: $sdl_file" >&2
    exit 1
  }
fi
# Exe must not still depend on shared MinGW runtime DLLs (SAC blocks unsigned ones).
if command -v x86_64-w64-mingw32-objdump >/dev/null; then
  deps_dump="$(x86_64-w64-mingw32-objdump -p "$dist/hollow-tide.exe")"
  for dll in libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll; do
    if grep -q "DLL Name: $dll" <<<"$deps_dump"; then
      echo "exe still imports $dll — static MinGW link failed" >&2
      exit 1
    fi
  done
fi
[[ ! -e "$dist/libwinpthread-1.dll" ]] || {
  echo "refusing to ship libwinpthread-1.dll beside the exe" >&2
  exit 1
}

# Double-click this. cwd is forced to the bat's folder so hydrothunder.toml and frontend/ resolve.
cat > "$dist/Play Hollow Tide.bat" <<'EOF'
@echo off
cd /d "%~dp0"
start "" "hollow-tide.exe"
EOF

cat > "$dist/README.txt" <<'EOF'
Hollow Tide — portable Windows build
==================================================

What to copy
------------
Delete any older Hollow Tide folder on the USB first, then copy this entire
folder (or unzip hollow-tide-windows-x64.zip on the Windows PC).
You need: hollow-tide.exe, SDL3.dll, frontend/, hydrothunder.toml, and the .bat.
(No lib*.dll — MinGW runtimes are linked into the exe.)
You also need your own original disc image (.chd / .gdi / .cue).
Do not redistribute the disc image.

If Windows says a DLL is "not designed to run on Windows" with error 0xc0e90002,
that is often Windows 11 Smart App Control blocking an unsigned DLL — this build
avoids shipping those. Needs 64-bit Windows 10/11.

How to run (no command line)
----------------------------
Double-click "Play Hollow Tide.bat" (or hollow-tide.exe).

First launch opens a launcher window:
  1. Click the disc / browse control and pick your .chd (or .gdi / .cue)
  2. Click Start Game

The disc path is saved under %APPDATA%\dreamcomp\hydrothunder\settings.ini,
so the next double-click can start without picking it again.

Requirements
------------
- 64-bit Windows 10/11
- Vulkan from your GPU driver (see below)

Vulkan — do I have it?
----------------------
An NVIDIA GeForce RTX 4070 Ti with stock Game Ready / Studio drivers already
includes Vulkan. You do not need the Vulkan SDK.

Quick checks on Windows:
  1. File Explorer -> C:\Windows\System32\vulkan-1.dll  (should exist)
  2. Or Win+R -> dxdiag -> Display tab: your 4070 Ti is listed
  3. Optional: install "vulkaninfo" from the LunarG Vulkan SDK Runtime and run
     `vulkaninfo --summary` — look for a GeForce RTX 4070 Ti device

This folder deliberately does NOT include vulkan-1.dll, so Windows uses the
loader from your NVIDIA driver. If the game fails to open a Vulkan device,
update the GeForce driver from nvidia.com and try again.

Keyboard defaults (same as macOS)
---------------------------------
- Arrows: steer (also move through menus)
- Up / W: throttle    Down / Q: brake
- Space / Z: boost    C / S: camera
- Enter: Start        F1: controller bindings    Esc: quit / back

Steering wheel + pedals
-----------------------
Use the dreamcomp Controls tab (not F1 — F1 is the HUD editor).

1. Plug in the wheel, pedal box, and optional handbrake.
2. Before Start Game: Controls -> Wheel / pedals -> Edit Profile
   (or in-game: Escape -> Controls -> same / footer toggle to W).
3. Click WHEEL / PEDALS / HANDBRAKE to assign each device.
4. Click a binding box, then use the hardware:
     Analog Left / Right  <- wheel
     R Trigger            <- gas
     L Trigger            <- brake
     A                    <- clutch (boost)
     A's second box       <- optional handbrake boost
5. Saved immediately under %APPDATA%\dreamcomp\hydrothunder\bindings.txt

Optional (launcher Settings, or later)
--------------------------------------
  unlock_all     unlock every track and boat
  widescreen     wide races (menus stay 4:3)
EOF

# Zip without Mac resource forks — more reliable than Finder→USB for PE files.
zip_path="$root/dist/hollow-tide-windows-x64.zip"
mkdir -p "$root/dist"
rm -f "$zip_path"
(
  cd "$dist"
  # -X strips extra attrs; -r recursive. Run from inside so paths are flat in the zip.
  zip -X -r "$zip_path" .
)

echo "staged: $dist"
echo "zip:    $zip_path"
ls -la "$dist"
ls -la "$zip_path"
