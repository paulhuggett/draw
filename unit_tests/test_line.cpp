//===- unit_tests/test_line.cpp -------------------------------------------===//
//*  _ _             *
//* | (_)_ __   ___  *
//* | | | '_ \ / _ \ *
//* | | | | | |  __/ *
//* |_|_|_| |_|\___| *
//*                  *
//===----------------------------------------------------------------------===//
// Copyright © 2025 Paul Bowen-Huggett
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// “Software”), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// SPDX-License-Identifier: MIT
//===----------------------------------------------------------------------===//
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tuple>

#include "draw/bitmap.hpp"

using testing::ElementsAre;
using namespace draw::literals;

namespace {

TEST(Line, ShortHorizontal) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 8);
  bmp.line(draw::point{2, 5}, draw::point{11, 5});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b,  // [3]
                                       0b00000000_b, 0b00000000_b,  // [4]
                                       0b00111111_b, 0b11110000_b,  // [5]
                                       0b00000000_b, 0b00000000_b,  // [6]
                                       0b00000000_b, 0b00000000_b   // [7]
                                       ));
}

TEST(Line, VeryShortHorizontal) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 8);
  bmp.line(draw::point{2, 5}, draw::point{6, 5});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b,  // [3]
                                       0b00000000_b, 0b00000000_b,  // [4]
                                       0b00111110_b, 0b00000000_b,  // [5]
                                       0b00000000_b, 0b00000000_b,  // [6]
                                       0b00000000_b, 0b00000000_b   // [7]
                                       ));
}

TEST(Line, VeryShortHorizontalInTheFinalByte) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 4);
  bmp.line(draw::point{10, 3}, draw::point{14, 3});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00111110_b   // [3]
                                       ));
}

TEST(Line, LongHorizontal) {
  auto [store, bmp] = draw::create_bitmap_and_store(24, 4);
  bmp.line(draw::point{2, 1}, draw::point{21, 1});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b, 0b00000000_b,  // [0]
                                       0b00111111_b, 0b11111111_b, 0b11111100_b,  // [1]
                                       0b00000000_b, 0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b, 0b00000000_b   // [3]
                                       ));
}

TEST(Line, OverLongHorizontal) {
  // The line end is too far in the x direction. Check that it is correctly clipped.
  auto [store, bmp] = draw::create_bitmap_and_store(16, 4);
  bmp.line(draw::point{2, 1}, draw::point{21, 1});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00111111_b, 0b11111111_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b   // [3]
                                       ));
}

TEST(Line, OverLongHorizontalLastRow) {
  // Similar to the OverLongHorizontal test in that the x-ordinate of the line end is too large. This checks that we do not write beyond the end of the bitmap storage vector.
  auto [store, bmp] = draw::create_bitmap_and_store(16, 4);
  bmp.line(draw::point{0, 3}, draw::point{21, 3});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b11111111_b, 0b11111111_b   // [3]
                                       ));
}

TEST(Line, HorizontalClippedXTooLarge) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 4);
  bmp.line(draw::point{16, 3}, draw::point{25, 3});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b   // [3]
                                       ));
}

TEST(Line, HorizontalClippedYTooLarge) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 4);
  bmp.line(draw::point{2, 4}, draw::point{11, 4});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b   // [3]
                                       ));
}

TEST(Line, Vertical) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 8);
  bmp.line(draw::point{2, 2}, draw::point{2, 5});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00100000_b, 0b00000000_b,  // [2]
                                       0b00100000_b, 0b00000000_b,  // [3]
                                       0b00100000_b, 0b00000000_b,  // [4]
                                       0b00100000_b, 0b00000000_b,  // [5]
                                       0b00000000_b, 0b00000000_b,  // [6]
                                       0b00000000_b, 0b00000000_b   // [7]
                                       ));
}

TEST(Line, LastVerticalColumn) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 8);
  bmp.line(draw::point{15, 2}, draw::point{15, 6});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000001_b,  // [2]
                                       0b00000000_b, 0b00000001_b,  // [3]
                                       0b00000000_b, 0b00000001_b,  // [4]
                                       0b00000000_b, 0b00000001_b,  // [5]
                                       0b00000000_b, 0b00000001_b,  // [6]
                                       0b00000000_b, 0b00000000_b   // [7]
                                       ));
}

TEST(Line, VerticalClippedXTooLarge) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 4);
  bmp.line(draw::point{16, 2}, draw::point{16, 6});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b   // [3]
                                       ));
}

TEST(Line, VerticalClippedYTooLarge) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 4);
  bmp.line(draw::point{1, 4}, draw::point{1, 10});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b   // [3]
                                       ));
}

TEST(Line, Diagonal1) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 4);
  bmp.line(draw::point{0, 0}, draw::point{15, 3});
  EXPECT_THAT(bmp.store(), ElementsAre(0b11100000_b, 0b00000000_b,  // [0]
                                       0b00011111_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b11111000_b,  // [2]
                                       0b00000000_b, 0b00000111_b   // [3]
                                       ));
}

TEST(Line, Diagonal2) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 4);
  bmp.line(draw::point{0, 3}, draw::point{15, 0});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000111_b,  // [0]
                                       0b00000000_b, 0b11111000_b,  // [1]
                                       0b00011111_b, 0b00000000_b,  // [2]
                                       0b11100000_b, 0b00000000_b   // [3]
                                       ));
}

} // end anonymous namespace
