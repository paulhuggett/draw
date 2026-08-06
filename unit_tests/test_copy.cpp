//===- unit_tests/test_copy.cpp -------------------------------------------===//
//*                         *
//*   ___ ___  _ __  _   _  *
//*  / __/ _ \| '_ \| | | | *
//* | (_| (_) | |_) | |_| | *
//*  \___\___/| .__/ \__, | *
//*           |_|    |___/  *
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
#include "draw/mono_bitmap.hpp"
#include "draw/rgba_bitmap.hpp"

// Google test/mock/fuzz
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(DRAW_FUZZ_TESTS) && DRAW_FUZZ_TESTS
#include <fuzztest/fuzztest.h>
#endif

// Local include
#include "create_bitmap.hpp"
#include "rect.hpp"

namespace {

using testing::ElementsAre;
using testing::ElementsAreArray;
using namespace draw::literals;

std::tuple<std::vector<std::byte>, draw::mono_bitmap> create_pattern_filled_bitmap_and_store(std::uint16_t const width,
                                                                                             std::uint16_t const height,
                                                                                             draw::pattern const& pat) {
  auto [store, bmp] = create_bitmap_and_store(width, height);
  bmp.paint_rect(bmp.bounds(), draw::mono_pen{.pat = pat});
  return {std::move(store), std::move(bmp)};
}

std::tuple<std::vector<std::byte>, draw::mono_bitmap> create_black_filled_bitmap_and_store(std::uint16_t const width,
                                                                                           std::uint16_t const height) {
  return create_pattern_filled_bitmap_and_store(width, height, draw::black);
}

std::tuple<std::vector<std::byte>, draw::mono_bitmap> create_framed_bitmap_and_store(std::uint16_t const width,
                                                                                     std::uint16_t const height) {
  auto [store, bmp] = create_bitmap_and_store(width, height);
  bmp.frame_rect(bmp.bounds(), draw::mono_pen{.pat = draw::black});
  return std::make_tuple(std::move(store), std::move(bmp));
}

constexpr std::array empty{
    0b00000000_b,  // [0]
    0b00000000_b,  // [1]
    0b00000000_b,  // [2]
    0b00000000_b,  // [3]
    0b00000000_b,  // [4]
    0b00000000_b,  // [5]
    0b00000000_b,  // [6]
    0b00000000_b   // [7]
};

TEST(Copy, SmallerCopiedToTopLeft) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(5U, 4U);
  bmp.copy(bmp2, {.x = 0, .y = 0}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b11111000_b,  // [0]
                                       0b11111000_b,  // [1]
                                       0b11111000_b,  // [2]
                                       0b11111000_b,  // [3]
                                       0b00000000_b,  // [4]
                                       0b00000000_b,  // [5]
                                       0b00000000_b,  // [6]
                                       0b00000000_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 3, .right = 4}));
}

TEST(Copy, SmallerCopiedToMiddleCopyMode) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  // Fill the destination bitmap with gray.
  bmp.paint_rect(bmp.bounds(), draw::mono_pen{.pat = draw::gray});
  // Reset the dirty area.
  bmp.clean();

  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, {.x = 2, .y = 2}, draw::transfer_mode::mode_copy);

  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b,  // [0]
                                       0b01010101_b,  // [1]
                                       0b10111110_b,  // [2]
                                       0b01111101_b,  // [3]
                                       0b10111110_b,  // [4]
                                       0b01111101_b,  // [5]
                                       0b10101010_b,  // [6]
                                       0b01010101_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 2, .left = 2, .bottom = 5, .right = 5}));
}

TEST(Copy, SmallerCrossingDestinationBytesCopiedToMiddleCopyMode) {
  auto [store, bmp] = create_bitmap_and_store(16U, 8U);

  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, {.x = 6, .y = 2}, draw::transfer_mode::mode_copy);

  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000011_b, 0b11000000_b,  // [2]
                                       0b00000011_b, 0b11000000_b,  // [3]
                                       0b00000011_b, 0b11000000_b,  // [4]
                                       0b00000011_b, 0b11000000_b,  // [5]
                                       0b00000000_b, 0b00000000_b,  // [6]
                                       0b00000000_b, 0b00000000_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 2, .left = 6, .bottom = 5, .right = 9}));
}

TEST(Copy, SmallerCrossingDestinationBytesCopiedToCornerCopyMode) {
  auto [store, bmp] = create_bitmap_and_store(16U, 8U);

  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, {.x = 13, .y = 5}, draw::transfer_mode::mode_copy);

  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000000_b, 0b00000000_b,  // [3]
                                       0b00000000_b, 0b00000000_b,  // [4]
                                       0b00000000_b, 0b00000111_b,  // [5]
                                       0b00000000_b, 0b00000111_b,  // [6]
                                       0b00000000_b, 0b00000111_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 5, .left = 13, .bottom = 7, .right = 15}));
}

TEST(Copy, SmallerCopiedToMiddleOrMode) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  bmp.paint_rect(bmp.bounds(), draw::mono_pen{.pat = draw::gray});
  bmp.clean();

  auto [store2, bmp2] = create_bitmap_and_store(4U, 4U);
  bmp2.frame_rect(bmp2.bounds(), draw::mono_pen{});
  bmp2.clean();

  bmp.copy(bmp2, {.x = 2, .y = 2}, draw::transfer_mode::mode_or);
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b,  // [0]
                                       0b01010101_b,  // [1]
                                       0b10111110_b,  // [2]
                                       0b01110101_b,  // [3]
                                       0b10101110_b,  // [4]
                                       0b01111101_b,  // [5]
                                       0b10101010_b,  // [6]
                                       0b01010101_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 2, .left = 2, .bottom = 5, .right = 5}));
}

TEST(Copy, SmallerBitmapWithNegativeXAndPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, {.x = -2, .y = 0}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b11000000_b,  // [0]
                                       0b11000000_b,  // [1]
                                       0b11000000_b,  // [2]
                                       0b11000000_b,  // [3]
                                       0b00000000_b,  // [4]
                                       0b00000000_b,  // [5]
                                       0b00000000_b,  // [6]
                                       0b00000000_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 3, .right = 1}));
}

TEST(Copy, SmallerBitmapWithVeryNegativeX) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_bitmap_and_store(4U, 4U);
  bmp2.paint_rect(bmp2.bounds(), draw::mono_pen{.pat = draw::black});
  bmp.copy(bmp2, draw::point{.x = -8, .y = 0}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, SmallerBitmapWithPositiveXMakingItPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_bitmap_and_store(4U, 4U);
  bmp2.paint_rect(bmp2.bounds(), draw::mono_pen{.pat = draw::black});
  bmp.copy(bmp2, draw::point{.x = 6, .y = 0}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000011_b,  // [0]
                                       0b00000011_b,  // [1]
                                       0b00000011_b,  // [2]
                                       0b00000011_b,  // [3]
                                       0b00000000_b,  // [4]
                                       0b00000000_b,  // [5]
                                       0b00000000_b,  // [6]
                                       0b00000000_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 6, .bottom = 3, .right = 7}));
}

TEST(Copy, SmallerBitmapWithLargeX) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_bitmap_and_store(4U, 4U);
  bmp2.paint_rect(bmp2.bounds(), draw::mono_pen{.pat = draw::black});
  bmp.copy(bmp2, draw::point{.x = 10, .y = 0}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, SmallerBitmapWithNegativeYAndPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = -2}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b11110000_b,  // [0]
                                       0b11110000_b,  // [1]
                                       0b00000000_b,  // [2]
                                       0b00000000_b,  // [3]
                                       0b00000000_b,  // [4]
                                       0b00000000_b,  // [5]
                                       0b00000000_b,  // [6]
                                       0b00000000_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 1, .right = 3}));
}

TEST(Copy, SmallerBitmapWithVeryNegativeY) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = -10}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, SmallerBitmapWithYMakingItPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = 6}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b,  // [0]
                                       0b00000000_b,  // [1]
                                       0b00000000_b,  // [2]
                                       0b00000000_b,  // [3]
                                       0b00000000_b,  // [4]
                                       0b00000000_b,  // [5]
                                       0b11110000_b,  // [6]
                                       0b11110000_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 6, .left = 0, .bottom = 7, .right = 3}));
}

TEST(Copy, SmallerBitmapWithLargeY) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = 10}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, LargerCopiedToTopLeft) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16U, 16U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = 0}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b11111111_b,  // [0]
                                       0b11111111_b,  // [1]
                                       0b11111111_b,  // [2]
                                       0b11111111_b,  // [3]
                                       0b11111111_b,  // [4]
                                       0b11111111_b,  // [5]
                                       0b11111111_b,  // [6]
                                       0b11111111_b   // [7]
                                       ));
  EXPECT_EQ(bmp.bounds(), (draw::rect{.top = 0, .left = 0, .bottom = 7, .right = 7}));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 7, .right = 7}));
}

TEST(Copy, LargerBitmapWithNegativeXAndPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16U, 16U);
  bmp.copy(bmp2, draw::point{.x = -14, .y = 0}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b11000000_b,  // [0]
                                       0b11000000_b,  // [1]
                                       0b11000000_b,  // [2]
                                       0b11000000_b,  // [3]
                                       0b11000000_b,  // [4]
                                       0b11000000_b,  // [5]
                                       0b11000000_b,  // [6]
                                       0b11000000_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 7, .right = 1}));
}

TEST(Copy, LargerBitmapWithVeryNegativeX) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_bitmap_and_store(16U, 16U);
  bmp2.paint_rect(bmp2.bounds(), draw::mono_pen{.pat = draw::black});
  bmp.copy(bmp2, draw::point{.x = -24, .y = 0}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, LargerBitmapWithXMakingItPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_bitmap_and_store(16U, 16U);
  bmp2.paint_rect(bmp2.bounds(), draw::mono_pen{.pat = draw::black});
  bmp.copy(bmp2, draw::point{.x = 6, .y = 0}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000011_b,  // [0]
                                       0b00000011_b,  // [1]
                                       0b00000011_b,  // [2]
                                       0b00000011_b,  // [3]
                                       0b00000011_b,  // [4]
                                       0b00000011_b,  // [5]
                                       0b00000011_b,  // [6]
                                       0b00000011_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 6, .bottom = 7, .right = 7}));
}

TEST(Copy, LargerBitmapWithLargeX) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_bitmap_and_store(16U, 16U);
  bmp2.paint_rect(bmp2.bounds(), draw::mono_pen{.pat = draw::black});
  bmp.copy(bmp2, draw::point{.x = 20, .y = 0}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, LargerBitmapWithNegativeYAndPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16U, 16U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = -12}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b11111111_b,  // [0]
                                       0b11111111_b,  // [1]
                                       0b11111111_b,  // [2]
                                       0b11111111_b,  // [3]
                                       0b00000000_b,  // [4]
                                       0b00000000_b,  // [5]
                                       0b00000000_b,  // [6]
                                       0b00000000_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 3, .right = 7}));
}

TEST(Copy, LargerBitmapWithVeryNegativeY) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16U, 16U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = -20}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, LargerBitmapWithYMakingItPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16U, 16U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = 6}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b,  // [0]
                                       0b00000000_b,  // [1]
                                       0b00000000_b,  // [2]
                                       0b00000000_b,  // [3]
                                       0b00000000_b,  // [4]
                                       0b00000000_b,  // [5]
                                       0b11111111_b,  // [6]
                                       0b11111111_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 6, .left = 0, .bottom = 7, .right = 7}));
}

TEST(Copy, LargerBitmapWithLargeY) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16U, 16U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = 10}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
  EXPECT_EQ(bmp.dirty(), std::nullopt);
}

TEST(Copy, SmallerFramedTransferModeOr) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_framed_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = 0}, draw::transfer_mode::mode_or);
  EXPECT_THAT(bmp.store(), ElementsAre(0b11110000_b,  // [0]
                                       0b10010000_b,  // [1]
                                       0b10010000_b,  // [2]
                                       0b11110000_b,  // [3]
                                       0b00000000_b,  // [4]
                                       0b00000000_b,  // [5]
                                       0b00000000_b,  // [6]
                                       0b00000000_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 3, .right = 3}));
}

TEST(Copy, GrayWithSmallerFramedTransferModeCopy) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  bmp.paint_rect(bmp.bounds(), draw::mono_pen{.pat = draw::gray});
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b,  // [0]
                                       0b01010101_b,  // [1]
                                       0b10101010_b,  // [2]
                                       0b01010101_b,  // [3]
                                       0b10101010_b,  // [4]
                                       0b01010101_b,  // [5]
                                       0b10101010_b,  // [6]
                                       0b01010101_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 7, .right = 7}));
  auto [store2, bmp2] = create_framed_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = 0}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b11111010_b,  // [0]
                                       0b10010101_b,  // [1]
                                       0b10011010_b,  // [2]
                                       0b11110101_b,  // [3]
                                       0b10101010_b,  // [4]
                                       0b01010101_b,  // [5]
                                       0b10101010_b,  // [6]
                                       0b01010101_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 7, .right = 7}));
}
TEST(Copy, GrayWithSmallerFramedTransferModeOr) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  bmp.paint_rect(bmp.bounds(), draw::mono_pen{.pat = draw::gray});
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b,  // [0]
                                       0b01010101_b,  // [1]
                                       0b10101010_b,  // [2]
                                       0b01010101_b,  // [3]
                                       0b10101010_b,  // [4]
                                       0b01010101_b,  // [5]
                                       0b10101010_b,  // [6]
                                       0b01010101_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 7, .right = 7}));
  auto [store2, bmp2] = create_framed_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = 0}, draw::transfer_mode::mode_or);
  EXPECT_THAT(bmp.store(), ElementsAre(0b11111010_b,  // [0]
                                       0b11010101_b,  // [1]
                                       0b10111010_b,  // [2]
                                       0b11110101_b,  // [3]
                                       0b10101010_b,  // [4]
                                       0b01010101_b,  // [5]
                                       0b10101010_b,  // [6]
                                       0b01010101_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 7, .right = 7}));
}

TEST(Copy, CopyAlignedBytesTransferModeCopy) {
  constexpr auto bmp_width = 4U * 8U;
  auto [store, bmp] = create_bitmap_and_store(bmp_width, 8U);
  bmp.paint_rect(bmp.bounds(), draw::mono_pen{draw::gray});
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [0]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [1]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [2]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [3]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [4]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [5]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [6]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(),
            (draw::rect{.top = 0, .left = 0, .bottom = 7, .right = static_cast<draw::coordinate>(bmp_width) - 1}));
  auto [store2, bmp2] = create_framed_bitmap_and_store(2U * 8U, 4U);
  bmp.copy(bmp2, draw::point{.x = 8, .y = 2}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [0]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [1]
                                       0b10101010_b, 0b11111111_b, 0b11111111_b, 0b10101010_b,  // [2]
                                       0b01010101_b, 0b10000000_b, 0b00000001_b, 0b01010101_b,  // [3]
                                       0b10101010_b, 0b10000000_b, 0b00000001_b, 0b10101010_b,  // [4]
                                       0b01010101_b, 0b11111111_b, 0b11111111_b, 0b01010101_b,  // [5]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [6]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(),
            (draw::rect{.top = 0, .left = 0, .bottom = 7, .right = static_cast<draw::coordinate>(bmp_width) - 1}));
}

TEST(Copy, CopyAlignedBytesPartialRightEdgeTransferModeCopy) {
  constexpr auto bmp_width = 4U * 8U;
  auto [store, bmp] = create_bitmap_and_store(bmp_width, 8U);
  bmp.paint_rect(bmp.bounds(), draw::mono_pen{draw::gray});
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [0]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [1]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [2]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [3]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [4]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [5]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [6]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(),
            (draw::rect{.top = 0, .left = 0, .bottom = 7, .right = static_cast<draw::coordinate>(bmp_width) - 1}));
  auto [store2, bmp2] = create_framed_bitmap_and_store(2U * 8U - 4U, 4U);
  bmp.copy(bmp2, draw::point{.x = 8, .y = 2}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [0]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [1]
                                       0b10101010_b, 0b11111111_b, 0b11111010_b, 0b10101010_b,  // [2]
                                       0b01010101_b, 0b10000000_b, 0b00010101_b, 0b01010101_b,  // [3]
                                       0b10101010_b, 0b10000000_b, 0b00011010_b, 0b10101010_b,  // [4]
                                       0b01010101_b, 0b11111111_b, 0b11110101_b, 0b01010101_b,  // [5]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [6]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(),
            (draw::rect{.top = 0, .left = 0, .bottom = 7, .right = static_cast<draw::coordinate>(bmp_width) - 1}));
}

TEST(Copy, CopyMultipleAlignedBytesTransferModeCopy) {
  auto [store, bmp] = create_bitmap_and_store(32U, 6U);
  bmp.paint_rect(bmp.bounds(), draw::mono_pen{draw::gray});
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [0]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [1]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [2]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [3]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [4]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b   // [5]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 5, .right = 31}));
  auto [store2, bmp2] = create_framed_bitmap_and_store(24U, 4U);
  bmp.copy(bmp2, draw::point{.x = 3, .y = 1}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [0]
                                       0b01011111_b, 0b11111111_b, 0b11111111_b, 0b11110101_b,  // [1]
                                       0b10110000_b, 0b00000000_b, 0b00000000_b, 0b00101010_b,  // [2]
                                       0b01010000_b, 0b00000000_b, 0b00000000_b, 0b00110101_b,  // [3]
                                       0b10111111_b, 0b11111111_b, 0b11111111_b, 0b11101010_b,  // [4]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b   // [5]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 5, .right = 31}));
}

TEST(Copy, MisalignedTiny) {
  auto [store, bmp] = create_bitmap_and_store(16U, 1U);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b));
  EXPECT_EQ(bmp.dirty(), std::nullopt);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(8U, 1U);
  bmp.copy(bmp2, draw::point{.x = 2, .y = 0}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00111111_b, 0b11000000_b));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 2, .bottom = 0, .right = 9}));
}

TEST(Copy, MisalignedWideModeCopy) {
  auto [store, bmp] = create_bitmap_and_store(24U, 5U);
  bmp.paint_rect(bmp.bounds(), draw::mono_pen{draw::gray});
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 4, .right = 23}));
  auto [store2, bmp2] = create_framed_bitmap_and_store(16U, 3U);
  bmp.copy(bmp2, draw::point{.x = 2, .y = 1}, draw::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b,  // [0]
                                       0b01111111_b, 0b11111111_b, 0b11010101_b,  // [1]
                                       0b10100000_b, 0b00000000_b, 0b01101010_b,  // [2]
                                       0b01111111_b, 0b11111111_b, 0b11010101_b,  // [3]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b   // [4]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 4, .right = 23}));
}

TEST(Copy, MisalignedWideModeOr) {
  auto [store, bmp] = create_bitmap_and_store(24U, 5U);
  bmp.paint_rect(bmp.bounds(), draw::mono_pen{draw::gray});
  auto [store2, bmp2] = create_framed_bitmap_and_store(16U, 3U);
  bmp.copy(bmp2, draw::point{.x = 2, .y = 1}, draw::transfer_mode::mode_or);
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b,  // [0]
                                       0b01111111_b, 0b11111111_b, 0b11010101_b,  // [1]
                                       0b10101010_b, 0b10101010_b, 0b11101010_b,  // [2]
                                       0b01111111_b, 0b11111111_b, 0b11010101_b,  // [3]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b   // [4]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 4, .right = 23}));
}

TEST(Copy, CopyAlignedBytesTransferModeOr) {
  auto [store, bmp] = create_bitmap_and_store(4U * 8U, 8U);
  bmp.paint_rect(bmp.bounds(), draw::mono_pen{draw::gray});
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [0]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [1]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [2]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [3]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [4]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [5]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [6]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b   // [7]
                                       ));
  auto [store2, bmp2] = create_framed_bitmap_and_store(2U * 8U, 4U);
  bmp.copy(bmp2, draw::point{.x = 8, .y = 2}, draw::transfer_mode::mode_or);
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [0]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [1]
                                       0b10101010_b, 0b11111111_b, 0b11111111_b, 0b10101010_b,  // [2]
                                       0b01010101_b, 0b11010101_b, 0b01010101_b, 0b01010101_b,  // [3]
                                       0b10101010_b, 0b10101010_b, 0b10101011_b, 0b10101010_b,  // [4]
                                       0b01010101_b, 0b11111111_b, 0b11111111_b, 0b01010101_b,  // [5]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [6]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b   // [7]
                                       ));
}

class CopyToRGBA : public testing::Test {
protected:
  static constexpr auto red = draw::rgba{0xFF, 0x00, 0x00};
  static constexpr auto green = draw::rgba{0x00, 0xFF, 0x00};
  static constexpr auto R = draw::rgba_premult{red};
  static constexpr auto G = draw::rgba_premult{green};
  static constexpr auto B = draw::rgba_premult{draw::rgba{}};
};

TEST_F(CopyToRGBA, SmallerCopiedToTopLeft) {
  auto [dest_store, dest_bitmap] = create_bitmap32_and_store(4U, 4U);
  auto [src_store, src_bitmap] = create_black_filled_bitmap_and_store(2U, 2U);
  dest_bitmap.copy(src_bitmap, {.x = 0, .y = 0}, red, green);
  EXPECT_THAT(dest_bitmap.store(), ElementsAre(R, R, B, B,  // [0]
                                               R, R, B, B,  // [1]
                                               B, B, B, B,  // [2]
                                               B, B, B, B   // [3]
                                               ));
  EXPECT_EQ(dest_bitmap.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 1, .right = 1}));
}

TEST_F(CopyToRGBA, SmallerGrayCopiedToTopLeft) {
  auto [dest_store, dest_bitmap] = create_bitmap32_and_store(4U, 4U);
  auto [src_store, src_bitmap] = create_pattern_filled_bitmap_and_store(2U, 2U, draw::gray);
  dest_bitmap.copy(src_bitmap, {.x = 0, .y = 0}, red, green);
  EXPECT_THAT(dest_bitmap.store(), ElementsAre(R, G, B, B,  // [0]
                                               G, R, B, B,  // [1]
                                               B, B, B, B,  // [2]
                                               B, B, B, B   // [3]
                                               ));
  EXPECT_EQ(dest_bitmap.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 1, .right = 1}));
}

TEST_F(CopyToRGBA, SmallerCopiedToBottomRight) {
  auto [dest_store, dest_bitmap] = create_bitmap32_and_store(4U, 4U);
  auto [src_store, src_bitmap] = create_black_filled_bitmap_and_store(2U, 2U);
  dest_bitmap.copy(src_bitmap, {.x = 2, .y = 2}, red, green);
  EXPECT_THAT(dest_bitmap.store(), ElementsAre(B, B, B, B,  // [0]
                                               B, B, B, B,  // [1]
                                               B, B, R, R,  // [2]
                                               B, B, R, R   // [3]
                                               ));
  EXPECT_EQ(dest_bitmap.dirty(), (draw::rect{.top = 2, .left = 2, .bottom = 3, .right = 3}));
}

TEST_F(CopyToRGBA, SmallerGrayCopiedToBottomRight) {
  auto [dest_store, dest_bitmap] = create_bitmap32_and_store(4U, 4U);
  auto [src_store, src_bitmap] = create_pattern_filled_bitmap_and_store(2U, 2U, draw::gray);
  dest_bitmap.copy(src_bitmap, {.x = 2, .y = 2}, red, green);
  EXPECT_THAT(dest_bitmap.store(), ElementsAre(B, B, B, B,  // [0]
                                               B, B, B, B,  // [1]
                                               B, B, R, G,  // [2]
                                               B, B, G, R   // [3]
                                               ));
  EXPECT_EQ(dest_bitmap.dirty(), (draw::rect{.top = 2, .left = 2, .bottom = 3, .right = 3}));
}

TEST_F(CopyToRGBA, SmallerGrayCopiedToOverlapLeftEdge) {
  auto [dest_store, dest_bitmap] = create_bitmap32_and_store(4U, 4U);
  auto [src_store, src_bitmap] = create_pattern_filled_bitmap_and_store(2U, 2U, draw::gray);
  dest_bitmap.copy(src_bitmap, {.x = -1, .y = 1}, red, green);
  EXPECT_THAT(dest_bitmap.store(), ElementsAre(B, B, B, B,  // [0]
                                               G, B, B, B,  // [1]
                                               R, B, B, B,  // [2]
                                               B, B, B, B   // [3]
                                               ));
  EXPECT_EQ(dest_bitmap.dirty(), (draw::rect{.top = 1, .left = 0, .bottom = 2, .right = 0}));
}

TEST_F(CopyToRGBA, SmallerGrayCopiedPartiallyBeyondRightEdge) {
  auto [dest_store, dest_bitmap] = create_bitmap32_and_store(4U, 4U);
  auto [src_store, src_bitmap] = create_pattern_filled_bitmap_and_store(2U, 2U, draw::gray);
  dest_bitmap.copy(src_bitmap, {.x = 3, .y = 1}, red, green);
  EXPECT_THAT(dest_bitmap.store(), ElementsAre(B, B, B, B,  // [0]
                                               B, B, B, R,  // [1]
                                               B, B, B, G,  // [2]
                                               B, B, B, B   // [3]
                                               ));
  EXPECT_EQ(dest_bitmap.dirty(), (draw::rect{.top = 1, .left = 3, .bottom = 2, .right = 3}));
}

TEST_F(CopyToRGBA, SmallerGrayCopiedPartiallyBeyondTopEdge) {
  auto [dest_store, dest_bitmap] = create_bitmap32_and_store(4U, 4U);
  auto [src_store, src_bitmap] = create_pattern_filled_bitmap_and_store(2U, 2U, draw::gray);
  dest_bitmap.copy(src_bitmap, {.x = 1, .y = -1}, red, green);
  EXPECT_THAT(dest_bitmap.store(), ElementsAre(B, G, R, B,  // [0]
                                               B, B, B, B,  // [1]
                                               B, B, B, B,  // [2]
                                               B, B, B, B   // [3]
                                               ));
  EXPECT_EQ(dest_bitmap.dirty(), (draw::rect{.top = 0, .left = 1, .bottom = 0, .right = 2}));
}

TEST_F(CopyToRGBA, SmallerGrayCopiedPartiallyBeyondBottomEdge) {
  auto [dest_store, dest_bitmap] = create_bitmap32_and_store(4U, 4U);
  auto [src_store, src_bitmap] = create_pattern_filled_bitmap_and_store(2U, 2U, draw::gray);
  dest_bitmap.copy(src_bitmap, {.x = 1, .y = 3}, red, green);
  EXPECT_THAT(dest_bitmap.store(), ElementsAre(B, B, B, B,  // [0]
                                               B, B, B, B,  // [1]
                                               B, B, B, B,  // [2]
                                               B, R, G, B   // [3]
                                               ));
  EXPECT_EQ(dest_bitmap.dirty(), (draw::rect{.top = 3, .left = 1, .bottom = 3, .right = 2}));
}

TEST_F(CopyToRGBA, LargerGrayFullOverlap) {
  auto [dest_store, dest_bitmap] = create_bitmap32_and_store(4U, 4U);
  auto [src_store, src_bitmap] = create_pattern_filled_bitmap_and_store(8U, 8U, draw::gray);
  dest_bitmap.copy(src_bitmap, {.x = 0, .y = 0}, red, green);
  EXPECT_THAT(dest_bitmap.store(), ElementsAre(R, G, R, G,  // [0]
                                               G, R, G, R,  // [1]
                                               R, G, R, G,  // [2]
                                               G, R, G, R   // [3]
                                               ));
  EXPECT_EQ(dest_bitmap.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 3, .right = 3}));
}

TEST_F(CopyToRGBA, EightPixelWideSource) {
  auto [dest_store, dest_bitmap] = create_bitmap32_and_store(8U, 4U);
  auto [src_store, src_bitmap] = create_pattern_filled_bitmap_and_store(8U, 8U, draw::black);
  dest_bitmap.copy(src_bitmap, {.x = 0, .y = -4}, red, green);
  EXPECT_THAT(dest_bitmap.store(), ElementsAre(R, R, R, R, R, R, R, R,  // [0]
                                               R, R, R, R, R, R, R, R,  // [1]
                                               R, R, R, R, R, R, R, R,  // [2]
                                               R, R, R, R, R, R, R, R   // [3]
                                               ));
  EXPECT_EQ(dest_bitmap.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 3, .right = 7}));
}

void CopyMonoToMono(draw::point src_size, draw::point dest_size, draw::point pos) {
  if (constexpr auto valid_size = [](draw::point const size) constexpr { return size.x > 0 && size.y > 0; };
      !valid_size(src_size) || !valid_size(dest_size)) {
    return;
  }
  if (constexpr auto sensible_size = [](draw::point const size) constexpr { return size.x < 32 && size.y < 32; };
      !sensible_size(src_size) || !sensible_size(dest_size)) {
    return;
  }
  auto [src_store, src_bitmap] = create_pattern_filled_bitmap_and_store(
      static_cast<std::uint16_t>(src_size.x), static_cast<std::uint16_t>(src_size.y), draw::black);
  auto [dest_store, dest_bitmap] =
      create_bitmap_and_store(static_cast<std::uint16_t>(dest_size.x), static_cast<std::uint16_t>(dest_size.y));
  dest_bitmap.copy(src_bitmap, pos, draw::transfer_mode::mode_copy);
}
#if defined(DRAW_FUZZ_TESTS) && DRAW_FUZZ_TESTS
// NOLINTNEXTLINE
FUZZ_TEST(CopyMonoToMonoFuzz, CopyMonoToMono);
#endif
// NOLINTNEXTLINE
TEST(CopyMonoToMonoFuzz, Empty) {
  CopyMonoToMono({.x = 1, .y = 1}, {.x = 1, .y = 1}, {.x = 0, .y = 0});
}

void CopyMonoToRGBA(draw::point src_size, draw::point dest_size, draw::point pos) {
  if (constexpr auto valid_size = [](draw::point const size) constexpr { return size.x > 0 && size.y > 0; };
      !valid_size(src_size) || !valid_size(dest_size)) {
    return;
  }
  if (constexpr auto sensible_size = [](draw::point const size) constexpr { return size.x < 32 && size.y < 32; };
      !sensible_size(src_size) || !sensible_size(dest_size)) {
    return;
  }
  auto [src_store, src_bitmap] = create_pattern_filled_bitmap_and_store(
      static_cast<std::uint16_t>(src_size.x), static_cast<std::uint16_t>(src_size.y), draw::black);
  auto [dest_store, dest_bitmap] =
      create_bitmap32_and_store(static_cast<std::uint16_t>(dest_size.x), static_cast<std::uint16_t>(dest_size.y));
  dest_bitmap.copy(src_bitmap, pos, draw::rgba{0xFF, 0x00, 0x00}, draw::rgba{0x00, 0xFF, 0x00});
}

#if defined(DRAW_FUZZ_TESTS) && DRAW_FUZZ_TESTS
// NOLINTNEXTLINE
FUZZ_TEST(CopyToRGBAFuzz, CopyMonoToRGBA);
#endif
// NOLINTNEXTLINE
TEST(CopyToRGBAFuzz, Empty) {
  CopyMonoToRGBA({.x = 1, .y = 1}, {.x = 1, .y = 1}, {.x = 0, .y = 0});
}

}  // end anonymous namespace
