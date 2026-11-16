#pragma once

namespace dream {
class System;
}
namespace dreamcomp {
class Settings;
}

namespace hydrothunder {

// Re-assert unlock flags each vblank when unlock_all=true (default off).
void on_vblank_unlocks(dream::System& sys, dreamcomp::Settings& settings);

}  // namespace hydrothunder
