// The only header translated code includes: everything dream-translate's output may use from the
// runtime. Emitted units static_assert DREAM_GEN_INTERFACE_VERSION against the version their
// translator was built for, so stale generated code fails to compile instead of misbehaving.
// Bump it (and kGenInterfaceVersion in translator/src/emit/emit.cpp) whenever anything reachable
// from here changes in a way emitted code depends on. The runtime never includes generated headers.
#pragma once

#include "dream/runtime/memory.h"
#include "dream/runtime/sh4/abi.h"
#include "dream/runtime/sh4/ctx.h"
#include "dream/runtime/sh4/ops.h"

#define DREAM_GEN_INTERFACE_VERSION 1
