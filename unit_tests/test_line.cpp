#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tuple>

#include "bitmap.hpp"

using testing::ElementsAre;

namespace {

consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}

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
