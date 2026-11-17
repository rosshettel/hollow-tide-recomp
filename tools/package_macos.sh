#!/usr/bin/env bash
# Stage a portable macOS arm64 folder and zip it (no disc / no extracted boot file).
#
# Usage: package_macos.sh <build-dir> [shortsha]
# Env: HT_DIST_MAC, HT_ZIP_PATH, HT_SDL3_DYLIB, HT_MOLTENVK_DYLIB, HT_MOLTENVK_ICD
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
build="${1:?build dir}"
shortsha="${2:-unknown}"
dist="${HT_DIST_MAC:-$root/dist/macos}"
zip_path="${HT_ZIP_PATH:-$root/dist/hollow-tide-macos-arm64-${shortsha}.zip}"

bin="$build/game/hollow-tide"
[[ -x "$bin" ]] || { echo "no binary at $bin" >&2; exit 1; }
[[ -d "$build/game/frontend" ]] || { echo "frontend assets missing" >&2; exit 1; }

sdl="${HT_SDL3_DYLIB:-}"
if [[ -z "$sdl" ]]; then
  for c in /opt/homebrew/opt/sdl3/lib/libSDL3.0.dylib /usr/local/opt/sdl3/lib/libSDL3.0.dylib; do
    [[ -f "$c" ]] && sdl=$c && break
  done
fi
[[ -n "$sdl" && -f "$sdl" ]] || { echo "set HT_SDL3_DYLIB to libSDL3.0.dylib" >&2; exit 1; }

mvk="${HT_MOLTENVK_DYLIB:-}"
if [[ -z "$mvk" ]]; then
  for c in /opt/homebrew/opt/molten-vk/lib/libMoltenVK.dylib /usr/local/opt/molten-vk/lib/libMoltenVK.dylib; do
    [[ -f "$c" ]] && mvk=$c && break
  done
fi
[[ -n "$mvk" && -f "$mvk" ]] || { echo "set HT_MOLTENVK_DYLIB" >&2; exit 1; }

icd="${HT_MOLTENVK_ICD:-}"
if [[ -z "$icd" ]]; then
  for c in /opt/homebrew/etc/vulkan/icd.d/MoltenVK_icd.json \
           /usr/local/etc/vulkan/icd.d/MoltenVK_icd.json; do
    [[ -f "$c" ]] && icd=$c && break
  done
fi
[[ -n "$icd" && -f "$icd" ]] || { echo "set HT_MOLTENVK_ICD" >&2; exit 1; }

rm -rf "$dist"
mkdir -p "$dist/lib" "$dist/vulkan/icd.d"
cp "$bin" "$dist/hollow-tide"
cp "$sdl" "$dist/lib/libSDL3.0.dylib"
ln -sf libSDL3.0.dylib "$dist/lib/libSDL3.dylib"
cp "$mvk" "$dist/lib/libMoltenVK.dylib"
# ICD library_path is relative to the JSON file: ../../lib/libMoltenVK.dylib from vulkan/icd.d/
cat > "$dist/vulkan/icd.d/MoltenVK_icd.json" <<'EOF'
{
    "file_format_version" : "1.0.0",
    "ICD": {
        "library_path": "../../lib/libMoltenVK.dylib",
        "api_version" : "1.4.0",
        "is_portability_driver" : true
    }
}
EOF

cp "$root/game/hydrothunder.toml" "$dist/"
[[ -f "$root/game/hud_overrides.ini" ]] && cp "$root/game/hud_overrides.ini" "$dist/"
cp -R "$build/game/frontend" "$dist/frontend"
cp "$root/LICENSE" "$dist/LICENSE.txt"
[[ -f "$root/dreamcomp/THIRD_PARTY_NOTICES.md" ]] && cp "$root/dreamcomp/THIRD_PARTY_NOTICES.md" "$dist/"

# Point the binary at ./lib for SDL3; Vulkan ICD via the wrapper env.
install_name_tool -change /opt/homebrew/opt/sdl3/lib/libSDL3.0.dylib @loader_path/lib/libSDL3.0.dylib \
  "$dist/hollow-tide" 2>/dev/null || true
install_name_tool -change /usr/local/opt/sdl3/lib/libSDL3.0.dylib @loader_path/lib/libSDL3.0.dylib \
  "$dist/hollow-tide" 2>/dev/null || true
if otool -L "$dist/hollow-tide" | grep -q libvulkan; then
  vlib=""
  for c in /opt/homebrew/opt/vulkan-loader/lib/libvulkan.1.dylib \
           /usr/local/opt/vulkan-loader/lib/libvulkan.1.dylib; do
    [[ -f "$c" ]] && vlib=$c && break
  done
  if [[ -n "$vlib" ]]; then
    cp "$vlib" "$dist/lib/libvulkan.1.dylib"
    ln -sf libvulkan.1.dylib "$dist/lib/libvulkan.dylib"
    install_name_tool -change /opt/homebrew/opt/vulkan-loader/lib/libvulkan.1.dylib \
      @loader_path/lib/libvulkan.1.dylib "$dist/hollow-tide" 2>/dev/null || true
    install_name_tool -change /usr/local/opt/vulkan-loader/lib/libvulkan.1.dylib \
      @loader_path/lib/libvulkan.1.dylib "$dist/hollow-tide" 2>/dev/null || true
  fi
fi

cat > "$dist/Play Hollow Tide.command" <<'EOF'
#!/bin/bash
cd "$(dirname "$0")"
export VK_ICD_FILENAMES="$PWD/vulkan/icd.d/MoltenVK_icd.json"
export DYLD_LIBRARY_PATH="$PWD/lib:${DYLD_LIBRARY_PATH:-}"
exec ./hollow-tide
EOF
chmod +x "$dist/Play Hollow Tide.command" "$dist/hollow-tide"

cat > "$dist/README.txt" <<'EOF'
Hollow Tide — portable macOS (Apple Silicon) build
==================================================

A Dreamcast recomp port. You need your own original disc image
(.chd / .gdi / .cue). Do not redistribute the disc image.

Unzip, then double-click "Play Hollow Tide.command" (or run ./hollow-tide
from Terminal after setting VK_ICD_FILENAMES as in that script).

The first launch opens a launcher: pick the disc, then Start Game. Disc path
is saved under ~/Library/Application Support/dreamcomp/hydrothunder/.

This zip never includes game data.

Keyboard defaults match Windows (arrows / W Q / Space / Enter).
EOF

if [[ -n "$(find "$dist" \( -iname '1ST_READ.BIN' -o -iname '*.chd' -o -iname '*.gdi' \) -print -quit)" ]]; then
  echo "refusing to package disc / boot files" >&2
  exit 1
fi

mkdir -p "$(dirname "$zip_path")"
rm -f "$zip_path"
(
  cd "$dist"
  # -X strips Mac resource forks / xattrs that confuse other machines.
  zip -X -r "$zip_path" .
)

echo "staged: $dist"
echo "zip:    $zip_path"
ls -la "$zip_path"
