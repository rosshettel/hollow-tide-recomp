#!/usr/bin/env bash
# Restore game/extracted/fs/1ST_READ.BIN for CI packaging.
# Never commit or upload this file. Source: secret HT_BOOT_URL (see docs/ci.md).
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
expected_sha1="${HT_BOOT_SHA1:-d8d54a28eac55cb4b63d9693951f9582a1b2505a}"
dest="$root/game/extracted/fs/1ST_READ.BIN"

if [[ -z "${HT_BOOT_URL:-}" ]]; then
  echo "HT_BOOT_URL is unset. Add a repository secret that serves 1ST_READ.BIN (see docs/ci.md)." >&2
  exit 1
fi

mkdir -p "$(dirname "$dest")"
echo "downloading boot binary…"
# Optional Authorization (e.g. "Bearer ghp_…" or "token ghp_…") via HT_BOOT_AUTH.
# GitHub release-asset API URLs also need Accept: application/octet-stream.
curl_args=(-fsSL -o "$dest" -H "Accept: application/octet-stream")
if [[ -n "${HT_BOOT_AUTH:-}" ]]; then
  curl_args+=(-H "Authorization: ${HT_BOOT_AUTH}")
fi
curl "${curl_args[@]}" "$HT_BOOT_URL"

if command -v sha1sum >/dev/null; then
  got="$(sha1sum "$dest" | awk '{print $1}')"
elif command -v shasum >/dev/null; then
  got="$(shasum -a 1 "$dest" | awk '{print $1}')"
else
  echo "no sha1 tool" >&2
  exit 1
fi
if [[ "$got" != "$expected_sha1" ]]; then
  echo "boot SHA-1 mismatch: got $got want $expected_sha1" >&2
  rm -f "$dest"
  exit 1
fi
echo "boot ok ($got)"
