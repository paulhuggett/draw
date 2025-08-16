#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tuple>

#include "draw/bitmap.hpp"

using testing::ElementsAre;
using namespace draw::literals;

namespace {

TEST(Frame, AllInside) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 8);
  bmp.frame_rect(draw::rect{.top = 1, .left = 1, .bottom = 6, .right = 14});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b01111111_b, 0b11111110_b,  // [1]
                                       0b01000000_b, 0b00000010_b,  // [2]
                                       0b01000000_b, 0b00000010_b,  // [3]
                                       0b01000000_b, 0b00000010_b,  // [4]
                                       0b01000000_b, 0b00000010_b,  // [5]
                                       0b01111111_b, 0b11111110_b,  // [6]
                                       0b00000000_b, 0b00000000_b   // [7]
                                       ));
}

TEST(Frame, Max) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 8);
  bmp.frame_rect(draw::rect{.top = 0, .left = 0, .bottom = 7, .right = 15});
  EXPECT_THAT(bmp.store(), ElementsAre(0b11111111_b, 0b11111111_b,  // [0]
                                       0b10000000_b, 0b00000001_b,  // [1]
                                       0b10000000_b, 0b00000001_b,  // [2]
                                       0b10000000_b, 0b00000001_b,  // [3]
                                       0b10000000_b, 0b00000001_b,  // [4]
                                       0b10000000_b, 0b00000001_b,  // [5]
                                       0b10000000_b, 0b00000001_b,  // [5]
                                       0b11111111_b, 0b11111111_b   // [7]
                                       ));
}

TEST(Frame, TooTall) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 8);
  bmp.frame_rect(draw::rect{.top = 1, .left = 1, .bottom = 8, .right = 14});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b01111111_b, 0b11111110_b,  // [1]
                                       0b01000000_b, 0b00000010_b,  // [2]
                                       0b01000000_b, 0b00000010_b,  // [3]
                                       0b01000000_b, 0b00000010_b,  // [4]
                                       0b01000000_b, 0b00000010_b,  // [5]
                                       0b01000000_b, 0b00000010_b,  // [6]
                                       0b01000000_b, 0b00000010_b   // [7]
                                       ));
}

TEST(Frame, TooWide) {
  auto [store, bmp] = draw::create_bitmap_and_store(16, 8);
  bmp.frame_rect(draw::rect{.top = 1, .left = 1, .bottom = 6, .right = 16});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b01111111_b, 0b11111111_b,  // [1]
                                       0b01000000_b, 0b00000000_b,  // [2]
                                       0b01000000_b, 0b00000000_b,  // [3]
                                       0b01000000_b, 0b00000000_b,  // [4]
                                       0b01000000_b, 0b00000000_b,  // [5]
                                       0b01111111_b, 0b11111111_b,  // [6]
                                       0b00000000_b, 0b00000000_b   // [7]
                                       ));
}

TEST(Frame, MinimumSize) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 4);
  bmp.frame_rect(draw::rect{.top = 1, .left = 1, .bottom = 1, .right = 1});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b,  // [0]
                                       0b01000000_b,  // [1]
                                       0b00000000_b,  // [2]
                                       0b00000000_b   // [3]
                                       ));
}

TEST(Frame, Empty) {
  auto [store, bmp] = draw::create_bitmap_and_store(8, 4);
  bmp.frame_rect(draw::rect{.top = 1, .left = 1, .bottom = 0, .right = 0});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b,  // [0]
                                       0b00000000_b,  // [1]
                                       0b00000000_b,  // [2]
                                       0b00000000_b   // [3]
                                       ));
}

}  // end anonymous namespace
