//===- unit_tests/test_rgbs.cpp -------------------------------------------===//
//*            _            *
//*  _ __ __ _| |__   __ _  *
//* | '__/ _` | '_ \ / _` | *
//* | | | (_| | |_) | (_| | *
//* |_|  \__, |_.__/ \__,_| *
//*      |___/              *
//===----------------------------------------------------------------------===//
// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
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
#include "draw/types.hpp"

// Google Test
#include <gtest/gtest.h>

namespace {

using draw::rgba;
using draw::rgba_premult;

TEST(Rgba, White100) {
  rgba const w{.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF};
  rgba_premult const wpm{w};
  EXPECT_EQ(wpm, (rgba_premult{0xFF, 0xFF, 0xFF, 0xFF}));
}

TEST(Rgba, White50) {
  constexpr rgba w{.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0x7F};
  rgba_premult const wpm{w};
  EXPECT_EQ(wpm, (rgba_premult{0x7F, 0x7F, 0x7F, 0x7F}));
}

TEST(Rgba, Red100WithGreen50) {
  constexpr rgba r{.r = 0xFF, .g = 0x00, .b = 0x00, .a = 0xFF};
  constexpr rgba g50{.r = 0x00, .g = 0xFF, .b = 0x00, .a = 0x7F};
  constexpr rgba_premult rpm{r};
  constexpr rgba_premult g50pm{g50};

  auto outp = rpm;
  outp.composite(g50pm);
  EXPECT_EQ(outp, (rgba_premult{0x80, 0x7F, 0x00, 0xFF}));
  EXPECT_EQ(outp.to_straight(), (rgba{.r = 0x80, .g = 0x7F, .b = 0x00, .a = 0xFF}));
}

TEST(Rgba, Red50WithGreen50) {
  constexpr rgba r50{.r = 0xFF, .g = 0x00, .b = 0x00, .a = 0x7F};
  constexpr rgba g50{.r = 0x00, .g = 0xFF, .b = 0x00, .a = 0x7F};
  constexpr rgba_premult r50pm{r50};
  constexpr rgba_premult g50pm{g50};

  auto outp = r50pm;
  outp.composite(g50pm);
  EXPECT_EQ(outp, (rgba_premult{0x40, 0x7F, 0x00, 0xBF}));
  EXPECT_EQ(outp.to_straight(), (rgba{.r = 0x55, .g = 0xAA, .b = 0x00, .a = 0xBF}));
}

}  // end anonymous namespace
