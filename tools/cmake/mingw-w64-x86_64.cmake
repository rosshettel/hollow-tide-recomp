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

# Portable USB folder: bake MinGW runtimes into the exe so the stick only needs
# the official SDL3.dll beside hydro-thunder.exe. Windows 11 Smart App Control
# (0xc0e90002) blocks unsigned libwinpthread-1.dll when shipped separately.
#
# -Bstatic must stay open through the driver's trailing -lpthread (do NOT flip
# back to -Bdynamic). These flags have to live at the end of the link line, so
# they go in STANDARD_LIBRARIES — and the build script -D-forces them because
# CMake's Windows-GNU module overwrites *_INIT.
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++")
set(_HT_MINGW_WINLIBS
  "-lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32")
set(CMAKE_C_STANDARD_LIBRARIES
  "${_HT_MINGW_WINLIBS} -Wl,-Bstatic -lwinpthread -lpthread")
set(CMAKE_CXX_STANDARD_LIBRARIES
  "${_HT_MINGW_WINLIBS} -Wl,-Bstatic -lwinpthread -lpthread")
