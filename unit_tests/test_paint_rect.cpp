//===- unit_tests/test_paint_rect.cpp -------------------------------------===//
//*              _       _                   _    *
//*  _ __   __ _(_)_ __ | |_   _ __ ___  ___| |_  *
//* | '_ \ / _` | | '_ \| __| | '__/ _ \/ __| __| *
//* | |_) | (_| | | | | | |_  | | |  __/ (__| |_  *
//* | .__/ \__,_|_|_| |_|\__| |_|  \___|\___|\__| *
//* |_|                                           *
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

TEST(Frame, AllInsideBlack) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 8);
  bmp.paint_rect(draw::rect{.top = 1, .left = 1, .bottom = 6, .right = 14}, draw::black);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b01111111_b, 0b11111110_b,  // [1]
                                       0b01111111_b, 0b11111110_b,  // [2]
                                       0b01111111_b, 0b11111110_b,  // [3]
                                       0b01111111_b, 0b11111110_b,  // [4]
                                       0b01111111_b, 0b11111110_b,  // [5]
                                       0b01111111_b, 0b11111110_b,  // [6]
                                       0b00000000_b, 0b00000000_b   // [7]
                                       ));
}

TEST(Frame, AllInsideGray) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 8);
  bmp.paint_rect(draw::rect{.top = 1, .left = 1, .bottom = 6, .right = 14}, draw::gray);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b01010101_b, 0b01010100_b,  // [1]
                                       0b00101010_b, 0b10101010_b,  // [2]
                                       0b01010101_b, 0b01010100_b,  // [3]
                                       0b00101010_b, 0b10101010_b,  // [4]
                                       0b01010101_b, 0b01010100_b,  // [5]
                                       0b00101010_b, 0b10101010_b,  // [6]
                                       0b00000000_b, 0b00000000_b   // [7]
                                       ));
}

TEST(Frame, Max) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 8);
  bmp.paint_rect(draw::rect{.top = 0, .left = 0, .bottom = 7, .right = 15}, draw::black);
  EXPECT_THAT(bmp.store(), ElementsAre(0b11111111_b, 0b11111111_b,  // [0]
                                       0b11111111_b, 0b11111111_b,  // [1]
                                       0b11111111_b, 0b11111111_b,  // [2]
                                       0b11111111_b, 0b11111111_b,  // [3]
                                       0b11111111_b, 0b11111111_b,  // [4]
                                       0b11111111_b, 0b11111111_b,  // [5]
                                       0b11111111_b, 0b11111111_b,  // [5]
                                       0b11111111_b, 0b11111111_b   // [7]
                                       ));
}

TEST(Frame, TooTall) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 8);
  bmp.paint_rect(draw::rect{.top = 1, .left = 1, .bottom = 8, .right = 14}, draw::black);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b01111111_b, 0b11111110_b,  // [1]
                                       0b01111111_b, 0b11111110_b,  // [2]
                                       0b01111111_b, 0b11111110_b,  // [3]
                                       0b01111111_b, 0b11111110_b,  // [4]
                                       0b01111111_b, 0b11111110_b,  // [5]
                                       0b01111111_b, 0b11111110_b,  // [6]
                                       0b01111111_b, 0b11111110_b   // [7]
                                       ));
}

TEST(Frame, TooWide) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 8);
  bmp.paint_rect(draw::rect{.top = 1, .left = 1, .bottom = 6, .right = 16}, draw::black);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b01111111_b, 0b11111111_b,  // [1]
                                       0b01111111_b, 0b11111111_b,  // [2]
                                       0b01111111_b, 0b11111111_b,  // [3]
                                       0b01111111_b, 0b11111111_b,  // [4]
                                       0b01111111_b, 0b11111111_b,  // [5]
                                       0b01111111_b, 0b11111111_b,  // [6]
                                       0b00000000_b, 0b00000000_b   // [7]
                                       ));
}

TEST(Frame, MinimumSize) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 4);
  bmp.paint_rect(draw::rect{.top = 1, .left = 1, .bottom = 1, .right = 1}, draw::black);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b,  // [0]
                                       0b01000000_b,  // [1]
                                       0b00000000_b,  // [2]
                                       0b00000000_b   // [3]
                                       ));
}

TEST(Frame, Empty) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 4);
  bmp.paint_rect(draw::rect{.top = 1, .left = 1, .bottom = 0, .right = 0}, draw::black);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b,  // [0]
                                       0b00000000_b,  // [1]
                                       0b00000000_b,  // [2]
                                       0b00000000_b   // [3]
                                       ));
}

TEST(Frame, NegativeLeft) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 4);
  bmp.paint_rect(draw::rect{.top = 0, .left = -10, .bottom = 4, .right = 2}, draw::black);
  EXPECT_THAT(bmp.store(), ElementsAre(0b11100000_b,  // [0]
                                       0b11100000_b,  // [1]
                                       0b11100000_b,  // [2]
                                       0b11100000_b   // [3]
                                       ));
}
TEST(Frame, NegativeLeftAndRight) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 4);
  bmp.paint_rect(draw::rect{.top = 0, .left = -10, .bottom = 4, .right = -5}, draw::black);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b,  // [0]
                                       0b00000000_b,  // [1]
                                       0b00000000_b,  // [2]
                                       0b00000000_b   // [3]
                                       ));
}

TEST(Frame, NegativeTop) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 4);
  bmp.paint_rect(draw::rect{.top = -10, .left = 0, .bottom = 2, .right = 2}, draw::black);
  EXPECT_THAT(bmp.store(), ElementsAre(0b11100000_b,  // [0]
                                       0b11100000_b,  // [1]
                                       0b11100000_b,  // [2]
                                       0b00000000_b   // [3]
                                       ));
}

TEST(Frame, NegativeTopAndBottom) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 4);
  bmp.paint_rect(draw::rect{.top = -10, .left = 0, .bottom = -5, .right = 2}, draw::black);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b,  // [0]
                                       0b00000000_b,  // [1]
                                       0b00000000_b,  // [2]
                                       0b00000000_b   // [3]
                                       ));
}

}  // end anonymous namespace
