# Rule 2 host shim: platform APIs that are not SDL3 live here, nowhere else in the engine.
add_library(dream_platform STATIC
  src/backtrace.cpp
)
target_include_directories(dream_platform PUBLIC include)
dream_target_defaults(dream_platform)
add_library(dream::platform ALIAS dream_platform)
