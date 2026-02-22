//===- include/draw/glyph_cache.hpp -----------------------*- mode: C++ -*-===//
//*        _             _                      _           *
//*   __ _| |_   _ _ __ | |__     ___ __ _  ___| |__   ___  *
//*  / _` | | | | | '_ \| '_ \   / __/ _` |/ __| '_ \ / _ \ *
//* | (_| | | |_| | |_) | | | | | (_| (_| | (__| | | |  __/ *
//*  \__, |_|\__, | .__/|_| |_|  \___\__,_|\___|_| |_|\___| *
//*  |___/   |___/|_|                                       *
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
#ifndef DRAW_GLYPH_CACHE_HPP
#define DRAW_GLYPH_CACHE_HPP

#include <algorithm>
#include <cstddef>
#include <vector>

#include "bitmap.hpp"
#include "font.hpp"
#include "plru_cache.hpp"

namespace draw {

class glyph_cache {
public:
  glyph_cache() noexcept;
  [[nodiscard]] bitmap const& get(font const& f, char32_t code_point);

private:
  /// Renders an individual glyph into the supplied bitmap.
  [[nodiscard]] static bitmap render(font const& f, char32_t code_point, std::span<std::byte> bitmap_store);
  [[nodiscard]] static constexpr std::size_t get_store_size(font const& f) noexcept {
    std::size_t const stride = (f.widest + 7U) / 8U;
    std::size_t const pixel_height = f.height * 8U;
    return stride * pixel_height;
  }
  std::size_t const store_size_;

  /// A block of memory that is large enough to contain a full cache of the largest glyph in the font.
  std::vector<std::byte> store_;
  plru_cache<char32_t, bitmap, 8, 2> cache_;
};

}  // end namespace draw

#endif  // DRAW_GLYPH_CACHE_HPP
