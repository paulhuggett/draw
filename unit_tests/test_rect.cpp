//===- unit_tests/test_rect.cpp -------------------------------------------===//
//*                _    *
//*  _ __ ___  ___| |_  *
//* | '__/ _ \/ __| __| *
//* | | |  __/ (__| |_  *
//* |_|  \___|\___|\__| *
//*                     *
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
#include "draw/types.hpp"

// Google Test
#include <gtest/gtest.h>

// Local includes
#include "rect.hpp"

namespace {

TEST(Rect, Null) {
  constexpr draw::rect r;
  EXPECT_EQ(r, (draw::rect{.top = 0, .left = 0, .bottom = 0, .right = 0}));
  EXPECT_EQ(r.width(), 0);
  EXPECT_EQ(r.height(), 0);
}

// Note that I don't use constexpr here so that calls to rect methods count towards coverage.
TEST(Rect, InsetEmpty) {
  auto const r = draw::rect{}.inset(1, 1);
  EXPECT_EQ(r.width(), 0);
  EXPECT_EQ(r.height(), 0);
}

TEST(Rect, InsetSmaller) {
  auto const r = draw::rect{.top = 10, .left = 10, .bottom = 20, .right = 20}.inset(1, 1);
  EXPECT_EQ(r, (draw::rect{.top = 11, .left = 11, .bottom = 19, .right = 19}));
  EXPECT_EQ(r.width(), 8);
  EXPECT_EQ(r.height(), 8);
}

TEST(Rect, InsetLarger1) {
  auto const r = draw::rect{.top = 10, .left = 10, .bottom = 20, .right = 20}.inset(-1, -1);
  EXPECT_EQ(r, (draw::rect{.top = 9, .left = 9, .bottom = 21, .right = 21}));
  EXPECT_EQ(r.width(), 12);
  EXPECT_EQ(r.height(), 12);
}

TEST(Rect, InsetLarger2) {
  auto const r = draw::rect{.top = 10, .left = 10, .bottom = 20, .right = 20}.inset(-5, -5);
  EXPECT_EQ(r, (draw::rect{.top = 5, .left = 5, .bottom = 25, .right = 25}));
  EXPECT_EQ(r.width(), 20);
  EXPECT_EQ(r.height(), 20);
}

TEST(Rect, InsetToEmpty) {
  auto const r = draw::rect{.top = 2, .left = 2, .bottom = 4, .right = 4}.inset(2, 2);
  EXPECT_EQ(r, (draw::rect{.top = 3, .left = 3, .bottom = 3, .right = 3}));
  EXPECT_EQ(r.width(), 0);
  EXPECT_EQ(r.height(), 0);
}

TEST(Rect, InsetOutset) {
  auto const r = draw::rect{.top = 2, .left = 2, .bottom = 4, .right = 4}.inset(-2, -2);
  EXPECT_EQ(r, (draw::rect{.top = 0, .left = 0, .bottom = 6, .right = 6}));
}

TEST(Rect, Union) {
  auto const r1 = draw::rect{.top = 1, .left = 1, .bottom = 2, .right = 2};
  auto const r2 = draw::rect{.top = 2, .left = 2, .bottom = 3, .right = 3};
  auto const r3 = r1.union_rect(r2);
  EXPECT_EQ(r3, (draw::rect{.top = 1, .left = 1, .bottom = 3, .right = 3}));
  auto const r4 = r2.union_rect(r1);
  EXPECT_EQ(r4, (draw::rect{.top = 1, .left = 1, .bottom = 3, .right = 3}));
}

TEST(Rect, Offset) {
  auto const r1 = draw::rect{.top = 1, .left = 2, .bottom = 3, .right = 4};
  auto const r2 = r1.offset({.x = 2, .y = 1});
  EXPECT_EQ(r2, (draw::rect{.top = 2, .left = 4, .bottom = 4, .right = 6}));
}

}  // end anonymous namespace
