#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "bitmap.hpp"

using testing::ElementsAreArray;
using namespace testing;

namespace {

consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}

std::vector<std::byte> get_bitmap_vector(draw::bitmap const& bmp) {
  auto const store = bmp.store();
  return {std::begin(store), std::end(store)};
}
TEST(Line, ShortHorizontal) {
  draw::bitmap bmp{16U, 8U};
  bmp.line(draw::point{2, 5}, draw::point{11, 5});
  std::array const expected{
    0b00000000_b, 0b00000000_b, // [0]
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b, // [4]
    0b00111111_b, 0b11110000_b, // [5]
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b,
  };
  ASSERT_EQ(bmp.store().size(), expected.size());
  EXPECT_THAT(get_bitmap_vector(bmp), ElementsAreArray(expected));
}

#if 0
TEST(Line, VeryShortHorizontal) {
  draw::bitmap bmp{16U, 8U};
  bmp.line(draw::point{2, 5}, draw::point{6, 5});
  std::array const expected{
    0b00000000_b, 0b00000000_b, // [0]
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b, // [4]
    0b00111110_b, 0b00000000_b, // [5]
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b,
  };
  auto store = bmp.store();
  ASSERT_EQ(store.size(), expected.size());
  std::vector<std::byte> actual{store.begin(), store.end()};
  EXPECT_THAT(actual, ElementsAreArray(expected));
}
#endif

TEST(Line, LongHorizontal) {
  draw::bitmap bmp{24U, 4U};
  bmp.line(draw::point{2, 1}, draw::point{21, 1});
  std::array const expected{
    0b00000000_b, 0b00000000_b, 0b00000000_b, // [0]
    0b00111111_b, 0b11111111_b, 0b11111100_b, // [1]
    0b00000000_b, 0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b, 0b00000000_b,
  };
  ASSERT_EQ(bmp.store().size(), expected.size());
  EXPECT_THAT(get_bitmap_vector(bmp), ElementsAreArray(expected));
}

TEST(Line, OverLongHorizontal) {
  // The line end is too far in the x direction. Check that it is correctly clipped.
  draw::bitmap bmp{16U, 4U};
  bmp.line(draw::point{2, 1}, draw::point{21, 1});
  std::array const expected{
    0b00000000_b, 0b00000000_b, // [0]
    0b00111111_b, 0b11111111_b, // [1]
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b,
  };
  ASSERT_EQ(bmp.store().size(), expected.size());
  EXPECT_THAT(get_bitmap_vector(bmp), ElementsAreArray(expected));
}

TEST(Line, OverLongHorizontalLastRow) {
  // Similar to the OverLongHorizontal test in that the x-ordinate of the line end is too large. This checks that we do not write beyond the end of the bitmap storage vector.
  draw::bitmap bmp{16U, 4U};
  bmp.line(draw::point{0, 3}, draw::point{21, 3});
  std::array const expected{
    0b00000000_b, 0b00000000_b, // [0]
    0b00000000_b, 0b00000000_b, // [1]
    0b00000000_b, 0b00000000_b, // [2]
    0b11111111_b, 0b11111111_b, // [3]
  };
  ASSERT_EQ(bmp.store().size(), expected.size());
  EXPECT_THAT(get_bitmap_vector(bmp), ElementsAreArray(expected));
}

TEST(Line, HorizontalClippedXTooLarge) {
  draw::bitmap bmp{16U, 4U};
  bmp.line(draw::point{16, 3}, draw::point{25, 3});
  std::array const expected{
    0b00000000_b, 0b00000000_b, // [0]
    0b00000000_b, 0b00000000_b, // [1]
    0b00000000_b, 0b00000000_b, // [2]
    0b00000000_b, 0b00000000_b, // [3]
  };
  ASSERT_EQ(bmp.store().size(), expected.size());
  EXPECT_THAT(get_bitmap_vector(bmp), ElementsAreArray(expected));
}

TEST(Line, HorizontalClippedYTooLarge) {
  draw::bitmap bmp{16U, 4U};
  bmp.line(draw::point{2, 4}, draw::point{11, 4});
  std::array const expected{
    0b00000000_b, 0b00000000_b, // [0]
    0b00000000_b, 0b00000000_b, // [1]
    0b00000000_b, 0b00000000_b, // [2]
    0b00000000_b, 0b00000000_b, // [3]
  };
  ASSERT_EQ(bmp.store().size(), expected.size());
  EXPECT_THAT(get_bitmap_vector(bmp), ElementsAreArray(expected));
}

TEST(Line, Vertical) {
  draw::bitmap bmp{16U, 8U};
  bmp.line(draw::point{2, 2}, draw::point{2, 5});
  std::array const expected{
    0b00000000_b, 0b00000000_b, // [0]
    0b00000000_b, 0b00000000_b,
    0b00100000_b, 0b00000000_b,
    0b00100000_b, 0b00000000_b,
    0b00100000_b, 0b00000000_b, // [4]
    0b00100000_b, 0b00000000_b, // [5]
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b,
  };
  ASSERT_EQ(bmp.store().size(), expected.size());
  EXPECT_THAT(get_bitmap_vector(bmp), ElementsAreArray(expected));
}

TEST(Line, LastVerticalColumn) {
  draw::bitmap bmp{16U, 8U};
  bmp.line(draw::point{15, 2}, draw::point{15, 6});
  std::array const expected{
    0b00000000_b, 0b00000000_b, // [0]
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000001_b,
    0b00000000_b, 0b00000001_b,
    0b00000000_b, 0b00000001_b, // [4]
    0b00000000_b, 0b00000001_b, // [5]
    0b00000000_b, 0b00000001_b, // [6]
    0b00000000_b, 0b00000000_b, // [7]
  };
  ASSERT_EQ(bmp.store().size(), expected.size());
  EXPECT_THAT(get_bitmap_vector(bmp), ElementsAreArray(expected));
}

TEST(Line, VerticalClippedXTooLarge) {
  draw::bitmap bmp{16U, 4U};
  bmp.line(draw::point{16, 2}, draw::point{16, 6});
  std::array const expected{
    0b00000000_b, 0b00000000_b, // [0]
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b,
  };
  ASSERT_EQ(bmp.store().size(), expected.size());
  EXPECT_THAT(get_bitmap_vector(bmp), ElementsAreArray(expected));
}

TEST(Line, VerticalClippedYTooLarge) {
  draw::bitmap bmp{16U, 4U};
  bmp.line(draw::point{1, 4}, draw::point{1, 10});
  std::array const expected{
    0b00000000_b, 0b00000000_b, // [0]
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b,
    0b00000000_b, 0b00000000_b,
  };
  ASSERT_EQ(bmp.store().size(), expected.size());
  EXPECT_THAT(get_bitmap_vector(bmp), ElementsAreArray(expected));
}

TEST(Line, Diagonal1) {
  draw::bitmap bmp{16U, 4U};
  bmp.line(draw::point{0, 0}, draw::point{15, 3});
  std::array const expected{
    0b11100000_b, 0b00000000_b, // [0]
    0b00011111_b, 0b00000000_b,
    0b00000000_b, 0b11111000_b,
    0b00000000_b, 0b00000111_b,
  };
  ASSERT_EQ(bmp.store().size(), expected.size());
  EXPECT_THAT(get_bitmap_vector(bmp), ElementsAreArray(expected));
}

TEST(Line, Diagonal2) {
  draw::bitmap bmp{16U, 4U};
  bmp.line(draw::point{0, 3}, draw::point{15, 0});
  std::array const expected{
    0b00000000_b, 0b00000111_b, // [0]
    0b00000000_b, 0b11111000_b,
    0b00011111_b, 0b00000000_b,
    0b11100000_b, 0b00000000_b,
  };
  ASSERT_EQ(bmp.store().size(), expected.size());
  EXPECT_THAT(get_bitmap_vector(bmp), ElementsAreArray(expected));
}

} // end anonymous namespace
