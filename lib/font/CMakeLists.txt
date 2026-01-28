#===- lib/font/CMakeLists.txt ---------------------------------------------===//
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
include(FetchContent)
FetchContent_Declare(
  pypng
  GIT_REPOSITORY https://gitlab.com/drj11/pypng.git
  GIT_TAG        b2500eca7bafac80847f16125071c674123bc2dc # Update for release 0.20250521.0
)
FetchContent_MakeAvailable(pypng)
FetchContent_GetProperties(pypng SOURCE_DIR DRAW_PYPNG_DIR)
list(APPEND PYTHONPATH "${DRAW_PYPNG_DIR}/code")
message(STATUS "Setting PYTHONPATH for font.py to '${PYTHONPATH}'")

add_custom_command(
  COMMAND
    mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/draw"
  COMMAND
    "${CMAKE_COMMAND}" -E env "PYTHONPATH=${PYTHONPATH}"
    "${Python3_EXECUTABLE}" font.py -o "${CMAKE_CURRENT_BINARY_DIR}/draw" sans_font/16/sans16.json
  COMMAND
    "${CMAKE_COMMAND}" -E env "PYTHONPATH=${PYTHONPATH}"
    "${Python3_EXECUTABLE}" font.py -o "${CMAKE_CURRENT_BINARY_DIR}/draw" sans_font/32/sans32.json
  OUTPUT
    "${CMAKE_CURRENT_BINARY_DIR}/draw/sans16.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/draw/sans16.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/draw/sans32.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/draw/sans32.hpp"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Building Sans Font"
  DEPENDS
    font.py
    sans_font/16/sans16.json
    sans_font/16/basic_latin-0000-000F.png
    sans_font/16/basic_latin-0010-001F.png
    sans_font/16/basic_latin-0020-002F.png
    sans_font/16/basic_latin-0030-003F.png
    sans_font/16/basic_latin-0040-004F.png
    sans_font/16/basic_latin-0050-005F.png
    sans_font/16/basic_latin-0060-006F.png
    sans_font/16/basic_latin-0070-007F.png
    sans_font/16/basic_latin-0080-008F.png
    sans_font/16/latin1-00A0-00AF.png
    sans_font/16/latin1-00B0-00BF.png
    sans_font/16/latin1-00C0-00CF.png
    sans_font/16/latin1-00D0-00DF.png
    sans_font/16/latin1-00E0-00EF.png
    sans_font/16/latin1-00F0-00FF.png
    sans_font/16/white-square-25A1.png
    sans_font/16/replacement-FFFD.png
    sans_font/32/sans32.json
    sans_font/32/basic_latin-0020-002F.png
    sans_font/32/basic_latin-0030-003F.png
)
add_library(draw-font STATIC )
target_sources(draw-font PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/draw/sans16.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/draw/sans16.hpp"
  "${CMAKE_CURRENT_BINARY_DIR}/draw/sans32.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/draw/sans32.hpp"
  font.py
  sans_font/16/sans16.json
  sans_font/32/sans32.json
)

target_include_directories(draw-font
  PUBLIC "${DRAW_PROJECT_ROOT}/include"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    PUBLIC "${CMAKE_CURRENT_BINARY_DIR}"
)
setup_target(draw-font)
