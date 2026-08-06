#===- CMakeLists.txt ------------------------------------------------------===//
#*   ____ __  __       _        _     _     _        *
#*  / ___|  \/  | __ _| | _____| |   (_)___| |_ ___  *
#* | |   | |\/| |/ _` | |/ / _ \ |   | / __| __/ __| *
#* | |___| |  | | (_| |   <  __/ |___| \__ \ |_\__ \ *
#*  \____|_|  |_|\__,_|_|\_\___|_____|_|___/\__|___/ *
#*                                                   *
#===----------------------------------------------------------------------===//
# SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
# SPDX-License-Identifier: MIT
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
#===----------------------------------------------------------------------===//
cmake_minimum_required(VERSION 3.20)
project(draw CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if ((CMAKE_MAJOR_VERSION GREATER_EQUAL 3 AND CMAKE_MINOR_VERSION GREATER_EQUAL 25) OR CMAKE_MAJOR_VERSION GREATER_EQUAL 4)
  cmake_policy(SET CMP0140 NEW)
endif()


include(CTest)
include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)
include(FetchContent)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR
    CMAKE_SYSTEM_NAME STREQUAL "Windows" OR
    CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR
    CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  set(default_hosted Yes)
else()
  set(default_hosted No)
endif()


option(DRAW_COVERAGE "Enable code coverage" No)
option(DRAW_HOSTED "Is this a hosted environment?" ${default_hosted})
option(DRAW_UNIT_TESTS "Add unit tests?" Yes)
option(DRAW_FUZZ_TESTS "Use Google Fuzztest?" No)
option(DRAW_WERROR "Compiler warnings are errors?" No)


if (NOT Python_EXECUTABLE)
  find_package (Python3 COMPONENTS Interpreter)
  if (NOT Python3_Interpreter_FOUND)
    message(FATAL_ERROR "Python3 interpreter not found")
  endif()
endif()

FetchContent_Declare(
  icubaby
  GIT_REPOSITORY https://github.com/paulhuggett/icubaby.git
  GIT_TAG HEAD
)
if(DRAW_FUZZ_TESTS)
  # If we're fuzz testing, stop icubaby from dragging in Google Test.
  # This interferes with fuzztest's attempt to set up the library.
  set(ICUBABY_UNIT_TESTS No CACHE BOOL "Disable icubaby unit tests")
endif()
FetchContent_MakeAvailable(icubaby)

set(DRAW_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(lib)

if (DRAW_UNIT_TESTS OR DRAW_FUZZ_TESTS)
  add_subdirectory(unit_tests)
endif()

if (DRAW_HOSTED)
  add_subdirectory(demo)
endif()
