#===- unit_tests/CMakeLists.txt -------------------------------------------===//
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

include(setup_target)

enable_testing()


if(DRAW_FUZZ_TESTS)

  set(FUZZTEST_FUZZING_MODE On)
  set(FUZZTEST_REPO_BRANCH "1635d42" CACHE STRING "FuzzTest repository branch.")
  message(STATUS "Getting fuzztest dependency at branch ${FUZZTEST_REPO_BRANCH}")
  FetchContent_Declare(
    fuzztest
    GIT_REPOSITORY https://github.com/google/fuzztest.git
    GIT_TAG ${FUZZTEST_REPO_BRANCH}
  )
  FetchContent_MakeAvailable(fuzztest)

  fuzztest_setup_fuzzing_flags()
  foreach(target gtest gmock gtest_main gmock_main)
    target_compile_features(${target} PUBLIC cxx_std_23)
  endforeach()

elseif(DRAW_FUZZ_TESTS)

  # Tell gtest to link against the "Multi-threaded Debug DLL runtime library"
  # on Windows.
  set(gtest_force_shared_crt On CACHE BOOL "Always use msvcrt.dll")
  # We don't want to install either gtest or gmock.
  set(INSTALL_GTEST Off CACHE BOOL "Disable gtest install")
  set(INSTALL_GMOCK Off CACHE BOOL "Disable gmock install")

  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG HEAD
  )

  FetchContent_MakeAvailable(googletest)

  # Adjust some compilation options for the googletest targets
  foreach(target gtest gmock gmock_main gtest_main)
    if(TARGET ${target})
      target_compile_options(${target} PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-deprecated-declarations -Wno-covered-switch-default>
      )
      target_compile_features(${target} PUBLIC cxx_std_23)
      target_compile_definitions(${target} PUBLIC GTEST_REMOVE_LEGACY_TEST_CASEAPI_=1)
    endif()
  endforeach()

endif(DRAW_FUZZ_TESTS)



add_executable(draw-unit-tests)
target_sources(draw-unit-tests
  PRIVATE
    create_bitmap.cpp create_bitmap.hpp
    rect.hpp
    test_copy.cpp
    test_draw_char.cpp
    test_font.cpp
    test_frame_rect.cpp
    test_iumap.cpp
    test_line.cpp
    test_line32.cpp
    test_paint_rect.cpp
    test_plru_cache.cpp
    test_rect.cpp
    test_rgba.cpp
    test_rgba_paint_rect.cpp
    test_text.cpp
)
target_compile_definitions(draw-unit-tests PRIVATE DRAW_FUZZ_TESTS=$<BOOL:${DRAW_FUZZ_TESTS}>)
setup_target(draw-unit-tests PEDANTIC $<NOT:$<BOOL:${DRAW_FUZZ_TESTS}>>)

if(APPLE)
  add_custom_command(
    TARGET draw-unit-tests
    POST_BUILD
    COMMAND codesign --sign - --force --deep $<TARGET_FILE:draw-unit-tests>
    COMMENT "Ad-hoc signing draw-unit-tests for macOS System Policy"
  )
endif()


target_link_libraries(draw-unit-tests PRIVATE draw)
if (DRAW_FUZZ_TESTS)
  target_link_libraries(draw-unit-tests PRIVATE gtest gmock)
  link_fuzztest(draw-unit-tests)
else ()
  target_link_libraries(draw-unit-tests PRIVATE gmock_main)
endif (DRAW_FUZZ_TESTS)


include(GoogleTest)
gtest_discover_tests(draw-unit-tests)

