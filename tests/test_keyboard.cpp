// The Hydro Thunder keyboard layout parses cleanly and keeps the menu keys working.
#include <cassert>
#include <cstdio>
#include <string>

#include "dream/render/input.h"
#include "../src/keyboard.h"

int main() {
    using dream::render::Binding;
    using dream::render::BindSource;
    using dream::render::Bindings;
    using dream::render::PadControl;
    std::string warnings;
    const Bindings b = Bindings::from_text(hydrothunder::kKeyboard, &warnings);
    if (!warnings.empty())
        std::fprintf(stderr, "%s", warnings.c_str());
    assert(warnings.empty());
    const auto key = [](const char* k) { return Binding{BindSource::Key, k, 1}; };
    const auto& kb = b.keyboard;
    assert(kb.slot(PadControl::RightTrigger, 0) == key("Up"));
    assert(kb.slot(PadControl::RightTrigger, 1) == key("W"));
    assert(kb.slot(PadControl::LeftTrigger, 0) == key("Down"));
    assert(kb.slot(PadControl::A, 0) == key("Space"));
    assert(kb.slot(PadControl::A, 1) == key("Z"));
    assert(kb.slot(PadControl::Y, 0) == key("C"));
    assert(!kb[PadControl::StickUp].bound() && !kb[PadControl::StickDown].bound());
    // Menus: d-pad and Start as the engine has them; steering on the stick too.
    assert(kb[PadControl::Up] == key("Up") && kb[PadControl::Down] == key("Down"));
    assert(kb[PadControl::StickLeft] == key("Left") && kb[PadControl::StickRight] == key("Right"));
    assert(kb[PadControl::Start] == key("Return"));
    assert(b.gamepad[PadControl::RightTrigger] == Bindings::defaults().gamepad[PadControl::RightTrigger]);
    std::puts("keyboard layout ok");
    return 0;
}
