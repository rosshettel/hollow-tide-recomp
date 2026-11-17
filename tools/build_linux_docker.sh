#!/usr/bin/env bash
# M7 Linux build gate: compile the engine tests (and hydrothunder headless if the extracted
# binary is present) inside an Ubuntu container from this Mac. Rule 2 lists Linux x86_64.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
img="${HT_LINUX_IMAGE:-ubuntu:24.04}"
need() { command -v "$1" >/dev/null || { echo "missing $1" >&2; exit 1; }; }
need docker

docker run --rm -v "$root:/src:rw" -w /src "$img" bash -lc '
set -euo pipefail
export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
# Headless gate: no SDL3/Vulkan packages required (Ubuntu 24.04 still lacks libsdl3-dev in
# the default archive). Windowed Linux builds need those packages separately.
apt-get install -y -qq cmake ninja-build g++ python3 >/dev/null
cmake -S /src -B /src/build-linux -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DDREAM_BUILD_TESTS=ON \
  -DDREAM_RENDERER=OFF \
  -DDREAMCOMP_FRONTEND=OFF \
  -DDREAM_WERROR=OFF
cmake --build /src/build-linux --target dream_runtime_tests dream_render_tests --parallel
if [[ -f /src/game/extracted/fs/1ST_READ.BIN ]]; then
  cmake --build /src/build-linux --target dream-translate hydrothunder_boot --parallel
fi
ctest --test-dir /src/build-linux -R "runtime.unit|render.unit" --output-on-failure
echo "linux build gate: ok"
'
