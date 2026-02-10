#===- CMakeLists.txt ------------------------------------------------------===//
#*   ____ __  __       _        _     _     _        *
#*  / ___|  \/  | __ _| | _____| |   (_)___| |_ ___  *
#* | |   | |\/| |/ _` | |/ / _ \ |   | / __| __/ __| *
#* | |___| |  | | (_| |   <  __/ |___| \__ \ |_\__ \ *
#*  \____|_|  |_|\__,_|_|\_\___|_____|_|___/\__|___/ *
#*                                                   *
#===----------------------------------------------------------------------===//
# Copyright © 2025 Paul Bowen-Huggett
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# “Software”), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT
#===----------------------------------------------------------------------===//
cmake_minimum_required(VERSION 3.20)
project(draw CXX)

if (CMAKE_MAJOR_VERSION GREATER_EQUAL 3 AND
    CMAKE_MINOR_VERSION GREATER_EQUAL 25 OR
    CMAKE_MAJOR_VERSION GREATER 3)
  cmake_policy(SET CMP0140 NEW)
endif()

include(CTest)
include(CheckCXXCompilerFlag)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR
    CMAKE_SYSTEM_NAME STREQUAL "Windows" OR
    CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR
    CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  set(default_hosted Yes)
elseif ()
  set(default_hosted No)
endif ()

option(DRAW_WERROR "Compiler warnings are errors?" No)
option(DRAW_UNIT_TESTS "Add unit tests?" Yes)
option(DRAW_HOSTED "Is this a hosted environment?" ${default_hosted})

if (NOT Python_EXECUTABLE)
  find_package (Python3 COMPONENTS Interpreter)
  if (NOT Python3_Interpreter_FOUND)
    message(FATAL_ERROR "Python3 interpreter not found")
  endif()
endif()

# If the compiler accepts the command-line flag given by the
# 'flag' parameter, 'result' is set to flag otherwise to an
# empty string.
function(flag_if_available flag result)
  string(MAKE_C_IDENTIFIER "DRAW_HAS_${flag}" flag_var)
  check_cxx_compiler_flag(${flag} ${flag_var})
  if (${flag_var})
    set(${result} ${flag})
  else()
    unset(${result})
  endif()
  return(PROPAGATE ${result})
endfunction(flag_if_available)

# setup target
# ~~~~~~~~~~~~
function (setup_target target)
  target_compile_features(${target} PUBLIC cxx_std_23)

  set(clang_options
    -Weverything
    -Wno-c++14-extensions
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-exit-time-destructors
    -Wno-padded
    -Wno-weak-vtables

    # TODO: just temporary
    -Wno-global-constructors
  )
  set(gcc_options -Wall -Wextra -pedantic -Wno-maybe-uninitialized)
  set(msvc_options
    -W4     # enable lots of warnings
    -wd4068 # unknown pragma
    -wd4324 # structure was padded due to alignment specifier
  )
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang$")
    # Set flags that are not available in all versions of clang.
    set(extra_flags
      -Wno-c++20-compat
      -Wno-c++2a-compat
      -Wno-covered-switch-default
      -Wno-ctad-maybe-unsupported
      -Wno-poison-system-directories
      -Wno-unsafe-buffer-usage
      -Wno-switch-enum
    )
    foreach(flag IN LISTS extra_flags)
      flag_if_available(${flag} new_flag)
      list(APPEND clang_options ${new_flag})
    endforeach()
  endif()

  if (DRAW_WERROR)
    list(APPEND clang_options -Werror)
    list(APPEND gcc_options -Werror)
    list(APPEND msvc_options /WX)
  endif()

  if (DRAW_HOSTED)
    target_compile_definitions(${target} PRIVATE DRAW_HOSTED=1)
  endif()

  target_compile_options(${target}
    PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${clang_options}>
    $<$<CXX_COMPILER_ID:GNU>:${gcc_options}>
    $<$<CXX_COMPILER_ID:MSVC>:${msvc_options}>
  )
  target_link_options(${target}
    PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${clang_options}>
    $<$<CXX_COMPILER_ID:GNU>:${gcc_options}>
    $<$<CXX_COMPILER_ID:MSVC>:>
  )
endfunction(setup_target)

set(DRAW_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(lib)

if (DRAW_UNIT_TESTS)
  enable_testing()
  # Tell gtest to link against the "Multi-threaded Debug DLL runtime library"
  # on Windows.
  set(gtest_force_shared_crt On CACHE BOOL "Always use msvcrt.dll")
  # We don't want to install either gtest or gmock.
  set(INSTALL_GTEST Off CACHE BOOL "Disable gtest install")
  set(INSTALL_GMOCK Off CACHE BOOL "Disable gmock install")
  add_subdirectory(googletest)
  add_subdirectory(unit_tests)
endif(DRAW_UNIT_TESTS)

if (DRAW_HOSTED)
  add_subdirectory(demo)
endif()
