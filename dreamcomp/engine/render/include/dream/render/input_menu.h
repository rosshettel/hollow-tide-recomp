// The binding screen: what it shows, what the keys do, and how it draws.
//
// Device-independent, so the navigation and the rebinding state machine are unit-tested without a
// window. The caller feeds it presses and captured inputs; it owns the Bindings and says when they
// need saving.
//
// The look is a white card with an orange banner and black type, which is of the period without
// reproducing anyone's mark. Hand-drawn rather than built on a general widget toolkit because the
// screen is a fixed list of rows, and a toolkit would mean a vendored dependency and an ADR for
// something that fits in a few hundred lines.
#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "dream/render/input.h"

namespace dream::render {

// An orange, a white and a near-black that reads as ink rather than as a hole in the card. A
// palette and a layout are not anybody's property; a logo is, so this screen carries none.
constexpr std::uint32_t kUiOrange = 0xFF2D8BEEu;  // RGBA, red in the low byte
constexpr std::uint32_t kUiWhite = 0xFFFFFFFFu;
constexpr std::uint32_t kUiInk = 0xFF241E1Cu;
constexpr std::uint32_t kUiGrey = 0xFF8A8A8Au;
constexpr std::uint32_t kUiShade = 0xB4000000u;  // dims the game behind the card

// Fixed edit targets: keyboard, gamepad set, raw joystick (wheel/pedals/handbrake).
constexpr unsigned kEditDeviceCount = 3;

class InputMenu {
public:
    enum class Action { None, Close, Save };

    void open(const Bindings& b);
    void close() { open_ = false; }
    bool is_open() const noexcept { return open_; }

    // Which device's bindings are being edited. All three are always live in play; this is the
    // list the screen shows. 0 keyboard, 1 gamepad, 2 joystick.
    unsigned device() const noexcept { return device_; }
    unsigned edit_slot() const noexcept { return edit_slot_; }

    const Bindings& bindings() const noexcept { return bindings_; }
    bool dirty() const noexcept { return dirty_; }
    void mark_saved() noexcept { dirty_ = false; }

    // True while the screen is waiting for a physical input to bind.
    bool awaiting() const noexcept { return awaiting_; }

    // Connected raw joysticks for the WHEEL / PEDALS / HANDBRAKE rows: (guid, display name).
    // An empty choice is implied as "--" (unassigned). Caller refreshes from SDL while open.
    void set_joystick_choices(std::vector<std::pair<std::string, std::string>> choices);

    // Navigation. `device_count` is accepted for call-site compatibility; wrapping uses
    // kEditDeviceCount.
    void move(int delta);
    void adjust(int delta, unsigned device_count = kEditDeviceCount);
    Action activate();
    void back();

    void apply_capture(const Binding& b, bool cancelled);

    // `devices` should list the three edit targets (KEYBOARD / GAMEPAD / JOYSTICK). Extra names are
    // ignored; fewer than three fall back to those labels.
    void draw(std::uint32_t* rgba, unsigned pitch, unsigned width, unsigned height,
              const std::vector<std::string>& devices) const;

private:
    // Keyboard/gamepad: DEVICE, DEADZONE, RAMP, bindings..., RESET, CLOSE.
    // Joystick: DEVICE, WHEEL, PEDALS, HANDBRAKE, DEADZONE, RAMP, bindings..., RESET, CLOSE.
    int top_count() const { return device_ == 2 ? 6 : 3; }
    int first_binding() const { return top_count(); }
    int row_count() const { return first_binding() + static_cast<int>(kPadControlCount) + 2; }
    int reset_row() const { return first_binding() + static_cast<int>(kPadControlCount); }
    int close_row() const { return reset_row() + 1; }
    bool on_binding(int row, PadControl& out) const;
    bool on_joy_role(int row, JoyRole& out) const;
    bool on_deadzone(int row) const;
    bool on_ramp(int row) const;
    bool on_device(int row) const { return row == 0; }

    Bindings bindings_{};
    DeviceBindings& editing();
    const DeviceBindings& editing() const;
    std::string role_device_label(JoyRole r) const;

    bool open_ = false;
    bool awaiting_ = false;
    bool dirty_ = false;
    unsigned device_ = 0;
    unsigned edit_slot_ = 0;  // second slot lets clutch and handbrake both bind boost
    int row_ = 0;
    std::vector<std::pair<std::string, std::string>> joy_choices_;
};

}  // namespace dream::render
