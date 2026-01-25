#===- demo/CMakeLists.txt -------------------------------------------------===//
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

if (DRAW_HOSTED)
  find_package(Curses)
  if(Curses_FOUND AND NOT TARGET Curses::Curses)
    add_library(Curses::Curses INTERFACE IMPORTED)
    set_target_properties(
      Curses::Curses
      PROPERTIES
        INTERFACE_LINK_LIBRARIES "${CURSES_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${CURSES_INCLUDE_DIRS}"
    )

    add_executable(draw-curses-count)
    target_sources(draw-curses-count PRIVATE curses_count.cpp)
    target_link_libraries(draw-curses-count PRIVATE draw)
    target_link_libraries(draw-curses-count PRIVATE Curses::Curses)
    setup_target(draw-curses-count)
  endif()

  add_executable(draw-time)
  target_sources(draw-time PRIVATE time.cpp)
  target_link_libraries(draw-time PRIVATE draw)
  setup_target(draw-time)

  add_executable(draw-all-chars)
  target_sources(draw-all-chars PRIVATE all_chars.cpp)
  target_link_libraries(draw-all-chars PRIVATE draw)
  setup_target(draw-all-chars)
endif()

