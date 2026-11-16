#!/usr/bin/env python3
"""Disc identity: the owner's image is the Hydro Thunder release this port was measured against.

Disc path: $HT_DISC, else "disc" in .dreamcomp/local.json (written by `dc.py setup`).
Exits 77 (CTest skip) when neither names an existing file.
"""

import glob
import hashlib
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "dreamcomp", "engine", "tools", "dcdisc"))
# dcdisc stops its libchdr search at the vendored engine's .gitmodules, so point it at our build.
if "DCDISC_LIBCHDR" not in os.environ:
    for lib in sorted(glob.glob(os.path.join(ROOT, "build*", "**", "libchdr.*"), recursive=True)):
        if lib.endswith((".dylib", ".so", ".dll")):
            os.environ["DCDISC_LIBCHDR"] = lib
            break

from dcdisc.image import open_image  # noqa: E402
from dcdisc.inspect import inspect_image  # noqa: E402
from dcdisc.iso9660 import Iso9660  # noqa: E402

PRODUCT = "T-9702N"
VERSION = "V1.020"
BOOT_SHA1 = "d8d54a28eac55cb4b63d9693951f9582a1b2505a"
SKIP = 77


def find_disc():
    disc = os.environ.get("HT_DISC")
    if not disc:
        try:
            with open(os.path.join(ROOT, ".dreamcomp", "local.json")) as f:
                disc = json.load(f).get("disc")
        except (OSError, ValueError):
            disc = None
    return disc if disc and os.path.isfile(disc) else None


def main():
    disc = find_disc()
    if not disc:
        print("SKIP: no disc image (set HT_DISC or run dc.py setup)")
        return SKIP
    with open_image(disc) as img:
        r = inspect_image(img, scan_all=False)
        boot = Iso9660(img).read_path(r.boot_file)
    ip = r.ipbin
    assert ip["product_number"].strip() == PRODUCT, ip["product_number"]
    assert ip["product_version"].strip() == VERSION, ip["product_version"]
    assert not int(ip["peripherals_raw"].strip() or "0", 16) & 1, "IP.BIN Windows CE flag is set"
    assert not r.wince, r.wince_reason
    assert r.boot_file.upper() == "1ST_READ.BIN", r.boot_file
    assert not r.boot_was_scrambled, "boot file stored scrambled; not a GD-ROM dump"
    sha1 = hashlib.sha1(boot).hexdigest()
    assert sha1 == BOOT_SHA1, sha1
    print(f"OK: {PRODUCT} {VERSION}, WinCE clear, {r.boot_file} SHA-1 {sha1}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
