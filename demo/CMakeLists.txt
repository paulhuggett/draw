#===- demo/CMakeLists.txt -------------------------------------------------===//
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

    add_executable(draw-curses-count curses_count.cpp)
    setup_target(draw-curses-count)
    target_link_libraries(draw-curses-count PRIVATE draw Curses::Curses)
    add_test(NAME draw-curses-count COMMAND draw-curses-count -one-shot)
    set_property(TEST draw-curses-count PROPERTY ENVIRONMENT "TERM=dumb")

    add_executable(draw-curses-rects curses_rect.cpp)
    setup_target(draw-curses-rects)
    target_link_libraries(draw-curses-rects PRIVATE draw Curses::Curses)

    find_package(Python3)
    if(NOT Python3_FOUND)
      message(WARNING "Python3 was not found: the draw-curses-rects test is not available.")
    else()
      add_test(
        NAME
          draw-curses-rects-json
        COMMAND
          "${Python3_EXECUTABLE}"
          "${CMAKE_CURRENT_SOURCE_DIR}/curses_rect_compare.py"
          "$<TARGET_FILE:draw-curses-rects>"
          "${CMAKE_CURRENT_SOURCE_DIR}/expected.json"
      )
    endif(NOT Python3_FOUND)

    # Run curses output for 100 frames. Set the box size to 3x3, the distance vector to [2,1],
    # a 1ms delay between frames
    add_test(
      NAME
        draw-curses-rects
      COMMAND
        draw-curses-rects
        -f 100     # frames
        -w 3 -h 3  # box width/height
        -x 2 -y 1  # distance vector x/y
        -d 1       # inter-frame delay
    )
    set_property(TEST draw-curses-rects PROPERTY ENVIRONMENT "TERM=dumb")
  endif()

  add_executable(draw-time)
  target_sources(draw-time PRIVATE time.cpp)
  target_link_libraries(draw-time PRIVATE draw)
  setup_target(draw-time)
  add_test(NAME draw-time COMMAND draw-time)

  add_executable(draw-all-chars)
  target_sources(draw-all-chars PRIVATE all_chars.cpp)
  target_link_libraries(draw-all-chars PRIVATE draw)
  setup_target(draw-all-chars)
  add_test(NAME draw-all-chars COMMAND draw-all-chars)
endif(DRAW_HOSTED)
