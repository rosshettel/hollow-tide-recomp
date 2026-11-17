#include "dream/render/input.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace dream::render {
namespace {

struct Entry {
    PadControl c;
    const char* key;    // in the file
    const char* label;  // in the UI
};

// The file keys are a promise: change one and everybody's binding for it silently reverts.
constexpr Entry kEntries[] = {
    {PadControl::A, "a", "A"},
    {PadControl::B, "b", "B"},
    {PadControl::X, "x", "X"},
    {PadControl::Y, "y", "Y"},
    {PadControl::Start, "start", "START"},
    {PadControl::Up, "dpad_up", "D-PAD UP"},
    {PadControl::Down, "dpad_down", "D-PAD DOWN"},
    {PadControl::Left, "dpad_left", "D-PAD LEFT"},
    {PadControl::Right, "dpad_right", "D-PAD RIGHT"},
    {PadControl::LeftTrigger, "ltrigger", "L TRIGGER"},
    {PadControl::RightTrigger, "rtrigger", "R TRIGGER"},
    {PadControl::StickUp, "stick_up", "STICK UP"},
    {PadControl::StickDown, "stick_down", "STICK DOWN"},
    {PadControl::StickLeft, "stick_left", "STICK LEFT"},
    {PadControl::StickRight, "stick_right", "STICK RIGHT"},
};
static_assert(sizeof kEntries / sizeof kEntries[0] == kPadControlCount,
              "every PadControl needs a file key and a label");

struct RoleEntry {
    JoyRole r;
    const char* key;
    const char* label;
};
constexpr RoleEntry kRoles[] = {
    {JoyRole::Wheel, "wheel", "WHEEL"},
    {JoyRole::Pedals, "pedals", "PEDALS"},
    {JoyRole::Handbrake, "handbrake", "HANDBRAKE"},
};
static_assert(sizeof kRoles / sizeof kRoles[0] == kJoyRoleCount,
              "every JoyRole needs a file key and a label");

std::string trim(std::string_view s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.remove_prefix(1);
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.remove_suffix(1);
    return std::string(s);
}

std::string upper(std::string s) {
    for (char& ch : s) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    return s;
}

bool parse_joy_code(std::string_view code, std::string& role_and_index) {
    // "wheel:0" / "pedals:12" — role key, colon, non-negative axis/button index.
    const auto colon = code.find(':');
    if (colon == std::string_view::npos || colon == 0 || colon + 1 >= code.size())
        return false;
    JoyRole role{};
    if (!joy_role_from_key(code.substr(0, colon), role))
        return false;
    const std::string_view idx = code.substr(colon + 1);
    if (idx.empty() || !std::all_of(idx.begin(), idx.end(), [](char c) {
            return c >= '0' && c <= '9';
        }))
        return false;
    role_and_index = std::string(code);
    return true;
}

}  // namespace

const char* pad_control_label(PadControl c) {
    return kEntries[static_cast<unsigned>(c)].label;
}

const char* pad_control_key(PadControl c) {
    return kEntries[static_cast<unsigned>(c)].key;
}

bool pad_control_from_key(std::string_view key, PadControl& out) {
    for (const Entry& e : kEntries) {
        if (key == e.key) {
            out = e.c;
            return true;
        }
    }
    return false;
}

const char* joy_role_key(JoyRole r) {
    return kRoles[static_cast<unsigned>(r)].key;
}

const char* joy_role_label(JoyRole r) {
    return kRoles[static_cast<unsigned>(r)].label;
}

bool joy_role_from_key(std::string_view key, JoyRole& out) {
    for (const RoleEntry& e : kRoles) {
        if (key == e.key) {
            out = e.r;
            return true;
        }
    }
    return false;
}

std::string Binding::text() const {
    switch (source) {
        case BindSource::Key:
            return "key:" + code;
        case BindSource::Button:
            return "button:" + code;
        case BindSource::Axis:
            return "axis:" + code + (sign < 0 ? "-" : "+");
        case BindSource::JoyAxis:
            // sign 0 = full-range pedal (rest at the low end); +/- = centred half-axis (wheel).
            if (sign == 0)
                return "jaxis:" + code;
            return "jaxis:" + code + (sign < 0 ? "-" : "+");
        case BindSource::JoyButton:
            return "jbutton:" + code;
        case BindSource::None:
            break;
    }
    return {};
}

bool Binding::parse(std::string_view text, Binding& out) {
    const auto colon = text.find(':');
    if (colon == std::string_view::npos)
        return false;
    const std::string_view kind = text.substr(0, colon);
    std::string code(text.substr(colon + 1));
    if (code.empty())
        return false;
    out = Binding{};
    if (kind == "key") {
        out.source = BindSource::Key;
    } else if (kind == "button") {
        out.source = BindSource::Button;
    } else if (kind == "axis") {
        out.source = BindSource::Axis;
        // The sign is the last character, and it is required: an axis without a half is ambiguous
        // and guessing "+" would bind the wrong direction half the time.
        if (code.back() == '-') {
            out.sign = -1;
        } else if (code.back() == '+') {
            out.sign = 1;
        } else {
            return false;
        }
        code.pop_back();
        if (code.empty())
            return false;
    } else if (kind == "jaxis") {
        out.source = BindSource::JoyAxis;
        out.sign = 0;  // full-range until a trailing +/- says otherwise
        if (code.back() == '-') {
            out.sign = -1;
            code.pop_back();
        } else if (code.back() == '+') {
            out.sign = 1;
            code.pop_back();
        }
        std::string normalised;
        if (!parse_joy_code(code, normalised))
            return false;
        out.code = normalised;
        return true;
    } else if (kind == "jbutton") {
        out.source = BindSource::JoyButton;
        std::string normalised;
        if (!parse_joy_code(code, normalised))
            return false;
        out.code = normalised;
        return true;
    } else {
        return false;
    }
    out.code = code;
    return true;
}

std::string Binding::label() const {
    if (!bound())
        return "--";
    if (source == BindSource::Axis)
        return upper(code) + (sign < 0 ? " -" : " +");
    if (source == BindSource::JoyAxis) {
        if (sign == 0)
            return upper(code);
        return upper(code) + (sign < 0 ? " -" : " +");
    }
    if (source == BindSource::JoyButton)
        return upper(code);
    return upper(code);
}

Bindings Bindings::defaults() {
    Bindings k;
    auto key = [](const char* name) { return Binding{BindSource::Key, name, 1}; };
    // The keyboard table the launcher has always had, so a first run with no file behaves exactly
    // as before this existed.
    k.keyboard[PadControl::A] = key("Z");
    k.keyboard[PadControl::B] = key("X");
    k.keyboard[PadControl::X] = key("A");
    k.keyboard[PadControl::Y] = key("S");
    k.keyboard[PadControl::Start] = key("Return");
    k.keyboard[PadControl::Up] = key("Up");
    k.keyboard[PadControl::Down] = key("Down");
    k.keyboard[PadControl::Left] = key("Left");
    k.keyboard[PadControl::Right] = key("Right");
    k.keyboard[PadControl::LeftTrigger] = key("Q");
    k.keyboard[PadControl::RightTrigger] = key("W");
    // The d-pad drives the stick as well, which is what the launcher did before there were
    // bindings: a title that steers with the stick and ignores the d-pad is otherwise unplayable
    // from a keyboard. Visible in the UI as a real binding rather than hidden in the launcher.
    k.keyboard[PadControl::StickUp] = key("Up");
    k.keyboard[PadControl::StickDown] = key("Down");
    k.keyboard[PadControl::StickLeft] = key("Left");
    k.keyboard[PadControl::StickRight] = key("Right");

    auto btn = [](const char* name) { return Binding{BindSource::Button, name, 1}; };
    auto axis = [](const char* name, int sign) { return Binding{BindSource::Axis, name, sign}; };
    // SDL's own gamepad names, so any pad it recognises works on first plug-in with no UI visit.
    // The Dreamcast's A and B sit where a modern pad's A and B do; X and Y likewise.
    k.gamepad[PadControl::A] = btn("a");
    k.gamepad[PadControl::B] = btn("b");
    k.gamepad[PadControl::X] = btn("x");
    k.gamepad[PadControl::Y] = btn("y");
    k.gamepad[PadControl::Start] = btn("start");
    k.gamepad[PadControl::Up] = btn("dpup");
    k.gamepad[PadControl::Down] = btn("dpdown");
    k.gamepad[PadControl::Left] = btn("dpleft");
    k.gamepad[PadControl::Right] = btn("dpright");
    k.gamepad[PadControl::LeftTrigger] = axis("lefttrigger", 1);
    k.gamepad[PadControl::RightTrigger] = axis("righttrigger", 1);
    k.gamepad[PadControl::StickUp] = axis("lefty", -1);
    k.gamepad[PadControl::StickDown] = axis("lefty", 1);
    k.gamepad[PadControl::StickLeft] = axis("leftx", -1);
    k.gamepad[PadControl::StickRight] = axis("leftx", 1);
    // Joystick bindings stay empty: axis indices differ per wheel/pedal box, so the player assigns
    // roles and captures axes once on the machine that has the hardware.
    return k;
}

std::string Bindings::to_text() const {
    std::ostringstream o;
    o << "# dream-recomp input bindings. Rewritten whenever they change in the UI.\n"
      << "# Physical inputs are SDL's own names, so this file survives an SDL upgrade.\n"
      << "# Joystick roles (wheel/pedals/handbrake) are SDL GUID hex; jaxis/jbutton name the role.\n"
      << "version = 1\n"
      << "deadzone = " << deadzone_percent << "\n"
      << "trigger_ramp = " << (trigger_ramp ? 1 : 0) << "\n";
    for (unsigned i = 0; i < kJoyRoleCount; ++i) {
        const auto r = static_cast<JoyRole>(i);
        o << joy_role_key(r) << "_device = " << joy_guid[i] << "\n";
    }
    auto section = [&o](const char* name, const DeviceBindings& d) {
        o << "\n[" << name << "]\n";
        for (unsigned i = 0; i < kPadControlCount; ++i) {
            const auto c = static_cast<PadControl>(i);
            o << pad_control_key(c) << " = " << d[c].text() << "\n";
        }
        // The second slots (dreamcomp), only where bound: "a.2 = key:Space".
        for (unsigned i = 0; i < kPadControlCount; ++i) {
            const auto c = static_cast<PadControl>(i);
            if (d.slot(c, 1).bound())
                o << pad_control_key(c) << ".2 = " << d.slot(c, 1).text() << "\n";
        }
    };
    section("keyboard", keyboard);
    section("gamepad", gamepad);
    section("joystick", joystick);
    return o.str();
}

Bindings Bindings::from_text(std::string_view text, std::string* warnings) {
    // Start from the defaults rather than from nothing: a file that sets half the controls leaves
    // the other half working, and a file that is complete nonsense leaves the game playable. A
    // binding UI whose config file can brick the controls is worse than no binding UI.
    Bindings out = Bindings::defaults();
    auto warn = [warnings](const std::string& s) {
        if (warnings)
            *warnings += s + "\n";
    };
    DeviceBindings* current = nullptr;
    std::string line;
    std::istringstream in{std::string(text)};
    unsigned lineno = 0;
    while (std::getline(in, line)) {
        ++lineno;
        const std::string s = trim(line);
        if (s.empty() || s[0] == '#')
            continue;
        if (s.front() == '[' && s.back() == ']') {
            const std::string name = trim(s.substr(1, s.size() - 2));
            if (name == "keyboard")
                current = &out.keyboard;
            else if (name == "gamepad")
                current = &out.gamepad;
            else if (name == "joystick")
                current = &out.joystick;
            else {
                current = nullptr;
                warn("line " + std::to_string(lineno) + ": unknown section [" + name + "]");
            }
            continue;
        }
        const auto eq = s.find('=');
        if (eq == std::string::npos) {
            warn("line " + std::to_string(lineno) + ": not a setting");
            continue;
        }
        const std::string key = trim(s.substr(0, eq));
        const std::string value = trim(s.substr(eq + 1));
        if (!current) {
            if (key == "deadzone") {
                // Clamped rather than rejected: a nonsensical number should not cost the whole
                // file, and a 100% dead zone would mean a stick that never moves.
                try {
                    out.deadzone_percent = std::clamp(std::stoi(value), 0, 90);
                } catch (...) {
                    warn("line " + std::to_string(lineno) + ": deadzone is not a number: " + value);
                }
            } else if (key == "trigger_ramp") {
                out.trigger_ramp = value != "0";
            } else if (key.size() > 7 && key.compare(key.size() - 7, 7, "_device") == 0) {
                JoyRole role{};
                if (joy_role_from_key(key.substr(0, key.size() - 7), role))
                    out.joy_guid[static_cast<unsigned>(role)] = value;
                else
                    warn("line " + std::to_string(lineno) + ": unknown setting " + key);
            } else if (key != "version") {
                warn("line " + std::to_string(lineno) + ": unknown setting " + key);
            }
            continue;
        }
        PadControl c{};
        unsigned slot = 0;
        std::string name = key;
        if (name.size() > 2 && name.compare(name.size() - 2, 2, ".2") == 0) {
            name.resize(name.size() - 2);
            slot = 1;
        }
        if (!pad_control_from_key(name, c)) {
            warn("line " + std::to_string(lineno) + ": unknown control " + key);
            continue;
        }
        if (value.empty()) {
            current->slot(c, slot) = Binding{};  // deliberately unbound
            continue;
        }
        Binding b;
        if (!Binding::parse(value, b)) {
            warn("line " + std::to_string(lineno) + ": cannot read binding " + value);
            continue;
        }
        current->slot(c, slot) = b;
    }
    return out;
}

}  // namespace dream::render
