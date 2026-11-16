// Hydro Thunder unlocks (M6): CodeBreaker "Enable All Tracks And Boats" for USA V1.020.
// Six 16-bit flags at 0x8C26B8D0..E4; the boot file holds pointers to that block (four sites).
// Off by default: no guest writes, so the write-hash gate matches baseline.
#include "dream/runtime/system.h"
#include "dreamcomp/settings.h"

namespace hydrothunder {
namespace {

// Sage / CodeBreaker: 01xxxxxx 00000001 -> write16(0x8Cxxxxxx, 1).
constexpr std::uint32_t kUnlockBase = 0x8C26B8D0;
constexpr int kUnlockWords = 6;  // +0, +4, +8, +C, +10, +14

void apply(dream::System& sys) {
    for (int i = 0; i < kUnlockWords; ++i)
        sys.memory.write16(kUnlockBase + static_cast<std::uint32_t>(i) * 4, 1);
}

}  // namespace

void on_vblank_unlocks(dream::System& sys, dreamcomp::Settings& settings) {
    // unlock_all covers tracks, boats, and bonus/hidden boats (one CB code family).
    if (!settings.get_bool("unlock_all", false))
        return;
    apply(sys);
}

}  // namespace hydrothunder
