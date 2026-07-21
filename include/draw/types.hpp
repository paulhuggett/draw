//===- include/draw/types.hpp -----------------------------*- mode: C++ -*-===//
//*  _                          *
//* | |_ _   _ _ __   ___  ___  *
//* | __| | | | '_ \ / _ \/ __| *
//* | |_| |_| | |_) |  __/\__ \ *
//*  \__|\__, | .__/ \___||___/ *
//*      |___/|_|               *
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
#ifndef DRAW_TYPES_HPP
#define DRAW_TYPES_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

#if defined(__clang__) && __clang__
#define DRAW_NONNULL _Nonnull
#else
#define DRAW_NONNULL
#endif

namespace draw {

namespace literals {

consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}

}  // end namespace literals

using coordinate = std::int16_t;

struct point {
  constexpr friend bool operator==(point const&, point const&) noexcept = default;

  coordinate x = 0;
  coordinate y = 0;
};

struct rect {
  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  coordinate top = 0;
  coordinate left = 0;
  coordinate bottom = 0;
  coordinate right = 0;
  // NOLINTEND(misc-non-private-member-variables-in-classes)

  constexpr friend bool operator==(rect const&, rect const&) noexcept = default;

  [[nodiscard]] constexpr coordinate width() const noexcept {
    return right > left ? static_cast<coordinate>(right - left) : coordinate{0};
  }
  [[nodiscard]] constexpr coordinate height() const noexcept {
    return bottom > top ? static_cast<coordinate>(bottom - top) : coordinate{0};
  }

  [[nodiscard]] constexpr point top_left() const noexcept { return {.x = left, .y = top}; }
  [[nodiscard]] constexpr point bot_right() const noexcept { return {.x = right, .y = bottom}; }
  [[nodiscard]] constexpr rect union_rect(rect const& other) const noexcept {
    return {.top = std::min(top, other.top),
            .left = std::min(left, other.left),
            .bottom = std::max(bottom, other.bottom),
            .right = std::max(right, other.right)};
  }

  [[nodiscard]] constexpr rect offset(point p) const noexcept {
    return {.top = static_cast<coordinate>(top + p.y),
            .left = static_cast<coordinate>(left + p.x),
            .bottom = static_cast<coordinate>(bottom + p.y),
            .right = static_cast<coordinate>(right + p.x)};
  }

  /// Shrinks or expands the rectangle.
  ///
  /// The left and right sides are moved in by dx; the top and bottom in by dy.
  /// If the resulting width or height becomes less than 1, the empty rectangle iis set to the empty rectangle.
  ///
  /// \param dx  The distance by which the left position is incremented and the right decremented
  /// \param dy  The distance by which the top position is incremented and the bottom decremented
  /// \returns  The new rectangle
  [[nodiscard]] constexpr rect inset(coordinate dx, coordinate dy) const noexcept {
    dy = std::min(dy, static_cast<coordinate>(height() / 2));
    dx = std::min(dx, static_cast<coordinate>(width() / 2));
    return {
        .top = static_cast<coordinate>(top + dy),
        .left = static_cast<coordinate>(left + dx),
        .bottom = static_cast<coordinate>(bottom - dy),
        .right = static_cast<coordinate>(right - dy),
    };
  }
};

struct pattern {
  std::array<std::byte, 8> data;
};

struct rgba {
  constexpr bool operator==(rgba const& rhs) const noexcept = default;
  std::uint8_t r = 0x00;
  std::uint8_t g = 0x00;
  std::uint8_t b = 0x00;
  std::uint8_t a = 0xFF;
};

class rgba_premult {
public:
  constexpr rgba_premult() noexcept = default;
  constexpr explicit rgba_premult(rgba const& other) noexcept
      : r{premultiply(other.r, other.a)},
        g{premultiply(other.g, other.a)},
        b{premultiply(other.b, other.a)},
        a{other.a} {}
  constexpr rgba_premult(std::uint8_t const r_, std::uint8_t const g_, std::uint8_t const b_,
                         std::uint8_t const a_ = 0xFF) noexcept
      : r{r_}, g{g_}, b{b_}, a{a_} {}
  constexpr bool operator==(rgba_premult const& rhs) const noexcept = default;

  constexpr rgba_premult& composite(rgba_premult const& other) noexcept {
    r = static_cast<std::uint8_t>(other.r + div255(r * (0xFF - other.a)));
    g = static_cast<std::uint8_t>(other.g + div255(g * (0xFF - other.a)));
    b = static_cast<std::uint8_t>(other.b + div255(b * (0xFF - other.a)));
    a = static_cast<std::uint8_t>(other.a + div255(a * (0xFF - other.a)));
    return *this;
  }
  [[nodiscard]] constexpr rgba to_straight() const noexcept {
    return a == 0 ? rgba{.r = 0, .g = 0, .b = 0, .a = 0}
                  : rgba{.r = undo_premultiply(r, a), .g = undo_premultiply(g, a), .b = undo_premultiply(b, a), .a = a};
  }

  std::uint8_t r = 0x00;
  std::uint8_t g = 0x00;
  std::uint8_t b = 0x00;
  std::uint8_t a = 0xFF;

private:
  [[nodiscard]] static constexpr std::uint8_t premultiply(std::uint8_t const c, std::uint8_t const a) noexcept {
    return static_cast<std::uint8_t>(div255(static_cast<unsigned>(c) * a));
  }
  [[nodiscard]] static constexpr std::uint8_t undo_premultiply(std::uint8_t const c, std::uint8_t const a) noexcept {
    assert(a != 0);
    return static_cast<std::uint8_t>(std::min((c * 0xFFU + a / 2U) / a, 0xFFU));
  }
  // x / 255 is approx (x + (x >> 8)) >> 8
  [[nodiscard]] static constexpr unsigned div255(unsigned const x) noexcept { return (x + 0x80 + (x >> 8)) >> 8; }
};

}  // end namespace draw

#endif  // DRAW_TYPES_HPP
