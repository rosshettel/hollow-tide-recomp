// Hydro Thunder's keyboard layout, in the bindings file format (dream/render/input.h).
#pragma once

namespace hydrothunder {

// Measured in a Thunder Park race (docs/findings/m5-race.md): R trigger throttle, L trigger brake,
// A boost, Y camera, stick or d-pad left/right steer; d-pad up/down do nothing in a race, so the
// arrows still move through the menus. Z, X, Enter, Q and W keep the engine's meanings.
inline constexpr const char* kKeyboard = R"(version = 1
[keyboard]
rtrigger = key:Up
rtrigger.2 = key:W
ltrigger = key:Down
ltrigger.2 = key:Q
a = key:Space
a.2 = key:Z
y = key:C
y.2 = key:S
stick_up =
stick_down =
)";

}  // namespace hydrothunder
