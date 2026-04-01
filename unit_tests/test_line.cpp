//===- unit_tests/test_line.cpp -------------------------------------------===//
//*  _ _             *
//* | (_)_ __   ___  *
//* | | | '_ \ / _ \ *
//* | | | | | |  __/ *
//* |_|_|_| |_|\___| *
//*                  *
//===----------------------------------------------------------------------===//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
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
//===----------------------------------------------------------------------===//

// DUT
#include "draw/bitmap.hpp"

// Google test/mock
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Local include
#include "create_bitmap.hpp"
#include "rect.hpp"

using testing::ElementsAre;
using namespace draw::literals;

namespace {

TEST(Line, ShortHorizontal) {
  auto [store, bmp] = create_bitmap_and_store(16U, 8U);
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
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 5, .left = 2, .bottom = 5, .right = 11}));
}

TEST(Line, VeryShortHorizontal) {
  auto [store, bmp] = create_bitmap_and_store(16U, 8U);
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
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 5, .left = 2, .bottom = 5, .right = 6}));
}

TEST(Line, VeryShortHorizontalInTheFinalByte) {
  auto [store, bmp] = create_bitmap_and_store(16U, 4U);
  bmp.line(draw::point{10, 3}, draw::point{14, 3});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00111110_b   // [3]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 3, .left = 10, .bottom = 3, .right = 14}));
}

TEST(Line, LongHorizontal) {
  auto [store, bmp] = create_bitmap_and_store(24U, 4U);
  bmp.line(draw::point{2, 1}, draw::point{21, 1});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b, 0b00000000_b,  // [0]
                                       0b00111111_b, 0b11111111_b, 0b11111100_b,  // [1]
                                       0b00000000_b, 0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b, 0b00000000_b   // [3]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 1, .left = 2, .bottom = 1, .right = 21}));
}

TEST(Line, OverLongHorizontal) {
  // The line end is too far in the x direction. Check that it is correctly clipped.
  auto [store, bmp] = create_bitmap_and_store(16U, 4U);
  bmp.line(draw::point{2, 1}, draw::point{21, 1});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00111111_b, 0b11111111_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b   // [3]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 1, .left = 2, .bottom = 1, .right = 15}));
}

TEST(Line, OverLongHorizontalLastRow) {
  // Similar to the OverLongHorizontal test in that the x-ordinate of the line end is too large. This checks that we do
  // not write beyond the end of the bitmap storage vector.
  auto [store, bmp] = create_bitmap_and_store(16U, 4U);
  bmp.line(draw::point{0, 3}, draw::point{21, 3});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b11111111_b, 0b11111111_b   // [3]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 3, .left = 0, .bottom = 3, .right = 15}));
}

TEST(Line, HorizontalClippedXTooLarge) {
  auto [store, bmp] = create_bitmap_and_store(16U, 4U);
  bmp.line(draw::point{16, 3}, draw::point{25, 3});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b   // [3]
                                       ));
  EXPECT_EQ(bmp.dirty(), std::nullopt);
}

TEST(Line, HorizontalClippedYTooLarge) {
  auto [store, bmp] = create_bitmap_and_store(16U, 4U);
  bmp.line(draw::point{2, 4}, draw::point{11, 4});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b   // [3]
                                       ));
  EXPECT_EQ(bmp.dirty(), std::nullopt);
}

TEST(Line, Vertical) {
  auto [store, bmp] = create_bitmap_and_store(16U, 8U);
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
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 2, .left = 2, .bottom = 5, .right = 2}));
}

TEST(Line, LastVerticalColumn) {
  auto [store, bmp] = create_bitmap_and_store(16U, 8U);
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
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 2, .left = 15, .bottom = 6, .right = 15}));
}

TEST(Line, VerticalClippedXTooLarge) {
  auto [store, bmp] = create_bitmap_and_store(16U, 4U);
  bmp.line(draw::point{16, 2}, draw::point{16, 6});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b   // [3]
                                       ));
  EXPECT_EQ(bmp.dirty(), std::nullopt);
}

TEST(Line, VerticalClippedYTooLarge) {
  auto [store, bmp] = create_bitmap_and_store(16U, 4U);
  bmp.line(draw::point{1, 4}, draw::point{1, 10});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b   // [3]
                                       ));
  EXPECT_EQ(bmp.dirty(), std::nullopt);
}

TEST(Line, Diagonal1) {
  auto [store, bmp] = create_bitmap_and_store(16U, 4U);
  bmp.line(draw::point{0, 0}, draw::point{15, 3});
  EXPECT_THAT(bmp.store(), ElementsAre(0b11100000_b, 0b00000000_b,  // [0]
                                       0b00011111_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b11111000_b,  // [2]
                                       0b00000000_b, 0b00000111_b   // [3]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 3, .right = 15}));
}

TEST(Line, Diagonal2) {
  auto [store, bmp] = create_bitmap_and_store(16U, 4U);
  bmp.line(draw::point{0, 3}, draw::point{15, 0});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000111_b,  // [0]
                                       0b00000000_b, 0b11111000_b,  // [1]
                                       0b00011111_b, 0b00000000_b,  // [2]
                                       0b11100000_b, 0b00000000_b   // [3]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 3, .right = 15}));
}

}  // end anonymous namespace
