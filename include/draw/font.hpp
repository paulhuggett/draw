//===- include/draw/font.hpp ------------------------------*- mode: C++ -*-===//
//*   __             _    *
//*  / _| ___  _ __ | |_  *
//* | |_ / _ \| '_ \| __| *
//* |  _| (_) | | | | |_  *
//* |_|  \___/|_| |_|\__| *
//*                       *
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
#ifndef DRAW_FONT_HPP
#define DRAW_FONT_HPP

#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace draw {

constexpr auto white_square = std::uint32_t{0x25A1};

struct kerning_pair {
  /// Code point of the preceeding glyph
  std::uint32_t preceeding : 21 = 0;
  std::uint32_t pad : 3 = 0;
  std::uint32_t distance : 8 = 0;
};

// The glyphs in a font are always a multiple of 8 pixels tall as given by the font's 'height'
// member. The glyph data consists of an array of bytes which are grouped according to the font height: a
// height of 2 implies groups of 2 bytes each representing 8 pixels; a height of 4 indicates
// groups of 4 bytes give a total of 32 pixels per group. Each group of bytes represents a single
// column of pixels in the font. The least significant bit in each byte holds the pixel value for
// the smallest y position.
struct font {
  std::uint8_t id = 0;
  std::uint8_t baseline = 0;
  std::uint8_t widest = 0;
  std::uint8_t height : 4 = 0;  // in bytes rather than pixels.
  std::uint8_t spacing : 4 = 0;

  using glyph = std::tuple<std::span<kerning_pair const>, std::span<std::byte const>>;

  constexpr std::uint16_t width(glyph const& g) const noexcept {
    auto const& bitmap = std::get<std::span<std::byte const>>(g);
    return static_cast<std::uint16_t>(bitmap.size() / this->height);
  }

  glyph const* find_glyph(char32_t code_point) const {
    auto pos = glyphs.find(static_cast<std::uint32_t>(code_point));
    auto end = glyphs.end();
    if (pos == end) {
      pos = glyphs.find(white_square);
      if (pos == end) {
        // We've got no definition for the requested code point and no definition for U+25A1 (WHITE SQUARE). Last resort
        // is the first glyph.
        pos = glyphs.begin();
      }
    }
    return &pos->second;
  }

  // TODO: replace this with iumap<>
  // [https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/include/midi2/adt/iumap.hpp]
  // (or something like it).
  std::unordered_map<std::uint32_t, glyph> glyphs;
};

extern std::vector<font const*> all_fonts;

}  // end namespace draw

#endif  // DRAW_FONT_HPP
