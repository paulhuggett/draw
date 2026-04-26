//===- include/draw/glyph_cache.hpp -----------------------*- mode: C++ -*-===//
//*        _             _                      _           *
//*   __ _| |_   _ _ __ | |__     ___ __ _  ___| |__   ___  *
//*  / _` | | | | | '_ \| '_ \   / __/ _` |/ __| '_ \ / _ \ *
//* | (_| | | |_| | |_) | | | | | (_| (_| | (__| | | |  __/ *
//*  \__, |_|\__, | .__/|_| |_|  \___\__,_|\___|_| |_|\___| *
//*  |___/   |___/|_|                                       *
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
#ifndef DRAW_GLYPH_CACHE_HPP
#define DRAW_GLYPH_CACHE_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <span>
#include <vector>

#include "bitmap.hpp"
#include "font.hpp"
#include "plru_cache.hpp"

namespace draw {

class glyph_cache {
private:
  template <std::ranges::input_range FontsRange>
  [[nodiscard]] static constexpr std::size_t get_store_size(FontsRange&& fonts) noexcept {
    return std::ranges::max(fonts | std::views::transform(glyph_cache::get_font_store_size));
  }

public:
  template <std::ranges::input_range Range>
    requires std::is_same_v<
                 std::remove_cvref_t<std::unwrap_reference_t<std::ranges::range_value_t<std::remove_cvref_t<Range>>>>,
                 font>
  constexpr glyph_cache(Range&& fonts, std::span<std::byte> const& store) noexcept
      : store_size_{get_store_size(std::forward<Range>(fonts))}, store_{store} {
    assert(store_.size_bytes() >= store_size_);
  }

  constexpr glyph_cache(font const& f, std::span<std::byte> const& store) noexcept
      : glyph_cache(std::ranges::views::single(std::cref(f)), store) {}

  /// Returns a bitmap containing the rendered glyph from the supplied font.
  [[nodiscard]] bitmap const& get(font const& f, char32_t code_point);

  template <std::ranges::input_range FontsRange>
    requires std::is_same_v<
        std::remove_cvref_t<std::unwrap_reference_t<std::ranges::range_value_t<std::remove_cvref_t<FontsRange>>>>, font>
  [[nodiscard]] static constexpr std::size_t get_size(FontsRange&& fonts) noexcept {
    return decltype(cache_)::max_size() * glyph_cache::get_store_size(std::forward<FontsRange>(fonts));
  }
  [[nodiscard]] static constexpr std::size_t get_size(font const& f) noexcept {
    auto const fonts = std::ranges::views::single(std::cref(f));
    return get_size(fonts);
  }

private:
  /// Renders an individual glyph into the supplied bitmap.
  [[nodiscard]] static bitmap render(font const& f, char32_t code_point, std::span<std::byte> bitmap_store);

  /// Returns the number of bytes required for the largest glyph in the supplied font.
  [[nodiscard]] static constexpr std::size_t get_font_store_size(font const& f) noexcept {
    std::size_t const stride = (f.widest + 7U) / 8U;
    auto const pixel_height = f.height * 8U;
    return stride * pixel_height;
  }

  /// The size of the largest glyph that can be held by the cache.
  std::size_t store_size_;
  /// A block of memory that is large enough to contain a full cache of the largest glyph in the font.
  std::span<std::byte> store_;
  plru_cache<std::uint32_t, bitmap, 8U, 2U> cache_;
};

}  // end namespace draw

#endif  // DRAW_GLYPH_CACHE_HPP
