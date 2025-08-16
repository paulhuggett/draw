#===- lib/CMakeLists.txt --------------------------------------------------===//
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
add_subdirectory(font)

add_library(draw STATIC )
target_sources(draw PRIVATE
  "${DRAW_PROJECT_ROOT}/include/draw/font.hpp"
  "${DRAW_PROJECT_ROOT}/include/draw/bitmap.hpp"
  "${DRAW_PROJECT_ROOT}/include/draw/glyph_cache.hpp"
  "${DRAW_PROJECT_ROOT}/include/draw/plru_cache.hpp"
  "${DRAW_PROJECT_ROOT}/include/draw/icubaby.hpp"
  "${DRAW_PROJECT_ROOT}/include/draw/types.hpp"

  bitmap.cpp
  glyph_cache.cpp
)
target_include_directories(draw
  INTERFACE "${DRAW_PROJECT_ROOT}/include"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  INTERFACE "${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(draw PUBLIC draw-font)
setup_target(draw)
