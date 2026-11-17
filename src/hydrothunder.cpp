// Hydro Thunder: what this port tells dreamcomp.
#include "dream/runtime/system.h"
#include "dreamcomp/hook.h"
#include "dreamcomp/port.h"
#include "keyboard.h"
#include "unlocks.h"

namespace {

// Widescreen (docs/findings/m5-graphics.md): every 3D draw projects camera-space x and y with one
// focal length (x' = 320 + x*f/z, y' = 240 - y*f/z), and every object matrix on the stack is the
// camera matrix times the model's. Scaling the camera matrix's x row squeezes the whole scene
// horizontally into the 640x480 frame; the engine's wider render target undoes the squeeze. The
// game's frustum planes test camera-space points from the same matrices, so culling widens too.
constexpr std::uint32_t kCameraMatrix = 0x8C299CFC;  // 3x4, x' = m0*x + m3*y + m6*z + m9
float g_squeeze = 1.0f;                              // (4/3) / aspect, guest thread only

// 4:3 arrives as a rounded float: a squeeze of 0.99999994 would still rewrite the camera.
void widescreen(dream::System&, float aspect) {
    g_squeeze = aspect > 4.0f / 3.0f + 0.001f ? (4.0f / 3.0f) / aspect : 1.0f;
}

// Menus stay 4:3: their art is many separate flat pieces that neither HUD layout keeps together.
// 0x8C25C81C is the game's scene: 2 title, 3 menus, 4 loading, 5 the race from the first frame
// after loading (intro fly-by, countdown, race, end sequence). 0x8C1DD0B8 is 1 on the title and
// in attract demos, 0 in a player's race: it goes to 1 the moment a game over returns to the
// title, and keeps demos 4:3. (The mode word 0x8C1DD1C4 lagged the intro by 50 frames; the race
// clock's state 0x8C1DD1B4 missed the countdown and the results.)
constexpr std::uint32_t kScene = 0x8C25C81C, kAttract = 0x8C1DD0B8;
bool widescreen_now(dream::System& sys) {
    return sys.memory.read32(kScene) == 5 && sys.memory.read32(kAttract) == 0;
}

const dreamcomp::PortInfo kInfo = [] {
    dreamcomp::PortInfo p;
    p.id = "hydrothunder";
    p.title = "Hydro Thunder";
    p.default_bindings = hydrothunder::kKeyboard;
    p.widescreen = &widescreen;
    p.widescreen_now = &widescreen_now;
    p.on_vblank = &hydrothunder::on_vblank_unlocks;
    // Menus and the race HUD are flat polygons (2 sprites in a race frame) at shared depths
    // (race 1.0, 1.11, 1.52-1.56, 10; menus 6e-05-1.25); 3D reaches 1/w 4.7 but never flat and
    // shared, so the default overlay_z stays out of the way.
    p.hud.enabled = true;
    p.hud.polygons = true;
    // Blended frames show stray streaks and misplaced HUD icons in a race (spray particles and
    // blinking HUD pieces reorder between frames, so strips pair with the wrong neighbour).
    p.interpolation = false;
    return p;
}();
dreamcomp::RegisterPort g_register(kInfo);

}  // namespace

DC_HOOK_EXIT(set_camera) {
    if (g_squeeze == 1.0f)
        return;
    for (std::uint32_t i : {0u, 3u, 6u, 9u})
        dreamcomp::write_f32(m, kCameraMatrix + 4 * i,
                             dreamcomp::read_f32(m, kCameraMatrix + 4 * i) * g_squeeze);
}
