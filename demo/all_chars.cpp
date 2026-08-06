//===- demo/all_chars.cpp -------------------------------------------------===//
//*        _ _        _                     *
//*   __ _| | |   ___| |__   __ _ _ __ ___  *
//*  / _` | | |  / __| '_ \ / _` | '__/ __| *
//* | (_| | | | | (__| | | | (_| | |  \__ \ *
//*  \__,_|_|_|  \___|_| |_|\__,_|_|  |___/ *
//*                                         *
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

#include <vector>

#include "draw/glyph_cache.hpp"
#include "draw/mono_bitmap.hpp"
#include "draw/sans16.hpp"
#include "draw/text.hpp"
#include "draw/types.hpp"

namespace {

using draw::coordinate;
using draw::font;
using draw::mono_bitmap;
using draw::point;

std::vector<char32_t> sorted_code_points(font const& f) {
  std::vector<char32_t> code_points;
  std::ranges::copy(std::views::keys(f.glyphs) | std::views::filter([](char32_t const k) { return k > ' '; }),
                    std::back_inserter(code_points));
  std::ranges::sort(code_points);
  return code_points;
}

}  // end anonymous namespace

int main() {
  constexpr auto frame_width = std::uint16_t{128U};
  constexpr auto frame_height = std::uint16_t{32U};
  std::array<std::byte, mono_bitmap::required_store_size(frame_width, frame_height)> frame_store{};
  mono_bitmap bitmap{frame_store, frame_width, frame_height};
  static auto const& font = draw::sans16;
  std::vector glyph_cache_store{draw::glyph_cache::get_size(font), std::byte{0U}};
  draw::glyph_cache gc{font, glyph_cache_store};

  point pos;
  for (auto const code_point : sorted_code_points(font)) {
    auto const width = static_cast<std::int16_t>(char_width(font, code_point));
    if (pos.x + width > bitmap.width()) {
      pos.x = 0;
      pos.y += font.height * 8;
      if (pos.y >= bitmap.height()) {
        break;
      }
    }

    draw_char(bitmap, gc, font, code_point, pos, draw::transfer_mode::mode_or);
    pos.x += width + 1;
  }
#if defined(DRAW_HOSTED) && DRAW_HOSTED
  bitmap.dump();
#endif
}
