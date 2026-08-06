//===- unit_tests/test_rgba_paint_rect.cpp --------------------------------===//
//*              _       _                   _    *
//*  _ __   __ _(_)_ __ | |_   _ __ ___  ___| |_  *
//* | '_ \ / _` | | '_ \| __| | '__/ _ \/ __| __| *
//* | |_) | (_| | | | | | |_  | | |  __/ (__| |_  *
//* | .__/ \__,_|_|_| |_|\__| |_|  \___|\___|\__| *
//* |_|                                           *
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
#include "draw/rgba_bitmap.hpp"

// Google test/mock
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Local includes
#include "create_bitmap.hpp"

using testing::ElementsAre;
using namespace draw::literals;
using draw::rgba_pen;
using draw::rgba_premult;

namespace {

TEST(RGBAFrame, WhiteOnBlack) {
  auto const b = draw::rgba_premult{0x00, 0x00, 0x00, 0xFF};
  auto const w = draw::rgba_premult{0xFF, 0xFF, 0xFF, 0xFF};
  auto [store, bitmap] = create_bitmap32_and_store(8U, 8U);
  bitmap.paint_rect(draw::rect{.top = 1, .left = 1, .bottom = 6, .right = 6}, rgba_pen{});
  EXPECT_THAT(bitmap.store(), ElementsAre(b, b, b, b, b, b, b, b, b, w, w, w, w, w, w, b, b, w, w, w, w, w, w, b, b, w,
                                          w, w, w, w, w, b, b, w, w, w, w, w, w, b, b, w, w, w, w, w, w, b, b, w, w, w,
                                          w, w, w, b, b, b, b, b, b, b, b, b));
}

using draw::rgba_premult;

TEST(RGBAFrame, RedAndGreenOnWhite) {
  auto const b = draw::rgba{0x00, 0x00, 0x00, 0xFF};
  auto const r = draw::rgba{0xFF, 0x00, 0x00, 0x80};
  auto const g = draw::rgba{0x00, 0xFF, 0x00, 0x80};

  auto const B = rgba_premult{b};
  auto const R = B.composite(draw::rgba_premult{r});
  auto const G = B.composite(draw::rgba_premult{g});
  auto const X = B.composite(R).composite(draw::rgba_premult{g});

  auto [store, bitmap] = create_bitmap32_and_store(8U, 8U);
  bitmap.paint_rect(draw::rect{.top = 1, .left = 1, .bottom = 4, .right = 4}, rgba_pen{.foreground = r});
  bitmap.paint_rect(draw::rect{.top = 3, .left = 3, .bottom = 7, .right = 7}, rgba_pen{.foreground = g});
  EXPECT_THAT(bitmap.store(), ElementsAre(B, B, B, B, B, B, B, B,  // [0]
                                          B, R, R, R, R, B, B, B,  // [1]
                                          B, R, R, R, R, B, B, B,  // [2]
                                          B, R, R, X, X, G, G, G,  // [3]
                                          B, R, R, X, X, G, G, G,  // [4]
                                          B, B, B, G, G, G, G, G,  // [5]
                                          B, B, B, G, G, G, G, G,  // [6]
                                          B, B, B, G, G, G, G, G   // [7]
                                          ));
}

}  // end anonymous namespace
