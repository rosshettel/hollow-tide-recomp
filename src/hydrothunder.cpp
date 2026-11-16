// Hydro Thunder: what this port tells dreamcomp.
#include "dream/runtime/system.h"
#include "dreamcomp/hook.h"
#include "dreamcomp/port.h"
#include "keyboard.h"

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
// 0x8C1DD1B4 is 3 from the race's first frame until it ends (0 in menus, 1 while loading); it
// changes exactly twice per race. Attract demos never set it, so they stay 4:3.
constexpr std::uint32_t kRaceState = 0x8C1DD1B4;
bool widescreen_now(dream::System& sys) { return sys.memory.read32(kRaceState) == 3; }

const dreamcomp::PortInfo kInfo = [] {
    dreamcomp::PortInfo p;
    p.id = "hydrothunder";
    p.title = "Hydro Thunder";
    p.default_bindings = hydrothunder::kKeyboard;
    p.widescreen = &widescreen;
    p.widescreen_now = &widescreen_now;
    // Menus and the race HUD are flat polygons (2 sprites in a race frame) at shared depths
    // (race 1.0, 1.11, 1.52-1.56, 10; menus 6e-05-1.25); 3D reaches 1/w 4.7 but never flat and
    // shared, so the default overlay_z stays out of the way.
    p.hud.enabled = true;
    p.hud.polygons = true;
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
