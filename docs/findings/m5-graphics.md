# Widescreen races (macOS)

Races widen to the window's shape (Expanded aspect, default; capped at 32:9); menus, loading
screens, results and attract demos stay 4:3. Higher resolution is the existing `--scale N` (or
the Resolution setting).

## How the game projects

- Camera block at 0x8C26B900 (BSS), filled by 0x0C07DC30 from a binary-angle field of view:
  focal 320/tan(h) at +0x90, tan(h) at +0x94, 240/focal at +0x98, frustum planes from +0xB8.
- The vertex loops (0x0C050284, 0x0C0502D4) and the point projector (0x0C0485EC) use one focal
  for both axes after the view transform: `x' = 320 + x*f/z`, `y' = 240 - y*f/z` (320 is a code
  literal, 240 at 0x8C1B05B8).
- The matrix stack's base 0x8C299CFC holds the camera matrix (3x4, x row = elements 0, 3, 6, 9);
  0x8C093E48 copies the camera into it and resets the stack pointer (0x8C29AA7C). Every object
  matrix is built on top of it.

## The port

- `set_camera@exit` on 0x8C093E48 scales the camera's x row by (4/3)/aspect: the scene is squeezed
  into 640x480 and the anamorphic render target unsqueezes it. Culling uses the same camera-space
  points, so it widens too. 4:3 is a no-op (the aspect arrives rounded; a 0.99999994 squeeze
  changed the gate hash).
- HUD: flat polygons at shared depths (race 1.0, 1.11, 1.52-1.56, 10; 2 sprites per race frame),
  3D up to 1/w 4.7 but never flat and shared: `hud.polygons = true`, default `overlay_z`. With the
  default `edges` layout the gauges sit at the screen edges.
- Menus: composed of many flat pieces; `edges` split HARD in two and left buttons stretched,
  `center` detached the track names from their buttons. They stay 4:3 via
  `PortInfo::widescreen_now`: 0x8C1DD1B4 == 3 from the race's first frame (guest 2530) to its end
  (9996), 0 in menus, 1 while loading, never set by attract demos. 0x8C1DD124 looked like a state
  in sparse dumps but flips every ~15 frames mid-race (the first try flickered).

## Checks

- 12:5 shots, race frame 3970 and Choose Track 1180 (`dc.py shots ... -- --aspect 12:5`).
- Windowed 12:5 race to time-out: `render target now` 1280x960, 2304x960, 1280x960, nothing else.
- Gate at `aspect = original`: gameplay 216661ec86e35b64 (M5), attract identical to M5 over its
  2,406 frames. Expanded: gameplay changes (the widened camera), attract unchanged.
- ctest 10/10.
