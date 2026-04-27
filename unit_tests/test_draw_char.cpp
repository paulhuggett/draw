//===- unit_tests/test_draw_char.cpp --------------------------------------===//
//*      _                           _                 *
//*   __| |_ __ __ ___      __   ___| |__   __ _ _ __  *
//*  / _` | '__/ _` \ \ /\ / /  / __| '_ \ / _` | '__| *
//* | (_| | | | (_| |\ V  V /  | (__| | | | (_| | |    *
//*  \__,_|_|  \__,_| \_/\_/    \___|_| |_|\__,_|_|    *
//*                                                    *
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
#include "draw/all_fonts.hpp"
#include "draw/bitmap.hpp"
#include "draw/font.hpp"
#include "draw/glyph_cache.hpp"
#include "draw/sans16.hpp"
#include "draw/types.hpp"

// Standard library
#include <vector>

// Google test/mock
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Local includes
#include "create_bitmap.hpp"

namespace {

using testing::ElementsAre;

TEST(DrawChar, Basic) {
  using namespace draw::literals;

  static constexpr std::array bitmap_0020 = {
      0b01010101_b,
      0b10101010_b,  // column 0
      0b10101010_b,
      0b01010101_b,  // column 1
  };
  constexpr auto character = char32_t{0x20};
  constexpr draw::font const minimal{
      .id = 0xFF,
      .baseline = 12,
      .widest = 1,
      .height = 2,
      .spacing = 1,
      .glyphs = draw::font::glyph_map{
          {character, draw::glyph{decltype(draw::glyph::kerns)::from_array(draw::empty_kern),
                                  decltype(draw::glyph::bm)::from_array(bitmap_0020)}},
      }};
  auto [store, bmp] = create_bitmap_and_store(8U, 16U);
  std::vector glyph_cache_store{draw::glyph_cache::get_size(minimal), std::byte{0U}};
  draw::glyph_cache gc{std::ranges::subrange{&minimal, &minimal + 1}, glyph_cache_store};
  bmp.draw_char(gc, minimal, character, draw::point{.x = 0, .y = 0});
  EXPECT_THAT(bmp.store(), ElementsAre(0b10000000_b,  // [0]
                                       0b01000000_b,  // [1]
                                       0b10000000_b,  // [2]
                                       0b01000000_b,  // [3]
                                       0b10000000_b,  // [4]
                                       0b01000000_b,  // [5]
                                       0b10000000_b,  // [6]
                                       0b01000000_b,  // [7]
                                       0b01000000_b,  // [8]
                                       0b10000000_b,  // [9]
                                       0b01000000_b,  // [10]
                                       0b10000000_b,  // [11]
                                       0b01000000_b,  // [12]
                                       0b10000000_b,  // [13]
                                       0b01000000_b,  // [14]
                                       0b10000000_b   // [15]
                                       ));
  EXPECT_EQ(bmp.dirty(), gc.get(minimal, character).bounds());
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 15, .right = 1}));
}

TEST(DrawChar, A) {
  using namespace draw::literals;

  auto [store, bmp] = create_bitmap_and_store(16U, 16U);
  std::vector glyph_cache_store{draw::glyph_cache::get_size(draw::all_fonts), std::byte{0U}};
  draw::glyph_cache gc{draw::all_fonts, glyph_cache_store};
  constexpr auto const& font = draw::sans16;
  constexpr auto character = U'A';
  bmp.draw_char(gc, font, character, draw::point{.x = 0, .y = 0});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000100_b, 0b00000000_b,  // [3]
                                       0b00000100_b, 0b00000000_b,  // [4]
                                       0b00001010_b, 0b00000000_b,  // [5]
                                       0b00001010_b, 0b00000000_b,  // [6]
                                       0b00010001_b, 0b00000000_b,  // [7]
                                       0b00010001_b, 0b00000000_b,  // [8]
                                       0b00111111_b, 0b10000000_b,  // [9]
                                       0b01000000_b, 0b01000000_b,  // [10]
                                       0b01000000_b, 0b01000000_b,  // [11]
                                       0b10000000_b, 0b00100000_b,  // [12]
                                       0b00000000_b, 0b00000000_b,  // [13]
                                       0b00000000_b, 0b00000000_b,  // [14]
                                       0b00000000_b, 0b00000000_b   // [15]
                                       ));
  EXPECT_EQ(bmp.dirty(), gc.get(font, character).bounds());
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 15, .right = 10}));
}
TEST(DrawChar, Sans32A_Sans16A) {
  using namespace draw::literals;

  auto [store, bmp] = create_bitmap_and_store(32U, 32U);
  std::vector glyph_cache_store{draw::glyph_cache::get_size(draw::all_fonts), std::byte{0U}};
  draw::glyph_cache gc{draw::all_fonts, glyph_cache_store};
  constexpr auto character = U'A';
  bmp.draw_char(gc, draw::sans16, character, draw::point{.x = 0, .y = 0});
  EXPECT_EQ(bmp.dirty(), gc.get(draw::sans16, character).bounds());
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 15, .right = 10}));
  bmp.draw_char(
      gc, draw::sans32, character,
      draw::point{.x = static_cast<draw::coordinate>(draw::bitmap::char_width(draw::sans16, character)), .y = 0});
  EXPECT_THAT(bmp.store(), ElementsAre(0b00000000_b, 0b00000000_b, 0b00000000_b, 0b00000000_b,  // [0]
                                       0b00000000_b, 0b00000000_b, 0b00000000_b, 0b00000000_b,  // [1]
                                       0b00000000_b, 0b00000000_b, 0b00000000_b, 0b00000000_b,  // [2]
                                       0b00000100_b, 0b00000000_b, 0b00000000_b, 0b00000000_b,  // [3]
                                       0b00000100_b, 0b00000000_b, 0b00001110_b, 0b00000000_b,  // [4]
                                       0b00001010_b, 0b00000000_b, 0b00001110_b, 0b00000000_b,  // [5]
                                       0b00001010_b, 0b00000000_b, 0b00011111_b, 0b00000000_b,  // [6]
                                       0b00010001_b, 0b00000000_b, 0b00011111_b, 0b00000000_b,  // [7]
                                       0b00010001_b, 0b00000000_b, 0b00111111_b, 0b10000000_b,  // [8]
                                       0b00111111_b, 0b10000000_b, 0b00111011_b, 0b10000000_b,  // [9]
                                       0b01000000_b, 0b01000000_b, 0b00111011_b, 0b10000000_b,  // [10]
                                       0b01000000_b, 0b01000000_b, 0b01110001_b, 0b11000000_b,  // [11]
                                       0b10000000_b, 0b00100000_b, 0b01110001_b, 0b11000000_b,  // [12]
                                       0b00000000_b, 0b00000000_b, 0b11110001_b, 0b11100000_b,  // [13]
                                       0b00000000_b, 0b00000000_b, 0b11100000_b, 0b11100000_b,  // [14]
                                       0b00000000_b, 0b00000001_b, 0b11100000_b, 0b11110000_b,  // [15]
                                       0b00000000_b, 0b00000001_b, 0b11000000_b, 0b01110000_b,  // [16]
                                       0b00000000_b, 0b00000001_b, 0b11111111_b, 0b11110000_b,  // [17]
                                       0b00000000_b, 0b00000011_b, 0b11111111_b, 0b11111000_b,  // [18]
                                       0b00000000_b, 0b00000011_b, 0b11111111_b, 0b11111000_b,  // [19]
                                       0b00000000_b, 0b00000111_b, 0b10000000_b, 0b00111100_b,  // [20]
                                       0b00000000_b, 0b00000111_b, 0b00000000_b, 0b00011100_b,  // [21]
                                       0b00000000_b, 0b00001111_b, 0b00000000_b, 0b00011110_b,  // [22]
                                       0b00000000_b, 0b00001110_b, 0b00000000_b, 0b00001110_b,  // [23]
                                       0b00000000_b, 0b00001110_b, 0b00000000_b, 0b00001110_b,  // [24]
                                       0b00000000_b, 0b00011100_b, 0b00000000_b, 0b00000111_b,  // [25]
                                       0b00000000_b, 0b00000000_b, 0b00000000_b, 0b00000000_b,  // [26]
                                       0b00000000_b, 0b00000000_b, 0b00000000_b, 0b00000000_b,  // [27]
                                       0b00000000_b, 0b00000000_b, 0b00000000_b, 0b00000000_b,  // [28]
                                       0b00000000_b, 0b00000000_b, 0b00000000_b, 0b00000000_b,  // [29]
                                       0b00000000_b, 0b00000000_b, 0b00000000_b, 0b00000000_b,  // [30]
                                       0b00000000_b, 0b00000000_b, 0b00000000_b, 0b00000000_b   // [31]
                                       ));
  EXPECT_EQ(bmp.dirty(), (draw::rect{.top = 0, .left = 0, .bottom = 31, .right = 31}));
}

}  // end anonymous namespace
