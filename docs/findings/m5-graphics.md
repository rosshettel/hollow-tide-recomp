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
  `PortInfo::widescreen_now`: scene word 0x8C25C81C == 5 (2 title, 3 menus, 4 loading, 5 from the
  first frame after loading: intro, countdown, race, end sequence) and attract flag 0x8C1DD0B8 == 0
  (1 on the title and in demos; set the moment a game over returns to the title, where the scene
  is still 5 for 36 frames). Earlier triggers, each found wanting in play: 0x8C1DD124 flips every
  ~15 frames mid-race (flicker); the race clock's state 0x8C1DD1B4 == 3 misses the countdown and
  the end sequence; the mode word 0x8C1DD1C4 (5/6) lags the intro by ~50 frames and stays 4 into
  the intro of a second race.
- Two-race 12:5 run: render target 4:3 (menus), wide (race 1), 4:3 (title after game over), wide
  (race 2). Confirmed by you in play, intro through results.

## Checks

- 12:5 shots, race frame 3970 and Choose Track 1180 (`dc.py shots ... -- --aspect 12:5`).
- Windowed 12:5 race to time-out: `render target now` 1280x960, 2304x960, 1280x960, nothing else.
- Gate at `aspect = original`: gameplay 216661ec86e35b64 (M5), attract identical to M5 over its
  2,406 frames. Expanded: gameplay changes (the widened camera), attract unchanged.
- ctest 10/10.
