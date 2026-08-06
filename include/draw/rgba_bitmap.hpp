//===- include/draw/rgba_bitmap.hpp -----------------------*- mode: C++ -*-===//
//*            _             _     _ _                          *
//*  _ __ __ _| |__   __ _  | |__ (_) |_ _ __ ___   __ _ _ __   *
//* | '__/ _` | '_ \ / _` | | '_ \| | __| '_ ` _ \ / _` | '_ \  *
//* | | | (_| | |_) | (_| | | |_) | | |_| | | | | | (_| | |_) | *
//* |_|  \__, |_.__/ \__,_| |_.__/|_|\__|_| |_| |_|\__,_| .__/  *
//* |___/                                          |_|          *
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
#ifndef DRAW_RGBA_BITMAP_HPP
#define DRAW_RGBA_BITMAP_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdlib>
#include <limits>
#include <memory>
#include <ranges>
#include <span>

#include "draw/basic_bitmap.hpp"
#include "draw/types.hpp"

#ifndef DRAW_HOSTED
#define DRAW_HOSTED (0)
#endif

namespace draw {

struct font;
class glyph_cache;

template <>
struct basic_pen<rgba> {
  pattern pat = black;
  rgba foreground{0xFF, 0xFF, 0xFF, 0xFF};
  rgba background{0x00, 0x00, 0x00, 0xFF};
};

using rgba_pen = basic_pen<rgba>;

template <>
struct basic_pen<rgba_premult> {
  constexpr basic_pen() noexcept = delete;
  constexpr basic_pen(basic_pen const&) noexcept = default;
  explicit constexpr basic_pen(basic_pen<rgba> const& other) noexcept
      : pat{other.pat}, foreground{other.foreground}, background{other.background} {}

  pattern pat = black;
  rgba_premult foreground{0xFF, 0xFF, 0xFF, 0xFF};
  rgba_premult background{0x00, 0x00, 0x00, 0xFF};
};

using rgba_premult_pen = basic_pen<rgba_premult>;

template <>
class basic_bitmap<rgba> {
  friend class glyph_cache;

public:
  constexpr basic_bitmap() noexcept = default;
  constexpr basic_bitmap(std::span<rgba_premult> const& store, std::uint16_t const width,
                         std::uint16_t const height) noexcept
      : width_{width}, height_{height}, stride_{required_stride(width)}, store_{store} {
    assert(store.size() >= this->actual_store_size() && "store is too small");
    assert(width <= static_cast<std::uint16_t>(std::numeric_limits<coordinate>::max()) && "width is too great");
    assert(height <= static_cast<std::uint16_t>(std::numeric_limits<coordinate>::max()) && "height is too great");
  }

  [[nodiscard]] static constexpr std::uint16_t required_stride(std::uint16_t const width) noexcept {
    assert(width <= static_cast<std::uint16_t>(std::numeric_limits<coordinate>::max()) && "width is too great");
    return std::min(width, static_cast<std::uint16_t>(std::numeric_limits<coordinate>::max()));
  }
  /// Returns the store size required for a bitmap with the supplied dimensions.
  /// \param width  The desired width of the bitmap in pixels.
  /// \param height  The desired height of the bitmap in pixels.
  /// \returns The size, in bytes, of the required frame store for a bitmap with the supplied dimensions.
  [[nodiscard]] static constexpr std::size_t required_store_size(std::uint16_t const width,
                                                                 std::uint16_t const height) noexcept {
    assert(height <= static_cast<std::uint16_t>(std::numeric_limits<coordinate>::max()) && "height is too great");
    return required_stride(width) * static_cast<std::size_t>(std::min(
                                        height, static_cast<std::uint16_t>(std::numeric_limits<coordinate>::max())));
  }
  [[nodiscard]] constexpr std::span<rgba_premult const> store() const noexcept { return store_; }
  [[nodiscard]] constexpr std::span<rgba_premult> store() noexcept { return store_; }

  enum class transfer_mode : std::uint8_t { mode_copy, mode_or };
  void copy(basic_bitmap<bool> const& source, point dest_pos, rgba const& foreground, rgba const& background) noexcept;
  void clear() noexcept { std::ranges::fill(this->store(), rgba_premult{}); }
  /// Sets or clears an individual pixel.
  /// \param p The pixel to be set
  /// \param pen The pen with which the pixel is drawn
  constexpr void set(point const p, rgba_pen const& pen) noexcept;
  constexpr void set(point const p, rgba_premult_pen const& pen) noexcept;
  /// Draws a straight line from p0 to p1.
  /// \param p0  Coordinate of one end of the line
  /// \param p1  Coordinate of the other end of the line
  /// \param pen The pen with which the line is drawn
  void line(point p0, point p1, rgba_pen const& pen) noexcept;

  void frame_rect(rect const& r, rgba_pen const& pen) noexcept { generic_frame_rect(*this, r, pen); }
  void paint_rect(rect const& r, rgba_pen const& pen) noexcept { generic_paint_rect(*this, r, pen); }

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

#if defined(DRAW_HOSTED) && DRAW_HOSTED
  void dump(std::FILE* stream = stdout) const;
#endif

private:
  std::uint16_t width_ = 0U;       ///< Width of the bitmap in pixels
  std::uint16_t height_ = 0U;      ///< Height of the bitmap in pixels
  std::uint16_t stride_ = 0U;      ///< Number of bytes per row
  std::optional<rect> dirty_;      ///< The area of the bitmap modified since the last call to clean(), if any.
  std::span<rgba_premult> store_;  ///< The backing store containing the bitmap's pixel data

  [[nodiscard]] constexpr std::size_t actual_store_size() const noexcept {
    return static_cast<std::size_t>(stride_) * height_;
  }
  void line_horizontal(unsigned x0, unsigned x1, unsigned const y, rgba_premult_pen const& pen) noexcept;
  void line_vertical(unsigned const x, unsigned y0, unsigned y1, rgba_premult_pen const& pen) noexcept;

  /// Adds the supplied rectangle to the "dirty" area.
  constexpr void mark_dirty(rect const& modified) noexcept {
    assert(modified.top >= 0 && modified.bottom >= modified.top && modified.bottom < height_);
    assert(modified.left >= 0 && modified.right >= modified.left && modified.right < width_);
    dirty_ = dirty_ ? dirty_->union_rect(modified) : modified;
  }
};

constexpr void basic_bitmap<rgba>::set(point const p, rgba_premult_pen const& pen) noexcept {
  if (p.x < 0 || p.y < 0) {
    return;
  }
  auto const x = static_cast<unsigned>(p.x);
  auto const y = static_cast<unsigned>(p.y);
  if (x >= width_ || y >= height_) {
    return;
  }

  auto const mask = std::byte{1} << (x % 8U);
  auto const pattern = pen.pat[y % 8];
  auto const& color = (pattern & mask) != std::byte{0} ? pen.foreground : pen.background;

  auto const index = y * stride_ + x;
  assert(index < this->actual_store_size());
  store_[index] = store_[index].composite(color);
  this->mark_dirty({.top = p.y, .left = p.x, .bottom = p.y, .right = p.x});
}
constexpr void basic_bitmap<rgba>::set(point const p, rgba_pen const& pen) noexcept {
  this->set(p, rgba_premult_pen{pen});
}

using rgba_bitmap = basic_bitmap<rgba>;

}  // end namespace draw

#endif  // DRAW_RGBA_BITMAP_HPP
