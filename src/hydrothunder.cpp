// Hydro Thunder: what this port tells dreamcomp.
#include "dreamcomp/hook.h"
#include "dreamcomp/port.h"

namespace {

const dreamcomp::PortInfo kInfo = [] {
    dreamcomp::PortInfo p;
    p.id = "hydrothunder";
    p.title = "Hydro Thunder";
    return p;
}();
dreamcomp::RegisterPort g_register(kInfo);

}  // namespace
