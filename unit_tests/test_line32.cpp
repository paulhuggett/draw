//===- unit_tests/test_line32.cpp -----------------------------------------===//
//* _ _            _________    *
//* | (_)_ __   ___|___ /___ \  *
//* | | | '_ \ / _ \ |_ \ __) | *
//* | | | | | |  __/___) / __/  *
//* |_|_|_| |_|\___|____/_____| *
//*                             *
//===----------------------------------------------------------------------===//
// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
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
#include "draw/bitmap32.hpp"

// Google test/mock
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Local include
#include "create_bitmap.hpp"
#include "rect.hpp"

using testing::ElementsAre;
using namespace draw::literals;

namespace {

constexpr auto red = draw::rgba{.r = 0xFF, .g = 0x00, .b = 0x00};
constexpr auto r = draw::rgba_premult{red};
constexpr auto x = draw::rgba_premult{};

TEST(Line32, Horizontal) {
  auto [store, bmp] = create_bitmap32_and_store(4U, 3U);
  bmp.line(draw::point{1, 1}, draw::point{3, 1}, red);
  EXPECT_THAT(bmp.store(), ElementsAre(x, x, x, x,  // [0]
                                       x, r, r, r,  // [1]
                                       x, x, x, x   // [2]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 1, .left = 1, .bottom = 1, .right = 3}));
}

TEST(Line32, HorizontalReverse) {
  auto [store, bmp] = create_bitmap32_and_store(4U, 3U);
  bmp.line(draw::point{3, 1}, draw::point{1, 1}, red);
  EXPECT_THAT(bmp.store(), ElementsAre(x, x, x, x,  // [0]
                                       x, r, r, r,  // [1]
                                       x, x, x, x   // [2]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 1, .left = 1, .bottom = 1, .right = 3}));
}

TEST(Line32, OverLongHorizontal) {
  // The line end is too far in the x direction. Check that it is correctly clipped.
  auto [store, bmp] = create_bitmap32_and_store(4U, 3U);
  bmp.line(draw::point{2, 1}, draw::point{21, 1}, red);
  EXPECT_THAT(bmp.store(), ElementsAre(x, x, x, x,  // [0]
                                       x, x, r, r,  // [1]
                                       x, x, x, x   // [2]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 1, .left = 2, .bottom = 1, .right = 3}));
}

TEST(Line32, OverLongHorizontalLastRow) {
  // Similar to the OverLongHorizontal test in that the x-ordinate of the line end is too large. This checks that we do
  // not write beyond the end of the bitmap storage vector.
  auto [store, bmp] = create_bitmap32_and_store(4U, 3U);
  bmp.line(draw::point{0, 2}, draw::point{4, 2}, red);
  EXPECT_THAT(bmp.store(), ElementsAre(x, x, x, x,  // [0]
                                       x, x, x, x,  // [1]
                                       r, r, r, r   // [2]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 2, .left = 0, .bottom = 2, .right = 3}));
}

TEST(Line32, HorizontalClippedXTooLarge) {
  auto [store, bmp] = create_bitmap32_and_store(4U, 3U);
  bmp.line(draw::point{5, 1}, draw::point{25, 1}, red);
  EXPECT_THAT(bmp.store(), ElementsAre(x, x, x, x,  // [0]
                                       x, x, x, x,  // [1]
                                       x, x, x, x   // [2]
                                       ));
  EXPECT_EQ(bmp.dirty(), std::nullopt);
}

TEST(Line32, HorizontalClippedYTooLarge) {
  auto [store, bmp] = create_bitmap32_and_store(4U, 3U);
  bmp.line(draw::point{0, 4}, draw::point{3, 4}, red);
  EXPECT_THAT(bmp.store(), ElementsAre(x, x, x, x,  // [0]
                                       x, x, x, x,  // [1]
                                       x, x, x, x   // [2]
                                       ));
  EXPECT_EQ(bmp.dirty(), std::nullopt);
}

TEST(Line32, Vertical) {
  auto [store, bmp] = create_bitmap32_and_store(4U, 5U);
  bmp.line(draw::point{2, 1}, draw::point{2, 3}, red);
  EXPECT_THAT(bmp.store(), ElementsAre(x, x, x, x,  // [0]
                                       x, x, r, x,  // [1]
                                       x, x, r, x,  // [2]
                                       x, x, r, x,  // [3]
                                       x, x, x, x   // [4]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 1, .left = 2, .bottom = 3, .right = 2}));
}

TEST(Line32, VerticalReversed) {
  auto [store, bmp] = create_bitmap32_and_store(4U, 5U);
  bmp.line(draw::point{2, 3}, draw::point{2, 1}, red);
  EXPECT_THAT(bmp.store(), ElementsAre(x, x, x, x,  // [0]
                                       x, x, r, x,  // [1]
                                       x, x, r, x,  // [2]
                                       x, x, r, x,  // [3]
                                       x, x, x, x   // [4]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 1, .left = 2, .bottom = 3, .right = 2}));
}

TEST(Line32, LastVerticalColumn) {
  auto [store, bmp] = create_bitmap32_and_store(4U, 5U);
  bmp.line(draw::point{3, 1}, draw::point{3, 3}, red);
  EXPECT_THAT(bmp.store(), ElementsAre(x, x, x, x,  // [0]
                                       x, x, x, r,  // [1]
                                       x, x, x, r,  // [2]
                                       x, x, x, r,  // [3]
                                       x, x, x, x   // [4]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 1, .left = 3, .bottom = 3, .right = 3}));
}

TEST(Line32, VerticalClippedXTooLarge) {
  auto [store, bmp] = create_bitmap32_and_store(4U, 5U);
  bmp.line(draw::point{16, 2}, draw::point{16, 6}, red);
  EXPECT_THAT(bmp.store(), ElementsAre(x, x, x, x,  // [0]
                                       x, x, x, x,  // [1]
                                       x, x, x, x,  // [2]
                                       x, x, x, x,  // [3]
                                       x, x, x, x   // [4]
                                       ));
  EXPECT_EQ(bmp.dirty(), std::nullopt);
}

TEST(Line32, VerticalClippedYTooLarge) {
  auto [store, bmp] = create_bitmap32_and_store(4U, 5U);
  bmp.line(draw::point{1, 5}, draw::point{1, 10}, red);
  EXPECT_THAT(bmp.store(), ElementsAre(x, x, x, x,  // [0]
                                       x, x, x, x,  // [1]
                                       x, x, x, x,  // [2]
                                       x, x, x, x,  // [3]
                                       x, x, x, x   // [4]
                                       ));
  EXPECT_EQ(bmp.dirty(), std::nullopt);
}

TEST(Line32, Diagonal1) {
  auto [store, bmp] = create_bitmap32_and_store(4U, 5U);
  bmp.line(draw::point{0, 0}, draw::point{3, 4}, red);
  EXPECT_THAT(bmp.store(), ElementsAre(r, x, x, x,  // [0]
                                       x, r, x, x,  // [1]
                                       x, x, r, x,  // [2]
                                       x, x, r, x,  // [3]
                                       x, x, x, r   // [4]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 4, .right = 3}));
}

TEST(Line32, Diagonal2) {
  auto [store, bmp] = create_bitmap32_and_store(4U, 5U);
  bmp.line(draw::point{3, 4}, draw::point{0, 0}, red);
  EXPECT_THAT(bmp.store(), ElementsAre(r, x, x, x,  // [0]
                                       x, r, x, x,  // [1]
                                       x, r, x, x,  // [2]
                                       x, x, r, x,  // [3]
                                       x, x, x, r   // [4]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 4, .right = 3}));
}

}  // end anonymous namespace
