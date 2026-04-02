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
#include "draw/font.hpp"
#include "draw/sans16.hpp"

// Google Test
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "draw/types.hpp"

namespace {

using draw::font;
using testing::ElementsAre;

using namespace draw::literals;

TEST(Font, FindGlyphLatinSmallLetterA) {
  font::glyph const* const g = sans16.find_glyph(U'a');
  ASSERT_NE(g, nullptr);
  EXPECT_EQ(std::get<font::kerning_pairs>(*g).size(), font::kerning_pairs{draw::empty_kern}.size());
  EXPECT_EQ(std::get<font::bytes>(*g).size(), 12);
}

TEST(Font, FindMissingGlyph) {
  font::glyph const* const g = sans16.find_glyph(char32_t{0x0600U});
  ASSERT_NE(g, nullptr);
  EXPECT_TRUE(std::get<font::kerning_pairs>(*g).empty());
  EXPECT_THAT(std::get<font::bytes>(*g), ElementsAre(0xfc_b, 0x1f_b, 0x04_b, 0x10_b, 0x04_b, 0x10_b, 0x04_b, 0x10_b,
                                                     0x04_b, 0x10_b, 0x04_b, 0x10_b, 0xfc_b, 0x1f_b));
}

}  // end anonymous namespace
