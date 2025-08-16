#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "draw/bitmap.hpp"

using testing::ElementsAre;
using testing::ElementsAreArray;
using namespace draw::literals;

namespace {

std::tuple<std::unique_ptr<std::byte[]>, draw::bitmap> create_black_filled_bitmap_and_store(std::uint16_t width,
                                                                                            std::uint16_t height) {
  auto [store, bmp] = draw::create_bitmap_and_store(width, height);
  bmp.paint_rect(bmp.bounds(), draw::black);
  return std::make_tuple(std::move(store), std::move(bmp));
}

std::tuple<std::unique_ptr<std::byte[]>, draw::bitmap> create_framed_bitmap_and_store(std::uint16_t width,
                                                                                      std::uint16_t height) {
  auto [store, bmp] = draw::create_bitmap_and_store(width, height);
  auto r = bmp.bounds();
  --r.right;
  --r.bottom;
  bmp.frame_rect(r);
  return std::make_tuple(std::move(store), std::move(bmp));
}

std::array const empty{
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
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(5, 4);
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
}

TEST(Copy, SmallerCopiedToMiddle) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4, 4);
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
}

TEST(Copy, SmallerBitmapWithNegativeXAndPartiallyVisible) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4, 4);
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
}

TEST(Copy, SmallerBitmapWithVeryNegativeX) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = draw::create_bitmap_and_store(4, 4);
  bmp2.paint_rect(bmp2.bounds(), draw::black);
  bmp.copy(bmp2, draw::point{.x = -8, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, SmallerBitmapWithXMakingItPartiallyVisible) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = draw::create_bitmap_and_store(4, 4);
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
}

TEST(Copy, SmallerBitmapWithLargeX) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = draw::create_bitmap_and_store(4, 4);
  bmp2.paint_rect(bmp2.bounds(), draw::black);
  bmp.copy(bmp2, draw::point{.x = 10, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, SmallerBitmapWithNegativeYAndPartiallyVisible) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4, 4);
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
}

TEST(Copy, SmallerBitmapWithVeryNegativeY) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4, 4);
  bmp.copy(bmp2, draw::point{.x = 0, .y = -10}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, SmallerBitmapWithYMakingItPartiallyVisible) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4, 4);
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
}

TEST(Copy, SmallerBitmapWithLargeY) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(4, 4);
  bmp.copy(bmp2, draw::point{.x = 0, .y = 10}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, LargerCopiedToTopLeft) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16, 16);
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
}

TEST(Copy, LargerBitmapWithNegativeXAndPartiallyVisible) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16, 16);
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
}
TEST(Copy, LargerBitmapWithVeryNegativeX) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = draw::create_bitmap_and_store(16, 16);
  bmp2.paint_rect(bmp2.bounds(), draw::black);
  bmp.copy(bmp2, draw::point{.x = -24, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, LargerBitmapWithXMakingItPartiallyVisible) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = draw::create_bitmap_and_store(16, 16);
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
}

TEST(Copy, LargerBitmapWithLargeX) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = draw::create_bitmap_and_store(16, 16);
  bmp2.paint_rect(bmp2.bounds(), draw::black);
  bmp.copy(bmp2, draw::point{.x = 20, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, LargerBitmapWithNegativeYAndPartiallyVisible) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16, 16);
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
}

TEST(Copy, LargerBitmapWithVeryNegativeY) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16, 16);
  bmp.copy(bmp2, draw::point{.x = 0, .y = -20}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, LargerBitmapWithYMakingItPartiallyVisible) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16, 16);
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
}

TEST(Copy, LargerBitmapWithLargeY) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(16, 16);
  bmp.copy(bmp2, draw::point{.x = 0, .y = 10}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAreArray(empty));
}

TEST(Copy, SmallerFramedTranferModeOr) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
  auto [store2, bmp2] = create_framed_bitmap_and_store(4, 4);
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
}

TEST(Copy, GrayWithSmallerFramedTranferModeCopy) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
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
  auto [store2, bmp2] = create_framed_bitmap_and_store(4, 4);
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
}
TEST(Copy, GrayWithSmallerFramedTranferModeOr) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 8);
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
  auto [store2, bmp2] = create_framed_bitmap_and_store(4, 4);
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
}

TEST(Copy, CopyAlignedBytesTransferModeCopy) {
  auto [store, bmp] = draw::create_bitmap_and_store(4 * 8, 8);
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
  auto [store2, bmp2] = create_framed_bitmap_and_store(2 * 8, 4);
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
}

TEST(Copy, CopyAlignedBytesPartialRightEdgeTransferModeCopy) {
  auto [store, bmp] = draw::create_bitmap_and_store(4 * 8, 8);
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
  auto [store2, bmp2] = create_framed_bitmap_and_store(2 * 8 - 4, 4);
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
}

TEST(Copy, CopyMultipleAlignedBytesTransferModeCopy) {
  auto [store, bmp] = draw::create_bitmap_and_store(32, 6);
  bmp.paint_rect(bmp.bounds(), draw::gray);
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [0]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [1]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [2]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b,  // [3]
                                       0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [4]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b   // [5]
                                       ));
  auto [store2, bmp2] = create_framed_bitmap_and_store(24, 4);
  bmp.copy(bmp2, draw::point{.x = 3, .y = 1}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b, 0b10101010_b,  // [0]
                                       0b01011111_b, 0b11111111_b, 0b11111111_b, 0b11110101_b,  // [1]
                                       0b10110000_b, 0b00000000_b, 0b00000000_b, 0b00101010_b,  // [2]
                                       0b01010000_b, 0b00000000_b, 0b00000000_b, 0b00110101_b,  // [3]
                                       0b10111111_b, 0b11111111_b, 0b11111111_b, 0b11101010_b,  // [4]
                                       0b01010101_b, 0b01010101_b, 0b01010101_b, 0b01010101_b   // [5]
                                       ));
}

TEST(Copy, MisalignedTiny) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 1);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b));
  auto [store2, bmp2] = create_black_filled_bitmap_and_store(8, 1);
  bmp.copy(bmp2, draw::point{.x = 2, .y = 0}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b00111111_b, 0b11000000_b));
}

TEST(Copy, MisalignedWideModeCopy) {
  auto [store, bmp] = draw::create_bitmap_and_store(24, 5);
  bmp.paint_rect(bmp.bounds(), draw::gray);
  auto [store2, bmp2] = create_framed_bitmap_and_store(16, 3);
  bmp.copy(bmp2, draw::point{.x = 2, .y = 1}, draw::bitmap::transfer_mode::mode_copy);
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b,
                                       0b01111111_b, 0b11111111_b, 0b11010101_b,
                                       0b10100000_b, 0b00000000_b, 0b01101010_b,
                                       0b01111111_b, 0b11111111_b, 0b11010101_b,
                                       0b10101010_b, 0b10101010_b, 0b10101010_b
                                       ));
}

TEST(Copy, MisalignedWideModeOr) {
  auto [store, bmp] = draw::create_bitmap_and_store(24, 5);
  bmp.paint_rect(bmp.bounds(), draw::gray);
  auto [store2, bmp2] = create_framed_bitmap_and_store(16, 3);
  bmp.copy(bmp2, draw::point{.x = 2, .y = 1}, draw::bitmap::transfer_mode::mode_or);
  EXPECT_THAT(bmp.store(), ElementsAre(0b10101010_b, 0b10101010_b, 0b10101010_b,
                                       0b01111111_b, 0b11111111_b, 0b11010101_b,
                                       0b10101010_b, 0b10101010_b, 0b11101010_b,
                                       0b01111111_b, 0b11111111_b, 0b11010101_b,
                                       0b10101010_b, 0b10101010_b, 0b10101010_b
                                       ));
}

TEST(Copy, CopyAlignedBytesTransferModeOr) {
  auto [store, bmp] = draw::create_bitmap_and_store(4 * 8, 8);
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
  auto [store2, bmp2] = create_framed_bitmap_and_store(2 * 8, 4);
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
