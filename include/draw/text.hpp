//===- include/draw/text.hpp ------------------------------*- mode: C++ -*-===//
//*  _            _    *
//* | |_ _____  _| |_  *
//* | __/ _ \ \/ / __| *
//* | ||  __/>  <| |_  *
//*  \__\___/_/\_\\__| *
//*                    *
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
#ifndef DRAW_TEXT_HPP
#define DRAW_TEXT_HPP

// icubaby dependency
#include "icubaby/icubaby.hpp"

// Local includes
#include "draw/font.hpp"
#include "draw/glyph_cache.hpp"
#include "draw/types.hpp"

namespace draw {

namespace details {

[[nodiscard]] constexpr coordinate glyph_spacing(font const& f, glyph const& g,
                                                 std::optional<char32_t> prev_code_point) {
  if (!prev_code_point.has_value()) {
    return 0;
  }
  auto space = static_cast<draw::coordinate>(f.spacing);
  auto const kerning_pairs_end = std::end(g.kerns);
  if (auto const kern_pos =
          std::find_if(std::begin(g.kerns), kerning_pairs_end,
                       [&prev_code_point](draw::kerning_pair const& kp) { return kp.preceding == prev_code_point; });
      kern_pos != kerning_pairs_end) {
    space -= static_cast<draw::coordinate>(kern_pos->distance);
  }
  return space;
}

template <typename DrawFn>
[[nodiscard]] constexpr draw::coordinate scan_code_point(draw::coordinate x, draw::font const& f,
                                                         char32_t const code_point,
                                                         std::optional<char32_t> const prev_code_point, DrawFn&& draw) {
  glyph const* const g = f.find_glyph(code_point);
  x += glyph_spacing(f, *g, prev_code_point);
  std::invoke(std::forward<DrawFn>(draw), code_point, x);
  x += f.width(*g);
  return x;
}

}  // end namespace details

template <typename DrawFn>
constexpr coordinate scan_string(font const& f, std::u8string_view s, DrawFn draw) {
  auto x = coordinate{0};

  std::optional<char32_t> prev_cp;

  icubaby::t8_32 transcoder;
  std::array<char32_t, 1U> code_point_buffer{};

  auto const begin = std::begin(code_point_buffer);  // NOLINT(*-qualified-auto)
  for (auto const cu : s) {
    if (auto const it = transcoder(cu, begin); it != begin) {  // NOLINT(*-qualified-auto)
      // We have a code point.
      assert(std::distance(begin, it) == 1);
      x = details::scan_code_point(x, f, *begin, prev_cp, draw);
      prev_cp = *begin;
    }
  }
  if (auto const it = transcoder.end_cp(begin); it != begin) {  // NOLINT(*-qualified-auto)
    // We have a code point.
    assert(std::distance(begin, it) == 1);
    x = details::scan_code_point(x, f, *begin, prev_cp, draw);
  }

  return x;
}

/// Returns the width of the specified code-point in pixels. It does not include any inter-character spacing allowance.
///
/// \param f  A font instance
/// \param code_point  The code point specifying the glyph to be drawn
/// \returns The width of the specified glyph
constexpr std::uint16_t char_width(font const& f, char32_t code_point) {
  glyph const* const g = f.find_glyph(code_point);
  assert(g != nullptr);
  return f.width(*g);
}

constexpr coordinate string_width(font const& f, std::u8string_view s) {
  return scan_string(f, s, [](char32_t /*code_point*/, coordinate /*x*/) constexpr {
    // do nothing
  });
}

template <typename Color>
class basic_bitmap;
class glyph_cache;

/// Renders an individual glyph.
///
/// \tparam Color  A color type
/// \param bitmap  The destination bitmap
/// \param gc  The glyph cache
/// \param f  The font in which the character will be rendered
/// \param code_point  The code point specifying the glyph to be drawn
/// \param pos The position at which the glyph should be drawn
template <typename Color, typename... Args>
void draw_char(basic_bitmap<Color>& bitmap, glyph_cache& gc, font const& f, char32_t const code_point, point pos,
               Args&&... args) {
  if (pos.x > bitmap.width() || pos.y > bitmap.height()) {
    return;
  }
  bitmap.copy(gc.get(f, code_point), pos, std::forward<Args>(args)...);
}

/// Renders a sequence of codepoints contained in a UTF-8 encoded string.
///
/// \tparam Color  A color type
/// \param bitmap  The destination bitmap
/// \param gc  The glyph cache
/// \param f  The font in which the character will be rendered
/// \param s  The UTF-8 encoded string to be drawn
/// \param pos  The position for the first of the run of glyphs
/// \returns  The origin position \p pos with the x coordinate increased by the width of all the rendered glyphs.
template <typename Color, typename... Args>
point draw_string(basic_bitmap<Color>& bitmap, glyph_cache& gc, font const& f, std::u8string_view s, point pos,
                  Args&&... args) {
  coordinate const new_x = scan_string(f, s, [&args..., &bitmap, &gc, &f, &pos](char32_t code_point, coordinate x) {
    draw_char(bitmap, gc, f, code_point, {.x = static_cast<coordinate>(pos.x + x), .y = pos.y},
              std::forward<Args>(args)...);
  });
  return {.x = static_cast<coordinate>(pos.x + new_x), .y = pos.y};
}

}  // end namespace draw

#endif  // DRAW_TEXT_HPP
