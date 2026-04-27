//===- include/draw/font.hpp ------------------------------*- mode: C++ -*-===//
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
#ifndef DRAW_FONT_HPP
#define DRAW_FONT_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>
#include <unordered_map>

#include "draw/iumap.hpp"
#include "draw/types.hpp"

namespace draw {

namespace details {

/// The hash function used for the glyph container. This is intended to be a perfect hash: the result for
/// every code-point is unique and less than the container's capacity.
struct glyph_hasher {
  constexpr std::size_t operator()(std::uint32_t const cp) const noexcept {
    switch (cp) {
    // The following two code-points are mapped to missing glyphs.
    case 0x25a1: return 127;  // WHITE SQUARE
    case 0xfffd:
      return 144;  // REPLACEMENT CHARACTER
    // Most code point map directly to their hash.
    default: return cp;
    }
  }
};

}  // end namespace details

constexpr auto white_square = std::uint32_t{0x25A1U};

struct kerning_pair {
  /// Code point of the preceding glyph
  std::uint32_t preceding : 21;
  std::uint32_t pad : 3;
  std::uint32_t distance : 8;

  friend constexpr bool operator==(kerning_pair const&, kerning_pair const&) noexcept = default;
};

/// Trivial_span is roughly equivalent to std::span<> but is a trivial type so that it can be used
/// as an argument to a constexpr iumap.
template <typename T>
struct trivial_span {
  using element_type = T;
  using value_type = std::remove_cv_t<T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T* DRAW_NONNULL;
  using const_pointer = T const* DRAW_NONNULL;
  using reference = T&;
  using const_reference = T const&;
  using iterator = pointer;
  using const_iterator = const_pointer;

  T* DRAW_NONNULL data_;
  std::size_t size_;

  [[nodiscard]] constexpr pointer data() const noexcept { return data_; }
  [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }
  [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0U; }

  [[nodiscard]] constexpr iterator begin() noexcept { return data_; }
  [[nodiscard]] constexpr const_iterator begin() const noexcept { return data_; }
  [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return data_; }

  [[nodiscard]] constexpr iterator end() noexcept { return data_ + size_; }
  [[nodiscard]] constexpr const_iterator end() const noexcept { return data_ + size_; }
  [[nodiscard]] constexpr const_iterator cend() const noexcept { return data_ + size_; }

  [[nodiscard]] constexpr reference operator[](size_type idx) const { return *(data_ + idx); }

  template <std::size_t Size>
  [[nodiscard]] static constexpr trivial_span from_array(std::array<T const, Size> const& arr) noexcept {
    return {.data_ = arr.data(), .size_ = Size};
  }
  template <std::size_t Size>
  [[nodiscard]] static constexpr trivial_span from_array(std::array<std::remove_const_t<T>, Size> const& arr) noexcept {
    return {.data_ = arr.data(), .size_ = Size};
  }
};
static_assert(std::is_trivial_v<trivial_span<int>>,
              "The trivial_span<> type must be trivial so that glyph_map can be constexpr");

struct glyph {
  trivial_span<kerning_pair const> kerns;
  trivial_span<std::byte const> bm;
};
static_assert(std::is_trivial_v<glyph>, "The glyph type must be trivial so that glyph_map can be constexpr");

// The glyphs in a font are always a multiple of 8 pixels tall as given by the font's 'height'
// member. The glyph data consists of an array of bytes which are grouped according to the font height: a
// height of 2 implies groups of 2 bytes each representing 8 pixels; a height of 4 indicates
// groups of 4 bytes give a total of 32 pixels per group. Each group of bytes represents a single
// column of pixels in the font. The least significant bit in each byte holds the pixel value for
// the smallest y position.
struct font {
  std::uint8_t id = 0U;
  std::uint8_t baseline = 0U;
  std::uint8_t widest = 0U;
  std::uint8_t height : 4 = 0U;  ///< Height of a glyph (measured in bytes rather than pixels).
  std::uint8_t spacing : 4 = 0U;

  using glyph_map = iumap<std::uint32_t, glyph, 256U, details::glyph_hasher>;
  glyph_map glyphs;

  [[nodiscard]] constexpr std::uint16_t width(glyph const& g) const noexcept {
    auto const& bitmap = g.bm;
    return static_cast<std::uint16_t>(bitmap.size() / this->height);
  }

  [[nodiscard]] constexpr glyph const* DRAW_NONNULL find_glyph(char32_t const code_point) const {
    assert(!glyphs.empty());
    auto pos = glyphs.find(code_point);
    if (auto const end = glyphs.end(); pos == end) {
      pos = glyphs.find(white_square);
      if (pos == end) {
        // We've got no definition for the requested code point and no definition for U+25A1 (WHITE SQUARE). Last resort
        // is the first glyph.
        pos = glyphs.begin();
      }
    }
    return &pos->second;
  }
};

constexpr std::array<kerning_pair const, 0> empty_kern;

}  // end namespace draw

#endif  // DRAW_FONT_HPP
