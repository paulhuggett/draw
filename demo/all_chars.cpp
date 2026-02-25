//===- demo/all_chars.cpp -------------------------------------------------===//
//*        _ _        _                     *
//*   __ _| | |   ___| |__   __ _ _ __ ___  *
//*  / _` | | |  / __| '_ \ / _` | '__/ __| *
//* | (_| | | | | (__| | | | (_| | |  \__ \ *
//*  \__,_|_|_|  \___|_| |_|\__,_|_|  |___/ *
//*                                         *
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

#include "draw/bitmap.hpp"
#include "draw/glyph_cache.hpp"
#include "draw/sans16.hpp"
#include "draw/sans32.hpp"
#include "draw/types.hpp"

using draw::bitmap;
using draw::coordinate;
using draw::font;
using draw::point;

namespace {

std::vector<char32_t> sorted_code_points(font const& f) {
  std::vector<char32_t> code_points;
  std::ranges::copy(std::views::keys(f.glyphs) | std::views::filter([](char32_t const k) { return k > ' '; }),
                    std::back_inserter(code_points));
  std::ranges::sort(code_points);
  return code_points;
}

}  // end anonymous namespace

int main() {
  constexpr auto frame_width = coordinate{128};
  constexpr auto frame_height = coordinate{32};
  std::array<std::byte, bitmap::required_store_size(frame_width, frame_height)> frame_store{};
  bitmap bm{frame_store, frame_width, frame_height};
  draw::glyph_cache gc;

  point pos;
  for (auto const& font = sans16; auto const code_point : sorted_code_points(font)) {
    auto const width = bm.char_width(font, code_point);
    if (pos.x + width > bm.width()) {
      pos.x = 0;
      pos.y += font.height * 8;
      if (pos.y >= bm.height()) {
        break;
      }
    }

    bm.draw_char(gc, sans16, code_point, pos);
    pos.x += width + 1;
  }
#if defined(DRAW_HOSTED) && DRAW_HOSTED
  bm.dump();
#endif
}
