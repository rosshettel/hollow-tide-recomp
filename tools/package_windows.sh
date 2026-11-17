#!/usr/bin/env bash
# Stage a portable Windows folder (same layout as dist/windows USB pack) and zip it.
# Works on a Windows host after a native clang-cl build, or on any host given the build outputs.
#
# Usage: package_windows.sh <build-dir> [shortsha]
# Env: HT_DIST_WIN (default dist/windows), HT_ZIP_PATH (default dist/hydro-thunder-windows-<sha>.zip)
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
build="${1:?build dir}"
shortsha="${2:-unknown}"
dist="${HT_DIST_WIN:-$root/dist/windows}"
zip_path="${HT_ZIP_PATH:-$root/dist/hydro-thunder-windows-${shortsha}.zip}"

exe="$build/game/hydro-thunder.exe"
[[ -f "$exe" ]] || { echo "no exe at $exe" >&2; exit 1; }
[[ -d "$build/game/frontend" ]] || { echo "frontend assets missing beside the exe" >&2; exit 1; }

rm -rf "$dist"
mkdir -p "$dist"
cp "$exe" "$dist/hydro-thunder.exe"
# Prefer the official win32-x64 runtime (USB pack / CI). Fall back to clang-cl POST_BUILD copy.
if [[ -n "${HT_SDL3_DLL:-}" && -f "$HT_SDL3_DLL" ]]; then
  cp "$HT_SDL3_DLL" "$dist/SDL3.dll"
elif [[ -f "$build/game/SDL3.dll" ]]; then
  cp "$build/game/SDL3.dll" "$dist/SDL3.dll"
else
  echo "SDL3.dll not found next to exe; set HT_SDL3_DLL" >&2
  exit 1
fi
cp "$root/game/hydrothunder.toml" "$dist/"
[[ -f "$root/game/hud_overrides.ini" ]] && cp "$root/game/hud_overrides.ini" "$dist/"
cp -R "$build/game/frontend" "$dist/frontend"
cp "$root/LICENSE" "$dist/LICENSE.txt"
[[ -f "$root/dreamcomp/THIRD_PARTY_NOTICES.md" ]] && cp "$root/dreamcomp/THIRD_PARTY_NOTICES.md" "$dist/"

if command -v file >/dev/null; then
  sdl_file="$(file -b "$dist/SDL3.dll")"
  echo "SDL3.dll: $sdl_file"
  [[ "$sdl_file" == *"PE32+"* && "$sdl_file" == *"x86-64"* ]] || {
    echo "refusing to stage non-x64 Windows SDL3.dll: $sdl_file" >&2
    exit 1
  }
fi

cat > "$dist/Play Hydro Thunder.bat" <<'EOF'
@echo off
cd /d "%~dp0"
start "" "hydro-thunder.exe"
EOF

cat > "$dist/README.txt" <<'EOF'
Hydro Thunder (Dreamcast) — portable Windows build
==================================================

What to copy
------------
Delete any older Hydro Thunder folder first, then unzip this archive on the
Windows PC. You need: hydro-thunder.exe, SDL3.dll, frontend/, hydrothunder.toml,
and the .bat. You also need your own Hydro Thunder disc image (.chd / .gdi / .cue).
Do not redistribute the disc image.

How to run (no command line)
----------------------------
Double-click "Play Hydro Thunder.bat" (or hydro-thunder.exe).

First launch opens a launcher window:
  1. Click the disc / browse control and pick your .chd (or .gdi / .cue)
  2. Click Start Game

The disc path is saved under %APPDATA%\dreamcomp\hydrothunder\settings.ini.

Requirements
------------
- 64-bit Windows 10/11
- Vulkan from your GPU driver (this folder does not ship vulkan-1.dll)

Keyboard defaults
-----------------
- Arrows: steer (also move through menus)
- Up / W: throttle    Down / Q: brake
- Space / Z: boost    C / S: camera
- Enter: Start        F1: controller bindings    Esc: quit / back

Steering wheel + pedals: dreamcomp Controls tab (not F1).
EOF

# Refuse to ship game data.
if [[ -n "$(find "$dist" \( -iname '1ST_READ.BIN' -o -iname '*.chd' -o -iname '*.gdi' \) -print -quit)" ]]; then
  echo "refusing to package disc / boot files" >&2
  exit 1
fi

mkdir -p "$(dirname "$zip_path")"
rm -f "$zip_path"
(
  cd "$dist"
  if command -v zip >/dev/null; then
    zip -X -r "$zip_path" .
  else
    # Windows runner without zip: PowerShell Compress-Archive.
    zip_win="$(cygpath -w "$zip_path" 2>/dev/null || echo "$zip_path")"
    powershell.exe -NoProfile -Command \
      "Compress-Archive -Path * -DestinationPath '$zip_win' -Force"
  fi
)

echo "staged: $dist"
echo "zip:    $zip_path"
ls -la "$zip_path"
