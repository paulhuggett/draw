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
#include "draw/bitmap.hpp"

// Google test/mock
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Local include
#include "create_bitmap.hpp"
#include "rect.hpp"

namespace {

using testing::ElementsAre;
using testing::ElementsAreArray;
using namespace draw::literals;

std::tuple<std::vector<std::byte>, draw::bitmap> create_black_filled_bitmap_and_store(std::uint16_t width,
                                                                                      std::uint16_t height) {
  auto [store, bmp] = create_bitmap_and_store(width, height);
  bmp.paint_rect(bmp.bounds(), draw::black);
  return std::make_tuple(std::move(store), std::move(bmp));
}

std::tuple<std::vector<std::byte>, draw::bitmap> create_framed_bitmap_and_store(std::uint16_t width,
                                                                                std::uint16_t height) {
  auto [store, bmp] = create_bitmap_and_store(width, height);
  bmp.frame_rect(bmp.bounds());
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
  bmp.copy(bmp2, draw::point{.x = 0, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
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

TEST(Copy, SmallerCopiedToMiddle) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, draw::point{.x = 2, .y = 2}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b,  // [0]
                                       0b00000000_b,  // [1]
                                       0b00111100_b,  // [2]
                                       0b00111100_b,  // [3]
                                       0b00111100_b,  // [4]
                                       0b00111100_b,  // [5]
                                       0b00000000_b,  // [6]
                                       0b00000000_b   // [7]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 2, .left = 2, .bottom = 5, .right = 5}));
}

TEST(Copy, SmallerBitmapWithNegativeXAndPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, draw::point{.x = -2, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
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
  bmp2.paint_rect(bmp2.bounds(), draw::black);
  bmp.copy(bmp2, draw::point{.x = -8, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, SmallerBitmapWithPositiveXMakingItPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_bitmap_and_store(4U, 4U);
  bmp2.paint_rect(bmp2.bounds(), draw::black);
  bmp.copy(bmp2, draw::point{.x = 6, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
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
  bmp2.paint_rect(bmp2.bounds(), draw::black);
  bmp.copy(bmp2, draw::point{.x = 10, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, SmallerBitmapWithNegativeYAndPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = -2}, draw::bitmap::transfer_mode::mode_copy);
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
  bmp.copy(bmp2, draw::point{.x = 0, .y = -10}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, SmallerBitmapWithYMakingItPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = 6}, draw::bitmap::transfer_mode::mode_copy);
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
  bmp.copy(bmp2, draw::point{.x = 0, .y = 10}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, LargerCopiedToTopLeft) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16U, 16U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
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
  bmp.copy(bmp2, draw::point{.x = -14, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
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
  bmp2.paint_rect(bmp2.bounds(), draw::black);
  bmp.copy(bmp2, draw::point{.x = -24, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, LargerBitmapWithXMakingItPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_bitmap_and_store(16U, 16U);
  bmp2.paint_rect(bmp2.bounds(), draw::black);
  bmp.copy(bmp2, draw::point{.x = 6, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
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
  bmp2.paint_rect(bmp2.bounds(), draw::black);
  bmp.copy(bmp2, draw::point{.x = 20, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, LargerBitmapWithNegativeYAndPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16U, 16U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = -12}, draw::bitmap::transfer_mode::mode_copy);
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
  bmp.copy(bmp2, draw::point{.x = 0, .y = -20}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, LargerBitmapWithYMakingItPartiallyVisible) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16U, 16U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = 6}, draw::bitmap::transfer_mode::mode_copy);
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
  bmp.copy(bmp2, draw::point{.x = 0, .y = 10}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
  EXPECT_EQ(bmp.dirty(), std::nullopt);
}

TEST(Copy, SmallerFramedTransferModeOr) {
  auto [store, bmp] = create_bitmap_and_store(8U, 8U);
  auto [store2, bmp2] = create_framed_bitmap_and_store(4U, 4U);
  bmp.copy(bmp2, draw::point{.x = 0, .y = 0}, draw::bitmap::transfer_mode::mode_or);
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
  bmp.paint_rect(bmp.bounds(), draw::gray);
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
  bmp.copy(bmp2, draw::point{.x = 0, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
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
  bmp.paint_rect(bmp.bounds(), draw::gray);
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
  bmp.copy(bmp2, draw::point{.x = 0, .y = 0}, draw::bitmap::transfer_mode::mode_or);
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
  bmp.paint_rect(bmp.bounds(), draw::gray);
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
  bmp.copy(bmp2, draw::point{.x = 8, .y = 2}, draw::bitmap::transfer_mode::mode_copy);
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
  bmp.paint_rect(bmp.bounds(), draw::gray);
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
  bmp.copy(bmp2, draw::point{.x = 8, .y = 2}, draw::bitmap::transfer_mode::mode_copy);
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
  bmp.paint_rect(bmp.bounds(), draw::gray);
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [0]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [1]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [2]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [3]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [4]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b   // [5]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 5, .right = 31}));
  auto [store2, bmp2] = create_framed_bitmap_and_store(24U, 4U);
  bmp.copy(bmp2, draw::point{.x = 3, .y = 1}, draw::bitmap::transfer_mode::mode_copy);
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
  bmp.copy(bmp2, draw::point{.x = 2, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00111111_b, 0b11000000_b));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 2, .bottom = 0, .right = 9}));
}

TEST(Copy, MisalignedWideModeCopy) {
  auto [store, bmp] = create_bitmap_and_store(24U, 5U);
  bmp.paint_rect(bmp.bounds(), draw::gray);
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 4, .right = 23}));
  auto [store2, bmp2] = create_framed_bitmap_and_store(16U, 3U);
  bmp.copy(bmp2, draw::point{.x = 2, .y = 1}, draw::bitmap::transfer_mode::mode_copy);
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
  bmp.paint_rect(bmp.bounds(), draw::gray);
  auto [store2, bmp2] = create_framed_bitmap_and_store(16U, 3U);
  bmp.copy(bmp2, draw::point{.x = 2, .y = 1}, draw::bitmap::transfer_mode::mode_or);
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
  bmp.paint_rect(bmp.bounds(), draw::gray);
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
  bmp.copy(bmp2, draw::point{.x = 8, .y = 2}, draw::bitmap::transfer_mode::mode_or);
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

}  // end anonymous namespace
