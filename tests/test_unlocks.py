#!/usr/bin/env python3
"""Unlock address self-check: the USA boot file must reference the CodeBreaker flag block."""
from __future__ import annotations

import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
PTR = bytes([0xD0, 0xB8, 0x26, 0x8C])  # 0x8C26B8D0 little-endian
UNLOCKS_CPP = ROOT / "src" / "unlocks.cpp"


def image_path() -> pathlib.Path | None:
    p = ROOT / "game" / "extracted" / "fs" / "1ST_READ.BIN"
    if p.is_file():
        return p
    local = ROOT / ".dreamcomp" / "local.json"
    if local.is_file():
        # setup always extracts beside the config; still require the boot file above
        pass
    return None


def main() -> int:
    text = UNLOCKS_CPP.read_text()
    if "0x8C26B8D0" not in text or 'unlock_all' not in text:
        print("unlocks.cpp missing base address or unlock_all key", file=sys.stderr)
        return 1
    img = image_path()
    if img is None:
        print("skip: no extracted 1ST_READ.BIN")
        return 77
    data = img.read_bytes()
    n = data.count(PTR)
    if n < 1:
        print(f"no pointer to 0x8C26B8D0 in {img}", file=sys.stderr)
        return 1
    print(f"ok: {n} refs to 0x8C26B8D0 in {img.name}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
