#!/usr/bin/env python3
"""Reference screenshots from Flycast at fixed frames, beside the recomp's own (M4).

Flycast is a local build of refs/flycast with tools/flycast-lua-screenshot.patch (adds
flycast.emulator.screenshot(path) to its Lua API); docs/findings/m4-picture.md has the commands.
It runs with a private HOME under build-flycast/ so the user's own Flycast settings are untouched.

  python3 tools/flycast_shots.py --at 60,300,600 [--ours build/m4/ours] [--out build/m4/flycast]

--at takes the recomp's presented frames (dc.py shots); Flycast is captured at the matching guest
frame (presented + --lead). Writes frame_NNNNN.png and compare.png (recomp left, Flycast right).
Exits 77 when Flycast or the disc is missing.
"""
import argparse
import json
import os
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FLYCAST = os.path.join(ROOT, "build-flycast", "build", "Flycast.app", "Contents", "MacOS", "Flycast")

LUA = """local shots = {%s}
local n = 0
flycast_callbacks = {
  vblank = function()
    n = n + 1
    if shots[n] then flycast.emulator.screenshot(shots[n]) end
  end
}
"""


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--at", required=True)
    ap.add_argument("--lead", type=int, default=24, help="guest frames ahead of presented ones")
    ap.add_argument("--flycast", default=FLYCAST)
    ap.add_argument("--disc")
    ap.add_argument("--out", default=os.path.join(ROOT, "build", "m4", "flycast"))
    ap.add_argument("--ours", default=os.path.join(ROOT, "build", "m4", "ours"))
    a = ap.parse_args()

    disc = a.disc
    local = os.path.join(ROOT, ".dreamcomp", "local.json")
    if not disc and os.path.exists(local):
        disc = json.load(open(local)).get("disc")
    if not os.path.isfile(a.flycast) or not disc or not os.path.isfile(disc):
        print(f"skip: need {a.flycast} and a disc (--disc or .dreamcomp/local.json)")
        return 77

    frames = sorted({int(x) for x in a.at.split(",")})
    out = os.path.abspath(a.out)
    os.makedirs(out, exist_ok=True)
    home = os.path.join(ROOT, "build-flycast", "home")
    cfg = os.path.join(home, "Library", "Application Support", "Flycast")
    os.makedirs(cfg, exist_ok=True)
    shots = {f + a.lead: os.path.join(out, f"frame_{f:05d}.png") for f in frames}
    for p in shots.values():
        if os.path.exists(p):
            os.remove(p)
    table = ", ".join(f"[{n}] = {json.dumps(p)}" for n, p in shots.items())
    with open(os.path.join(cfg, "m4shots.lua"), "w") as f:
        f.write(LUA % table)

    opts = ["config:LuaFileName=m4shots.lua", "config:UseReios=yes", "audio:backend=null",
            "config:Dreamcast.Region=1", "config:Dreamcast.Broadcast=0", "config:rend.Resolution=480"]
    cmd = [a.flycast]
    for o in opts:
        cmd += ["-config", o]
    cmd.append(disc)
    # flycast.emulator.exit() from the vblank callback does not quit the app, so stop it once every
    # shot is on disk. Flycast runs at real speed; three times that means Lua never ran.
    proc = subprocess.Popen(cmd, env=dict(os.environ, HOME=home))
    deadline = time.time() + 60 + 3 * max(shots) / 60
    while proc.poll() is None and time.time() < deadline:
        if all(os.path.exists(p) for p in shots.values()):
            time.sleep(2)  # the last PNG is written on the UI thread
            break
        time.sleep(1)
    else:
        if proc.poll() is None:
            print("Flycast timed out: is Lua built in? (tools/flycast-lua-screenshot.patch)")
    proc.terminate()
    proc.wait()

    made = [(f, shots[f + a.lead]) for f in frames if os.path.exists(shots[f + a.lead])]
    print(f"{len(made)} of {len(frames)} Flycast screenshots in {out}")
    from PIL import Image, ImageDraw
    w, h = 320, 240
    sheet = Image.new("RGB", (2 * w, h * len(made)))
    for row, (f, p) in enumerate(made):
        ours = os.path.join(a.ours, f"frame_{f:05d}.png")
        for col, src in enumerate([ours, p]):
            if os.path.exists(src):
                cell = Image.open(src).convert("RGB").resize((w, h))
                ImageDraw.Draw(cell).text((4, 4), f"{'recomp' if col == 0 else 'flycast'} f{f}",
                                          fill=(255, 255, 0))
                sheet.paste(cell, (col * w, row * h))
    sheet.save(os.path.join(out, "compare.png"))
    return 0 if len(made) == len(frames) else 1


if __name__ == "__main__":
    sys.exit(main())
