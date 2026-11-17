// Host backtrace helpers (rule 2 shim). Call sites must not include <execinfo.h> or <windows.h>.
#pragma once

#include <cstddef>

namespace dream::platform {

// Walk the host stack looking for emitted guest frames (fn_0c… / fn_8c…) and print them.
// No-op where the host has no backtrace API.
void print_guest_backtrace();

}  // namespace dream::platform
