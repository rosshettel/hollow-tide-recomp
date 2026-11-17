#include "dream/platform/backtrace.h"

#include <cstdio>
#include <cstring>

#if defined(__APPLE__) || defined(__linux__)
#include <execinfo.h>
#include <cstdlib>
#endif

namespace dream::platform {

void print_guest_backtrace() {
#if defined(__APPLE__) || defined(__linux__)
    void* frames[64];
    const int n = ::backtrace(frames, 64);
    char** names = ::backtrace_symbols(frames, n);
    if (!names)
        return;
    std::fprintf(stderr, "  guest call chain (innermost first):\n");
    unsigned shown = 0;
    for (int i = 0; i < n && shown < 12; ++i) {
        const char* fn = std::strstr(names[i], "fn_0c");
        if (!fn)
            fn = std::strstr(names[i], "fn_8c");
        if (!fn)
            continue;
        char addr[16] = {};
        std::snprintf(addr, sizeof addr, "%.11s", fn);
        std::fprintf(stderr, "    %s\n", addr);
        ++shown;
    }
    if (shown == 0)
        std::fprintf(stderr,
                     "    (no emitted frames on the stack; the fault is in the runtime "
                     "or in interpreted code)\n");
    std::free(names);
#else
    std::fprintf(stderr, "  guest call chain: not available on this platform\n");
#endif
}

}  // namespace dream::platform
