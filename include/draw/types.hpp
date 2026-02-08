//===- include/draw/types.hpp -----------------------------*- mode: C++ -*-===//
//*  _                          *
//* | |_ _   _ _ __   ___  ___  *
//* | __| | | | '_ \ / _ \/ __| *
//* | |_| |_| | |_) |  __/\__ \ *
//*  \__|\__, | .__/ \___||___/ *
//*      |___/|_|               *
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
#ifndef DRAW_TYPES_HPP
#define DRAW_TYPES_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

namespace draw {

namespace literals {

consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}

}  // end namespace literals

using coordinate = std::int16_t;

struct point {
  coordinate x = 0;
  coordinate y = 0;
};

struct rect {
  coordinate top = 0;
  coordinate left = 0;
  coordinate bottom = 0;
  coordinate right = 0;

  constexpr bool operator==(rect const &) const noexcept = default;

  [[nodiscard]] constexpr coordinate width() const noexcept { return right > left ? right - left : 0; }
  [[nodiscard]] constexpr coordinate height() const noexcept { return bottom > top ? bottom - top : 0; }
  [[nodiscard]] constexpr bool empty() const noexcept { return bottom <= top || right <= left; }

  /// Shrinks or expands the rectangle.
  ///
  /// The left and right sides are moved in by dx; the top and bottom in by dy.
  /// If the resulting width or height becomes less than 1, the empty rectangle iis set to the empty rectangle.
  ///
  /// \param dx  The distance by which the left position is incremented and the right decremented
  /// \param dy  The distance by which the top position is incremented and the bottom decremented
  /// \returns  The new rectangle
  [[nodiscard]] constexpr rect inset(coordinate dx, coordinate dy) const noexcept {
    auto t = top + dy;
    auto l = left + dx;
    auto b = bottom - dy;
    auto r = right - dy;
    if (b <= t || r <= l) {
      t = l = b = r = 0;
    }
    return {
        .top = static_cast<coordinate>(t),
        .left = static_cast<coordinate>(l),
        .bottom = static_cast<coordinate>(b),
        .right = static_cast<coordinate>(r),
    };
  }
};

struct pattern {
  std::array<std::byte, 8> data;
};

}  // end namespace draw

#endif  // DRAW_TYPES_HPP
