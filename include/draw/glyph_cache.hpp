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

#include <cstddef>
#include <vector>

#include "bitmap.hpp"
#include "font.hpp"
#include "plru_cache.hpp"

namespace draw {

class glyph_cache {
  /// Enable to inspect the unpacking and rotation of the font data.
  static constexpr bool trace_unpack = false;

public:
  explicit constexpr glyph_cache(font const& f) noexcept : font_{&f} {
    store_.resize(cache_.max_size() * glyph_cache::store_size(f));
  }

  [[nodiscard]] bitmap const& get(char32_t code_point);

  [[nodiscard]] font::glyph const* find_glyph(char32_t code_point) const;
  [[nodiscard]] constexpr font const* get_font() const noexcept { return font_; }
  [[nodiscard]] constexpr std::uint8_t spacing() const noexcept { return font_->spacing; }

private:
  /// Renders an individual glyph into the supplied bitmap.
  [[nodiscard]] bitmap render(std::span<std::byte> bitmap_store, char32_t code_point);

  [[nodiscard]] static constexpr std::size_t store_size(font const& f) noexcept {
    std::size_t const stride = (f.widest + 7U) / 8U;
    std::size_t const pixel_height = f.height * 8U;
    return stride * pixel_height;
  }
  font const* font_;
  /// A bitmap that is large enough to contain the largest glyph in the font.
#if 0
  // TODO: don't allocate a vector every time. Separate the store.
  struct entry {
    std::vector<std::byte> store;
    bitmap bm;
  };
#endif
  std::vector<std::byte> store_;
  plru_cache<std::uint32_t, bitmap, 8, 2> cache_;
};

}  // end namespace draw

#endif  // DRAW_GLYPH_CACHE_HPP
