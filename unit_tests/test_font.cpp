//===- unit_tests/test_font.cpp -------------------------------------------===//
//*   __             _    *
//*  / _| ___  _ __ | |_  *
//* | |_ / _ \| '_ \| __| *
//* |  _| (_) | | | | |_  *
//* |_|  \___/|_| |_|\__| *
//*                       *
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
#include "draw/font.hpp"
#include "draw/types.hpp"

// Standard library
#include <unordered_map>

// Google Test
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

using namespace draw::literals;
using draw::font;
using testing::ElementsAre;

TEST(Font, FindGlyphLatinSmallLetterA) {
  draw::glyph const* const g = draw::sans16.find_glyph(U'a');
  ASSERT_NE(g, nullptr);
  EXPECT_TRUE(g->kerns.empty());
  EXPECT_THAT(g->bm, ElementsAre(0x80_b, 0x0C_b, 0x40_b, 0x12_b, 0x40_b, 0x12_b, 0x40_b, 0x12_b, 0x80_b, 0x1F_b, 0x00_b,
                                 0x10_b));
}

TEST(Font, FindMissingGlyph) {
  EXPECT_EQ(draw::sans16.find_glyph(char32_t{0x0600U}), draw::sans16.find_glyph(draw::white_square));
}

TEST(Font, FindMissingGlyphNoWhiteSqaure) {
  static constexpr std::array bitmap_0020 = {0x00_b, 0x00_b, 0x00_b, 0x00_b};
  static constexpr font const minimal{.id = 0xFF,
                                      .baseline = 12,
                                      .widest = 1,
                                      .height = 2,
                                      .spacing = 1,
                                      .glyphs = draw::font::glyph_map{
                                          {0x20, draw::glyph{decltype(draw::glyph::kerns)::from_array(draw::empty_kern),
                                                             decltype(draw::glyph::bm)::from_array(bitmap_0020)}},
                                      }};
  EXPECT_EQ(minimal.find_glyph(char32_t{0x0600U}), minimal.find_glyph(char32_t{0x20U}));
}

class FontParam : public testing::TestWithParam<std::reference_wrapper<draw::font const>> {};

TEST_P(FontParam, PerfectHash) {
  draw::font const& font = GetParam();
  auto const& glyphs = font.glyphs;
  auto const& hash_function = glyphs.hash_function();

  // Check that the hash of every code-point in the font is both unique and less than the capacity of the container.
  std::unordered_map<std::size_t, std::uint32_t> hashes;
  std::ranges::for_each(glyphs, [&](auto const& kvp) {
    auto const& [code_point, _] = kvp;
    auto const hash = hash_function(code_point);
    EXPECT_LT(hash, glyphs.capacity()) << "hash for code-point " << code_point;
    auto [pos, did_insert] = hashes.emplace(hash, code_point);
    EXPECT_TRUE(did_insert) << "hash for code-point " << code_point << " collides with code-point " << pos->second;
  });
}

INSTANTIATE_TEST_SUITE_P(FontParam, FontParam, testing::ValuesIn(draw::all_fonts));

}  // end anonymous namespace
