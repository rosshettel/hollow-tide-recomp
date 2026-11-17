// What a player has bound to each of the Dreamcast pad's inputs.
//
// Device-independent on purpose, like the rest of this layer: no SDL here, so the model, the
// defaults and the file format are unit-tested on every CI runner. Physical inputs are held as
// portable *names* ("Return", "righttrigger") rather than as SDL's numeric codes, because a
// binding file outlives the SDL version that wrote it and a renumbered enum would silently rebind
// somebody's controls. The SDL layer resolves names to codes once, when the bindings change.
//
// Three devices are bound independently and all are live at once, which is what players expect: the
// keyboard still works with a pad plugged in, and a wheel with pedals works beside both. The device
// list in the UI chooses which set you are *editing*, not which one is active.
//
// Raw joysticks (wheels, pedal boxes, handbrakes) are not SDL gamepads. Their bindings name a
// *role* (wheel / pedals / handbrake) plus an axis or button index; the role is pinned to a
// physical device by SDL's GUID so the layout survives unplug/replug and a USB stick move.
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace dream::render {

// The pad's inputs as things a player binds. The stick directions are separate entries because a
// keyboard binds four keys to them while a pad binds one axis; the binding layer is where that
// difference belongs, not the launcher.
enum class PadControl : unsigned {
    A,
    B,
    X,
    Y,
    Start,
    Up,
    Down,
    Left,
    Right,
    LeftTrigger,
    RightTrigger,
    StickUp,
    StickDown,
    StickLeft,
    StickRight,
    Count
};
constexpr unsigned kPadControlCount = static_cast<unsigned>(PadControl::Count);

// For the UI. Upper case because the overlay font is.
const char* pad_control_label(PadControl c);
// For the file. Stable forever once written: changing one silently drops that binding.
const char* pad_control_key(PadControl c);
bool pad_control_from_key(std::string_view key, PadControl& out);

enum class BindSource : unsigned { None, Key, Button, Axis, JoyAxis, JoyButton };

// Roles a raw joystick can fill. The GUID for each role lives on Bindings; JoyAxis/JoyButton codes
// name the role, not the GUID, so swapping pedals for another box is one row change.
enum class JoyRole : unsigned { Wheel, Pedals, Handbrake, Count };
constexpr unsigned kJoyRoleCount = static_cast<unsigned>(JoyRole::Count);
const char* joy_role_key(JoyRole r);
const char* joy_role_label(JoyRole r);
bool joy_role_from_key(std::string_view key, JoyRole& out);

struct Binding {
    BindSource source = BindSource::None;
    std::string code;  // SDL's own name for the scancode, button or axis; or "role:index" for joy
    int sign = 1;      // which half of an axis; 0 = full-range pedal (rest at the low end)

    bool bound() const noexcept { return source != BindSource::None; }
    bool operator==(const Binding& o) const noexcept {
        return source == o.source && code == o.code &&
               ((source != BindSource::Axis && source != BindSource::JoyAxis) || sign == o.sign);
    }
    bool operator!=(const Binding& o) const noexcept { return !(*this == o); }

    // "key:Return", "button:a", "axis:righttrigger+", "jaxis:wheel:0+", "jaxis:pedals:1",
    // "jbutton:handbrake:0". Empty when unbound.
    std::string text() const;
    static bool parse(std::string_view text, Binding& out);
    // What the UI shows: the code alone, upper-cased, or "--" when unbound.
    std::string label() const;
};

// Two slots per input (dreamcomp), as the N64 recomp frontends have: either one drives the
// control. operator[] is the first slot, which is all the engine's own binding screen edits by
// default; the joystick screen also edits the second slot so clutch and a handbrake can both boost.
constexpr unsigned kBindingSlots = 2;

struct DeviceBindings {
    std::array<Binding, kPadControlCount> b{};
    std::array<Binding, kPadControlCount> alt{};  // the second slot
    const Binding& operator[](PadControl c) const { return b[static_cast<unsigned>(c)]; }
    Binding& operator[](PadControl c) { return b[static_cast<unsigned>(c)]; }
    const Binding& slot(PadControl c, unsigned s) const {
        return (s == 0 ? b : alt)[static_cast<unsigned>(c)];
    }
    Binding& slot(PadControl c, unsigned s) { return (s == 0 ? b : alt)[static_cast<unsigned>(c)]; }
};

struct Bindings {
    DeviceBindings keyboard;
    DeviceBindings gamepad;
    DeviceBindings joystick;  // raw wheel / pedals / handbrake; always live with the other two
    // SDL joystick GUID hex for each role; empty means that role is unassigned.
    std::array<std::string, kJoyRoleCount> joy_guid{};
    // Percent of an axis's travel ignored around the centre. A worn stick steers on its own
    // without one, which reads as a physics bug rather than as a hardware fault. Pedal full-range
    // bindings ignore this (they rest at one end).
    int deadzone_percent = 20;
    // Ramp a digital trigger to full over about 150 ms rather than jumping. A keyboard accelerator
    // is otherwise fully down or fully up, which for a driving game is most of why a pad matters.
    bool trigger_ramp = true;

    static Bindings defaults();
    std::string to_text() const;
    // Never fails destructively: an unreadable or partial file leaves the defaults in place for
    // anything it did not set, so a bad file cannot make the game unplayable with no way back.
    // `warnings` collects what was ignored and why.
    static Bindings from_text(std::string_view text, std::string* warnings = nullptr);
};

}  // namespace dream::render
