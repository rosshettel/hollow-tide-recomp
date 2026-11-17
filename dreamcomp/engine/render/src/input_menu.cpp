#include "dream/render/input_menu.h"

#include <algorithm>

#include "dream/render/overlay.h"

namespace dream::render {
namespace {
using u32 = std::uint32_t;

const char* kEditNames[kEditDeviceCount] = {"KEYBOARD", "GAMEPAD", "JOYSTICK"};
}  // namespace

void InputMenu::open(const Bindings& b) {
    bindings_ = b;
    open_ = true;
    awaiting_ = false;
    dirty_ = false;
    edit_slot_ = 0;
    row_ = first_binding();
}

void InputMenu::set_joystick_choices(std::vector<std::pair<std::string, std::string>> choices) {
    joy_choices_ = std::move(choices);
}

DeviceBindings& InputMenu::editing() {
    if (device_ == 1)
        return bindings_.gamepad;
    if (device_ == 2)
        return bindings_.joystick;
    return bindings_.keyboard;
}

const DeviceBindings& InputMenu::editing() const {
    if (device_ == 1)
        return bindings_.gamepad;
    if (device_ == 2)
        return bindings_.joystick;
    return bindings_.keyboard;
}

bool InputMenu::on_binding(int row, PadControl& out) const {
    if (row < first_binding() || row >= reset_row())
        return false;
    out = static_cast<PadControl>(row - first_binding());
    return true;
}

bool InputMenu::on_joy_role(int row, JoyRole& out) const {
    if (device_ != 2 || row < 1 || row > 3)
        return false;
    out = static_cast<JoyRole>(row - 1);
    return true;
}

bool InputMenu::on_deadzone(int row) const {
    return row == (device_ == 2 ? 4 : 1);
}

bool InputMenu::on_ramp(int row) const {
    return row == (device_ == 2 ? 5 : 2);
}

std::string InputMenu::role_device_label(JoyRole r) const {
    const std::string& guid = bindings_.joy_guid[static_cast<unsigned>(r)];
    if (guid.empty())
        return "--";
    for (const auto& c : joy_choices_) {
        if (c.first == guid)
            return c.second.empty() ? guid : c.second;
    }
    // Saved GUID not currently plugged in: still show something stable.
    return guid.size() > 8 ? guid.substr(0, 8) + "…" : guid;
}

void InputMenu::move(int delta) {
    if (awaiting_)
        return;  // the screen is modal while it waits: moving would bind the wrong row
    const int n = row_count();
    row_ = (row_ + delta % n + n) % n;
}

void InputMenu::adjust(int delta, unsigned) {
    if (awaiting_ || delta == 0)
        return;
    if (on_device(row_)) {
        const int n = static_cast<int>(kEditDeviceCount);
        const unsigned prev = device_;
        device_ = static_cast<unsigned>((static_cast<int>(device_) + delta % n + n) % n);
        if (device_ != prev) {
            edit_slot_ = 0;
            // Keep the cursor on a comparable row when the header height changes.
            if (row_ >= first_binding()) {
                const int bind = row_ - (prev == 2 ? 6 : 3);
                row_ = first_binding() + std::clamp(bind, 0, static_cast<int>(kPadControlCount) + 1);
            } else {
                row_ = 0;
            }
        }
        return;
    }
    JoyRole role{};
    if (on_joy_role(row_, role)) {
        // Cycle: unassigned -> each connected joystick -> unassigned.
        const int n = static_cast<int>(joy_choices_.size()) + 1;
        int idx = 0;  // 0 = unassigned
        const std::string& cur = bindings_.joy_guid[static_cast<unsigned>(role)];
        if (!cur.empty()) {
            for (int i = 0; i < static_cast<int>(joy_choices_.size()); ++i) {
                if (joy_choices_[static_cast<size_t>(i)].first == cur) {
                    idx = i + 1;
                    break;
                }
            }
        }
        idx = (idx + delta % n + n) % n;
        bindings_.joy_guid[static_cast<unsigned>(role)] =
            idx == 0 ? std::string() : joy_choices_[static_cast<size_t>(idx - 1)].first;
        dirty_ = true;
        return;
    }
    if (on_deadzone(row_)) {
        bindings_.deadzone_percent = std::clamp(bindings_.deadzone_percent + delta * 5, 0, 90);
        dirty_ = true;
        return;
    }
    if (on_ramp(row_)) {
        bindings_.trigger_ramp = !bindings_.trigger_ramp;
        dirty_ = true;
        return;
    }
    PadControl c{};
    if (device_ == 2 && on_binding(row_, c)) {
        // Left/right on a joystick binding picks which slot is being edited (clutch vs handbrake).
        edit_slot_ = delta > 0 ? 1u : 0u;
    }
}

InputMenu::Action InputMenu::activate() {
    if (awaiting_)
        return Action::None;
    if (row_ == close_row())
        return Action::Close;
    if (row_ == reset_row()) {
        // Only this device's set, so resetting the keyboard cannot silently discard a pad layout
        // somebody spent longer on. Joystick reset also clears role assignments.
        const Bindings d = Bindings::defaults();
        if (device_ == 0)
            editing() = d.keyboard;
        else if (device_ == 1)
            editing() = d.gamepad;
        else {
            editing() = d.joystick;
            bindings_.joy_guid = d.joy_guid;
        }
        dirty_ = true;
        return Action::Save;
    }
    if (on_ramp(row_)) {
        bindings_.trigger_ramp = !bindings_.trigger_ramp;
        dirty_ = true;
        return Action::Save;
    }
    PadControl c{};
    if (on_binding(row_, c)) {
        awaiting_ = true;
        return Action::None;
    }
    return Action::None;
}

void InputMenu::back() {
    if (awaiting_)
        awaiting_ = false;  // one escape leaves the capture, a second leaves the screen
    else
        open_ = false;
}

void InputMenu::apply_capture(const Binding& b, bool cancelled) {
    if (!awaiting_)
        return;
    awaiting_ = false;
    if (cancelled)
        return;
    PadControl c{};
    if (!on_binding(row_, c))
        return;
    // A physical input bound to two controls at once means pressing it does both, which reads as a
    // broken pad. The older binding gives way, and the row it came from is left unbound and
    // visibly so, rather than the new binding being refused with no explanation.
    DeviceBindings& d = editing();
    const unsigned slot = device_ == 2 ? edit_slot_ : 0u;
    for (unsigned i = 0; i < kPadControlCount; ++i) {
        if (d.b[i] == b)
            d.b[i] = Binding{};
        if (d.alt[i] == b)
            d.alt[i] = Binding{};
    }
    d.slot(c, slot) = b;
    dirty_ = true;
}

void InputMenu::draw(u32* rgba, unsigned pitch, unsigned width, unsigned height,
                     const std::vector<std::string>& devices) const {
    if (!open_ || !rgba || width == 0 || height == 0)
        return;
    const int w = static_cast<int>(width), h = static_cast<int>(height);
    // Everything is proportional to the frame, so the screen looks the same at --scale 1 and 4.
    const unsigned s = std::max(1u, width / 320u);
    const int line = static_cast<int>(text_height(s)) + static_cast<int>(s);
    const int pad = static_cast<int>(s) * 6;

    fill_rect(rgba, pitch, width, height, 0, 0, w, h, kUiShade);

    const int card_w = std::min(w - pad * 2, static_cast<int>(s) * 280);
    const int rows = row_count();
    const int card_h = std::min(h - pad * 2, line * (rows + 5) + pad * 2);
    const int cx = (w - card_w) / 2, cy = (h - pad * 2 - card_h) > 0 ? (h - card_h) / 2 : pad;
    fill_rect(rgba, pitch, width, height, cx, cy, card_w, card_h, kUiWhite);

    // The banner. A plain white block marks it and nothing more: an orange band with white type is
    // a palette, which nobody owns, while the console's spiral is a registered trademark and
    // reproducing it in this tool's own UI would be a bad idea however it was drawn.
    const int banner_h = line + pad;
    fill_rect(rgba, pitch, width, height, cx, cy, card_w, banner_h, kUiOrange);
    const int mark = banner_h / 3;
    fill_rect(rgba, pitch, width, height, cx + pad, cy + (banner_h - mark) / 2, mark, mark,
              kUiWhite);
    TextStyle banner{kUiWhite, 0, s};
    draw_text(rgba, pitch, width, height, cx + pad + mark + pad, cy + pad / 2, "CONTROLLER",
              banner);

    const int left = cx + pad;
    const int right = cx + card_w - pad;
    int y = cy + banner_h + pad;

    // One row. The selected row is a full-width orange bar with white type, which is the only
    // strong colour on the card and so reads as the cursor at a glance.
    auto row = [&](int index, const std::string& name, const std::string& value) {
        const bool sel = index == row_;
        if (sel)
            fill_rect(rgba, pitch, width, height, left - static_cast<int>(s) * 2,
                      y - static_cast<int>(s), right - left + static_cast<int>(s) * 4, line,
                      kUiOrange);
        const TextStyle t{sel ? kUiWhite : kUiInk, 0, s};
        draw_text(rgba, pitch, width, height, left, y, name, t);
        if (!value.empty()) {
            const int vx = right - static_cast<int>(text_width(value, s));
            draw_text(rgba, pitch, width, height, vx, y, value, t);
        }
        y += line;
    };

    const std::string device_name = device_ < devices.size() && !devices[device_].empty()
                                        ? devices[device_]
                                        : std::string(kEditNames[device_ < kEditDeviceCount
                                                                     ? device_
                                                                     : 0]);
    row(0, "DEVICE", "< " + device_name + " >");
    if (device_ == 2) {
        row(1, joy_role_label(JoyRole::Wheel), "< " + role_device_label(JoyRole::Wheel) + " >");
        row(2, joy_role_label(JoyRole::Pedals), "< " + role_device_label(JoyRole::Pedals) + " >");
        row(3, joy_role_label(JoyRole::Handbrake),
            "< " + role_device_label(JoyRole::Handbrake) + " >");
        row(4, "DEAD ZONE", "< " + std::to_string(bindings_.deadzone_percent) + "% >");
        row(5, "TRIGGER RAMP", bindings_.trigger_ramp ? "< ON >" : "< OFF >");
    } else {
        row(1, "DEAD ZONE", "< " + std::to_string(bindings_.deadzone_percent) + "% >");
        row(2, "TRIGGER RAMP", bindings_.trigger_ramp ? "< ON >" : "< OFF >");
    }

    y += static_cast<int>(s) * 2;
    fill_rect(rgba, pitch, width, height, left, y, right - left, static_cast<int>(s), kUiGrey);
    y += static_cast<int>(s) * 3;

    for (unsigned i = 0; i < kPadControlCount; ++i) {
        const auto c = static_cast<PadControl>(i);
        const int index = first_binding() + static_cast<int>(i);
        const bool waiting = awaiting_ && index == row_;
        std::string label = pad_control_label(c);
        if (device_ == 2 && edit_slot_ == 1 && index == row_)
            label += " .2";
        const Binding& shown =
            device_ == 2 ? editing().slot(c, edit_slot_) : editing()[c];
        // When both boost slots are set, show a compact hint on A even if editing slot 0.
        std::string value = waiting ? "PRESS ANY INPUT" : shown.label();
        if (!waiting && device_ == 2 && c == PadControl::A && editing().slot(c, 1).bound() &&
            edit_slot_ == 0)
            value += " / " + editing().slot(c, 1).label();
        row(index, label, value);
    }

    y += static_cast<int>(s) * 2;
    row(reset_row(), "RESET THIS DEVICE", "");
    row(close_row(), "CLOSE", "");

    // The footer says how to work the screen, because the screen is the only place that says it.
    const TextStyle hint{kUiGrey, 0, s};
    const char* help =
        awaiting_ ? "PRESS AN INPUT TO BIND IT, OR ESCAPE TO CANCEL"
        : device_ == 2
            ? "ARROWS MOVE, L/R SLOT OR DEVICE, RETURN BINDS"
            : "ARROWS MOVE, RETURN BINDS, ESCAPE CLOSES";
    draw_text(rgba, pitch, width, height, left, cy + card_h - line, help, hint);
}

}  // namespace dream::render
