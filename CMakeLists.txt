cmake_minimum_required(VERSION 3.24)
project(hydrothunder_recomp VERSION 0.1.0 LANGUAGES C CXX)

# Hydro Thunder (Dreamcast) as a native port, built with dreamcomp.
# dreamcomp is vendored at dreamcomp/ (pinned commit: dreamcomp/VENDORED.md); a local checkout
# elsewhere can be used instead with -DDREAMCOMP_DIR=<path>.
set(DREAMCOMP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dreamcomp" CACHE PATH "dreamcomp checkout")
if(NOT EXISTS "${DREAMCOMP_DIR}/cmake/DreamcompPort.cmake")
  message(FATAL_ERROR "dreamcomp not found at ${DREAMCOMP_DIR}")
endif()
# Before add_subdirectory, so the engine's tests (-DDREAM_BUILD_TESTS=ON) run from this ctest.
enable_testing()
# Hydro Thunder functions in the engine's differential harness and golden replay. The captured
# states embed results computed from the game's code, so they stay beside the extracted disc files.
set(HT_IMAGE ${CMAKE_CURRENT_SOURCE_DIR}/game/extracted/fs/1ST_READ.BIN)
set(DREAM_SH4_EXTRA_SLICES ${CMAKE_CURRENT_SOURCE_DIR}/game/slice.cmake)
set(DREAM_SH4_EXTRA_GOLDEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/game/extracted/golden)
add_subdirectory(${DREAMCOMP_DIR} dreamcomp)
add_subdirectory(game)

add_executable(hydrothunder_keyboard_test tests/test_keyboard.cpp)
target_link_libraries(hydrothunder_keyboard_test PRIVATE dream_render)
target_compile_options(hydrothunder_keyboard_test PRIVATE -UNDEBUG)
add_test(NAME hydrothunder.keyboard COMMAND hydrothunder_keyboard_test)

# Disc identity test (docs/findings/phase-00.md). Skips when no image is found.
find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(NAME hydrothunder.identity
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_identity.py)
  set_tests_properties(hydrothunder.identity PROPERTIES SKIP_RETURN_CODE 77)
  add_test(NAME hydrothunder.unlocks
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_unlocks.py)
  set_tests_properties(hydrothunder.unlocks PROPERTIES SKIP_RETURN_CODE 77)
  # The slice against Flycast: needs the oracle core (-DDREAM_FLYCAST_CORE) and the extracted image.
  if(DREAM_FLYCAST_CORE AND EXISTS ${HT_IMAGE} AND TARGET dream_sh4_runner)
    configure_file(game/oracle-cases.json ${CMAKE_CURRENT_BINARY_DIR}/hydrothunder-oracle-cases.json)
    add_test(NAME hydrothunder.differential
      COMMAND ${Python3_EXECUTABLE} ${DREAMCOMP_DIR}/engine/tools/oracle/differential.py
              --runner $<TARGET_FILE:dream_sh4_runner> --lib ${DREAM_FLYCAST_CORE}
              --cases ${CMAKE_CURRENT_BINARY_DIR}/hydrothunder-oracle-cases.json)
  endif()
endif()
