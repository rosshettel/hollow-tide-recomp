// Hydro Thunder: what this port tells dreamcomp.
#include "dreamcomp/hook.h"
#include "dreamcomp/port.h"
#include "keyboard.h"

namespace {

const dreamcomp::PortInfo kInfo = [] {
    dreamcomp::PortInfo p;
    p.id = "hydrothunder";
    p.title = "Hydro Thunder";
    p.default_bindings = hydrothunder::kKeyboard;
    return p;
}();
dreamcomp::RegisterPort g_register(kInfo);

}  // namespace
