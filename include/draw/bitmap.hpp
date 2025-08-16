//===- include/draw/bitmap.hpp ----------------------------*- mode: C++ -*-===//
//*  _     _ _                          *
//* | |__ (_) |_ _ __ ___   __ _ _ __   *
//* | '_ \| | __| '_ ` _ \ / _` | '_ \  *
//* | |_) | | |_| | | | | | (_| | |_) | *
//* |_.__/|_|\__|_| |_| |_|\__,_| .__/  *
//*                             |_|     *
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
#ifndef DRAW_BITMAP_HPP
#define DRAW_BITMAP_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdlib>
#include <limits>
#include <memory>
#include <ranges>
#include <span>
#include <tuple>

#include "types.hpp"

namespace draw {

class glyph_cache;

class bitmap {
  friend class glyph_cache;

public:
  constexpr bitmap() noexcept = default;
  constexpr bitmap(std::span<std::byte> store, std::uint16_t width, std::uint16_t height, std::uint16_t stride) noexcept
      : store_{store}, width_{width}, height_{height}, stride_{stride} {
    assert(store.size() >= this->actual_store_size() && "store is too small");
    assert(width <= static_cast<std::uint16_t>(std::numeric_limits<ordinate>::max()) && "width is too great");
    assert(height <= static_cast<std::uint16_t>(std::numeric_limits<ordinate>::max()) && "height is too great");
  }
  constexpr bitmap(std::span<std::byte> store, std::uint16_t width, std::uint16_t height) noexcept
      : bitmap(store, width, height, required_stride(width)) {}

  static constexpr std::uint16_t required_stride(std::uint16_t width) noexcept {
    assert(width <= static_cast<std::uint16_t>(std::numeric_limits<ordinate>::max()) && "width is too great");
    return (width + 7U) / 8U;
  }
  /// Returns the store size required for a bitmap with the supplied dimensions.
  static constexpr std::size_t required_store_size(std::uint16_t width, std::uint16_t height) noexcept {
    assert(height <= static_cast<std::uint16_t>(std::numeric_limits<ordinate>::max()) && "height is too great");
    return required_stride(width) * height;
  }

  enum class transfer_mode { mode_copy, mode_or };
  void copy(bitmap const& source, point dest_pos, transfer_mode mode);
  void clear() { std::ranges::fill(this->store(), std::byte{0}); }
  bool set(point p, bool new_state);
  /// \param p0  Coordinate of one end of the line
  /// \param p1  Coordinate of the other end of the line
  void line(point p0, point p1);

  void frame_rect(rect const& r);
  void paint_rect(rect const& r, pattern const& pat);

  /// Renders an individual glyph.
  ///
  /// \param gc  The glyph cache
  /// \param code_point  The code point specifying the glyph to be drawn
  /// \param pos The position at which the glyph should be drawn
  void draw_char(glyph_cache& gc, char32_t code_point, point pos);
  point draw_string(glyph_cache& gc, std::u8string_view s, point pos);
  /// Returns the character width of the specified character.
  ///
  /// \param gc  The glyph cache
  /// \param code_point  The code point specifying the glyph to be drawn
  /// \returns The width of the specified glyph
  std::uint16_t char_width(glyph_cache const& gc, char32_t code_point);

  [[nodiscard]] constexpr std::uint16_t width() const noexcept { return width_; }
  [[nodiscard]] constexpr std::uint16_t height() const noexcept { return height_; }
  [[nodiscard]] constexpr std::uint16_t stride() const noexcept { return stride_; }
  [[nodiscard]] constexpr rect bounds() const noexcept {
    return {.top = 0, .left = 0, .bottom = static_cast<ordinate>(height()), .right = static_cast<ordinate>(width())};
  }
  [[nodiscard]] constexpr std::span<std::byte const> store() const noexcept { return store_; }
  [[nodiscard]] constexpr std::span<std::byte> store() noexcept { return store_; }

  void dump(std::FILE* stream = stdout) const;

private:
  std::span<std::byte> store_;
  std::uint16_t width_ = 0;   ///< Width of the bitmap in pixels
  std::uint16_t height_ = 0;  ///< Height of the bitmap in pixels
  std::uint16_t stride_ = 0;  ///< Number of bytes per row

  constexpr std::size_t actual_store_size() const noexcept { return stride_ * height_; }
  void line_horizontal(std::uint16_t x0, std::uint16_t x1, std::uint16_t y, std::byte const pattern);
  void line_vertical(std::uint16_t x, std::uint16_t y0, std::uint16_t y1);
};

inline bool bitmap::set(point const p, bool const new_state) {
  if (p.x < 0 || p.y < 0) {
    return false;
  }
  auto const x = static_cast<unsigned>(p.x);
  auto const y = static_cast<unsigned>(p.y);
  if (x >= width_ || y >= height_) {
    return false;
  }
  auto const index = y * stride_ + x / 8U;
  assert(index < this->actual_store_size());
  auto& b = store_[index];
  auto const bit = std::byte{0x80} >> (x % 8U);
  // TODO: Checkout <https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching>
  if (new_state) {
    b |= bit;
  } else {
    b &= ~bit;
  }
  return true;
}

std::tuple<std::unique_ptr<std::byte[]>, draw::bitmap> create_bitmap_and_store(std::uint16_t width,
                                                                               std::uint16_t height);

extern pattern const black;
extern pattern const white;
extern pattern const gray;
extern pattern const light_gray;

ordinate string_width(glyph_cache& gc, std::u8string_view s);

}  // end namespace draw

#endif  // DRAW_BITMAP_HPP
