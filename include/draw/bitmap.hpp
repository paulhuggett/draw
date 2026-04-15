//===- include/draw/bitmap.hpp ----------------------------*- mode: C++ -*-===//
//*  _     _ _                          *
//* | |__ (_) |_ _ __ ___   __ _ _ __   *
//* | '_ \| | __| '_ ` _ \ / _` | '_ \  *
//* | |_) | | |_| | | | | | (_| | |_) | *
//* |_.__/|_|\__|_| |_| |_|\__,_| .__/  *
//*                             |_|     *
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

#include "draw/types.hpp"

#ifndef DRAW_HOSTED
#define DRAW_HOSTED (0)
#endif

namespace draw {

struct font;
class glyph_cache;

class bitmap {
  friend class glyph_cache;

public:
  constexpr bitmap() noexcept = default;
  constexpr bitmap(std::span<std::byte> const& store, std::uint16_t const width, std::uint16_t const height,
                   std::uint16_t const stride) noexcept
      : width_{width}, height_{height}, stride_{stride}, store_{store} {
    assert(store.size() >= this->actual_store_size() && "store is too small");
    assert(width <= static_cast<std::uint16_t>(std::numeric_limits<coordinate>::max()) && "width is too great");
    assert(height <= static_cast<std::uint16_t>(std::numeric_limits<coordinate>::max()) && "height is too great");
  }
  constexpr bitmap(std::span<std::byte> const& store, std::uint16_t const width, std::uint16_t const height) noexcept
      : bitmap(store, width, height, required_stride(width)) {}

  [[nodiscard]] static constexpr std::uint16_t required_stride(std::uint16_t const width) noexcept {
    assert(width <= static_cast<std::uint16_t>(std::numeric_limits<coordinate>::max()) && "width is too great");
    return static_cast<std::uint16_t>((width + 7U) / 8U);
  }
  /// Returns the store size required for a bitmap with the supplied dimensions.
  /// \param width  The desired width of the bitmap in pixels.
  /// \param height  The desired height of the bitmap in pixels.
  /// \returns The size, in bytes, of the required frame store for a bitmap with the supplied dimensions.
  [[nodiscard]] static constexpr std::size_t required_store_size(std::uint16_t width, std::uint16_t height) noexcept {
    assert(height <= static_cast<std::uint16_t>(std::numeric_limits<coordinate>::max()) && "height is too great");
    return required_stride(width) * static_cast<std::size_t>(height);
  }

  enum class transfer_mode : std::uint8_t { mode_copy, mode_or };
  void copy(bitmap const& source, point dest_pos, transfer_mode mode);
  void clear() { std::ranges::fill(this->store(), std::byte{0U}); }
  /// Sets or clears an individual pixel.
  /// \param p The pixel to be set
  /// \param new_state The desired state of the pixel
  constexpr void set(point p, bool new_state);
  /// Draws a straight line from p0 to p1.
  /// \param p0  Coordinate of one end of the line
  /// \param p1  Coordinate of the other end of the line
  void line(point p0, point p1);

  void frame_rect(rect const& r);
  void paint_rect(rect const& r, pattern const& pat);

  /// Renders an individual glyph.
  ///
  /// \param gc  The glyph cache
  /// \param f  The font in which the character will be rendered
  /// \param code_point  The code point specifying the glyph to be drawn
  /// \param pos The position at which the glyph should be drawn
  void draw_char(glyph_cache& gc, font const& f, char32_t code_point, point pos);
  /// \param gc  The glyph cache
  /// \param f  The font in which the character will be rendered
  /// \param s  The UTF-8 encoded string to be drawn
  /// \param pos  The position for the first of the run of glyphs
  /// \returns  The origin position \p pos with the x coordinate increased by the width of all the rendered glyphs.
  point draw_string(glyph_cache& gc, font const& f, std::u8string_view s, point pos);
  /// Returns the character width of the specified character.
  ///
  /// \param f  A font instance
  /// \param code_point  The code point specifying the glyph to be drawn
  /// \returns The width of the specified glyph
  [[nodiscard]] static std::uint16_t char_width(font const& f, char32_t code_point);

  [[nodiscard]] constexpr std::uint16_t width() const noexcept { return width_; }
  [[nodiscard]] constexpr std::uint16_t height() const noexcept { return height_; }
  [[nodiscard]] constexpr std::uint16_t stride() const noexcept { return stride_; }
  [[nodiscard]] constexpr rect bounds() const noexcept {
    return {.top = 0,
            .left = 0,
            .bottom = static_cast<coordinate>(height() - 1U),
            .right = static_cast<coordinate>(width() - 1U)};
  }
  [[nodiscard]] constexpr std::optional<rect> const& dirty() const noexcept { return dirty_; }
  constexpr void clean() noexcept { dirty_.reset(); }

  [[nodiscard]] constexpr std::span<std::byte const> store() const noexcept { return store_; }
  [[nodiscard]] constexpr std::span<std::byte> store() noexcept { return store_; }

#if defined(DRAW_HOSTED) && DRAW_HOSTED
  void dump(std::FILE* stream = stdout) const;
#endif

private:
  std::uint16_t width_ = 0U;    ///< Width of the bitmap in pixels
  std::uint16_t height_ = 0U;   ///< Height of the bitmap in pixels
  std::uint16_t stride_ = 0U;   ///< Number of bytes per row
  std::span<std::byte> store_;  ///< The backing store containing the bitmap's pixel data
  std::optional<rect> dirty_;   ///< The area of the bitmap modified since the last call to clean(), if any.

  [[nodiscard]] constexpr std::size_t actual_store_size() const noexcept {
    return static_cast<std::size_t>(stride_) * height_;
  }
  void line_horizontal(unsigned x0, unsigned x1, unsigned y, std::byte pattern);
  void line_vertical(unsigned x, unsigned y0, unsigned y1);

  /// Adds the supplied rectangle to the "dirty" area.
  constexpr void mark_dirty(rect const& modified) noexcept {
    dirty_ = dirty_ ? dirty_->union_rect(modified) : modified;
  }
};

constexpr void bitmap::set(point const p, bool const new_state) {
  if (p.x < 0 || p.y < 0) {
    return;
  }
  auto const x = static_cast<unsigned>(p.x);
  auto const y = static_cast<unsigned>(p.y);
  if (x >= width_ || y >= height_) {
    return;
  }
  auto const index = y * stride_ + x / 8U;
  assert(index < this->actual_store_size());
  auto& b = store_[index];
  auto const bit = std::byte{0x80U} >> (x % 8U);
  // TODO: Checkout <https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching>
  if (new_state) {
    b |= bit;
  } else {
    b &= ~bit;
  }

  this->mark_dirty({.top = p.y, .left = p.x, .bottom = p.y, .right = p.x});
}

extern pattern const black;
extern pattern const white;
extern pattern const gray;
extern pattern const light_gray;

}  // end namespace draw

#endif  // DRAW_BITMAP_HPP
