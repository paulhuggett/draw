cmake_minimum_required(VERSION 3.31)
project(draw CXX)

option(DRAW_UNIT_TESTS "Add unit tests" Yes)

find_package (Python3 COMPONENTS Interpreter)
if (NOT Python3_Interpreter_FOUND)
  message(FATAL_ERROR "Python3 interpreter not found") 
endif()

function(setup_target target)
  target_compile_features(${target} PRIVATE cxx_std_23)
  target_compile_options(${target} PRIVATE
    -Weverything
    -Wno-c++20-compat
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-covered-switch-default
    -Wno-exit-time-destructors
    -Wno-global-constructors
    -Wno-padded
    -Wno-poison-system-directories
  )
endfunction(setup_target)


add_subdirectory(lib)


if (DRAW_UNIT_TESTS)
  # Tell gtest to link against the "Multi-threaded Debug DLL runtime library"
  # on Windows.
  set(gtest_force_shared_crt On CACHE BOOL "Always use msvcrt.dll")
  # We don't want to install either gtest or gmock.
  set(INSTALL_GTEST Off CACHE BOOL "Disable gtest install")
  set(INSTALL_GMOCK Off CACHE BOOL "Disable gmock install")
  add_subdirectory(googletest)

  add_subdirectory(unit_tests)
endif(DRAW_UNIT_TESTS)

add_subdirectory(demo)
