# Cross-compile toolchain: macOS/Linux host -> Windows x86_64 with mingw-w64.
# dreamcomp's primary Windows path is clang-cl on a Windows host; this is the Mac fall-back used
# to stage a portable USB build when no Windows box is available.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
set(CMAKE_ADDR2LINE x86_64-w64-mingw32-addr2line)
set(CMAKE_AR x86_64-w64-mingw32-ar)
set(CMAKE_RANLIB x86_64-w64-mingw32-ranlib)
set(CMAKE_STRIP x86_64-w64-mingw32-strip)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Portable USB folder: bake libgcc/libstdc++ into the exe (avoids the 29MB
# libstdc++-6.dll on the stick). winpthread still needs its small shared DLL.
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++")
