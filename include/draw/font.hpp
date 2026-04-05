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
// Font includes
#include "draw/sans16.hpp"
#include "draw/sans32.hpp"

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
  std::uint32_t preceding : 21 = 0U;
  std::uint32_t pad : 3 = 0U;
  std::uint32_t distance : 8 = 0U;

  friend constexpr bool operator==(kerning_pair const&, kerning_pair const&) noexcept = default;
};

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
  std::uint8_t height : 4 = 0U;  // in bytes rather than pixels.
  std::uint8_t spacing : 4 = 0U;

  using kerning_pairs = std::span<kerning_pair const>;
  using bytes = std::span<std::byte const>;
  using glyph = std::tuple<kerning_pairs, bytes>;
  using glyph_map = iumap<std::uint32_t, glyph, 256U, details::glyph_hasher>;
  glyph_map glyphs;

  [[nodiscard]] constexpr std::uint16_t width(glyph const& g) const noexcept {
    auto const& bitmap = std::get<bytes>(g);
    return static_cast<std::uint16_t>(bitmap.size() / this->height);
  }

  [[nodiscard]] constexpr glyph const* find_glyph(char32_t const code_point) const {
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

constexpr std::array<kerning_pair, 0> empty_kern;

constexpr std::array all_fonts{std::cref(sans16), std::cref(sans32)};

}  // end namespace draw

#endif  // DRAW_FONT_HPP
