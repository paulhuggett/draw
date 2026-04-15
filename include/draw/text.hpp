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
#include "draw/types.hpp"

namespace draw {

namespace details {

constexpr coordinate glyph_spacing(font const& f, font::glyph const& g, std::optional<char32_t> prev_code_point) {
  if (!prev_code_point.has_value()) {
    return 0;
  }
  auto space = static_cast<draw::coordinate>(f.spacing);
  auto const kerning_pairs = std::get<std::span<draw::kerning_pair const>>(g);
  auto const kerning_pairs_end = std::end(kerning_pairs);
  if (auto const kern_pos =
          std::find_if(std::begin(kerning_pairs), kerning_pairs_end,
                       [&prev_code_point](draw::kerning_pair const& kp) { return kp.preceding == prev_code_point; });
      kern_pos != kerning_pairs_end) {
    space -= static_cast<draw::coordinate>(kern_pos->distance);
  }
  return space;
}

template <typename DrawFn>
constexpr draw::coordinate scan_code_point(draw::coordinate x, draw::font const& f, char32_t const code_point,
                                           std::optional<char32_t> const prev_code_point, DrawFn&& draw) {
  draw::font::glyph const* const g = f.find_glyph(code_point);
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

constexpr coordinate string_width(font const& f, std::u8string_view s) {
  return scan_string(f, s, [](char32_t /*code_point*/, coordinate /*x*/) constexpr {
    // do nothing
  });
}

}  // end namespace draw

#endif  // DRAW_TEXT_HPP
