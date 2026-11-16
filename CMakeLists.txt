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
add_subdirectory(${DREAMCOMP_DIR} dreamcomp)
add_subdirectory(game)

# Disc identity test (docs/findings/phase-00.md). Skips when no image is found.
find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(NAME hydrothunder.identity
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_identity.py)
  set_tests_properties(hydrothunder.identity PROPERTIES SKIP_RETURN_CODE 77)
endif()
