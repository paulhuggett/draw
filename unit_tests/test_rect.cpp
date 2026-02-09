//===- unit_tests/test_rect.cpp -------------------------------------------===//
//*                _    *
//*  _ __ ___  ___| |_  *
//* | '__/ _ \/ __| __| *
//* | | |  __/ (__| |_  *
//* |_|  \___|\___|\__| *
//*                     *
//===----------------------------------------------------------------------===//
// Copyright © 2025 Paul Bowen-Huggett
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
//
// SPDX-License-Identifier: MIT
//===----------------------------------------------------------------------===//
#include <gtest/gtest.h>

#include "draw/bitmap.hpp"
#include "draw/types.hpp"

namespace draw {

void PrintTo(rect const& r, std::ostream* os);
void PrintTo(rect const& r, std::ostream* os) {
  *os << "{.top=" << r.top << ",.left=" << r.left << ",.bottom=" << r.bottom << ",.right=" << r.right << '}';
}

}  // namespace draw
namespace {

TEST(Rect, Null) {
  draw::rect r;
  EXPECT_EQ(r.width(), 0);
  EXPECT_EQ(r.height(), 0);
  EXPECT_TRUE(r.empty());
}

TEST(Rect, InsetEmpty) {
  constexpr draw::rect r = draw::rect{}.inset(1, 1);
  EXPECT_EQ(r.width(), 0);
  EXPECT_EQ(r.height(), 0);
  EXPECT_TRUE(r.empty());
}

TEST(Rect, InsetSmaller) {
  constexpr draw::rect r = draw::rect{.top = 10, .left = 10, .bottom = 20, .right = 20}.inset(1, 1);
  EXPECT_EQ(r, (draw::rect{.top = 11, .left = 11, .bottom = 19, .right = 19}));
  EXPECT_EQ(r.width(), 8);
  EXPECT_EQ(r.height(), 8);
  EXPECT_FALSE(r.empty());
}

TEST(Rect, InsetLarger1) {
  constexpr draw::rect r = draw::rect{.top = 10, .left = 10, .bottom = 20, .right = 20}.inset(-1, -1);
  EXPECT_EQ(r, (draw::rect{.top = 9, .left = 9, .bottom = 21, .right = 21}));
  EXPECT_EQ(r.width(), 12);
  EXPECT_EQ(r.height(), 12);
  EXPECT_FALSE(r.empty());
}

TEST(Rect, InsetLarger2) {
  constexpr draw::rect r = draw::rect{.top = 10, .left = 10, .bottom = 20, .right = 20}.inset(-5, -5);
  EXPECT_EQ(r, (draw::rect{.top = 5, .left = 5, .bottom = 25, .right = 25}));
  EXPECT_EQ(r.width(), 20);
  EXPECT_EQ(r.height(), 20);
  EXPECT_FALSE(r.empty());
}

TEST(Rect, InsetToEmpty) {
  constexpr draw::rect r = draw::rect{.top = 10, .left = 10, .bottom = 20, .right = 20}.inset(1, 1);
  EXPECT_EQ(r, (draw::rect{.top = 11, .left = 11, .bottom = 19, .right = 19}));
  EXPECT_EQ(r.width(), 8);
  EXPECT_EQ(r.height(), 8);
  EXPECT_FALSE(r.empty());
}

}  // end anonymous namespace
