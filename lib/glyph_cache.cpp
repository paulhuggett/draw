//===- lib/glyph_cache.cpp ------------------------------------------------===//
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
#include "draw/glyph_cache.hpp"

#include <cassert>
#include <print>

namespace draw {

bitmap const& glyph_cache::get(font const& f, char32_t const code_point) {
  auto const key = static_cast<std::uint32_t>(code_point);
  return cache_.access(key, [this, &f, key, code_point]() {
    // Called when a glyph was not found in the cache.
    using cache = decltype(cache_);
    assert(cache::set(key) < cache::sets);
    assert(cache::way(key) < cache::ways);
    std::size_t const index = cache::set(key) * cache::ways + cache::way(key);
    std::size_t const size = this->store_size_;
    auto const begin = std::begin(store_) + index * size;
    auto const end = begin + size;
    assert(end <= std::end(store_));
    return this->render(f, code_point, std::span{begin, end});
  });
}

bitmap glyph_cache::render(font const& f, char32_t const code_point, std::span<std::byte> bitmap_store) {
  auto height = static_cast<std::uint16_t>(f.height * 8);

  font::glyph const* glyph = f.find_glyph(code_point);

  auto const& bitmaps = std::get<std::span<std::byte const>>(*glyph);
  // auto const width = static_cast<std::uint16_t>(bitmaps.size() / f.height);
  auto const width = f.width(*glyph);
  bitmap bm{bitmap_store, width, height};
  for (auto y = std::size_t{0}; y < height; ++y) {
    if constexpr (trace_unpack) {
      std::print("|");
    }

    auto x = 0U;

    if (width >= 8) {
      // Assign whole bytes.
      for (; x < (width & ~0b111U); x += 8) {
        auto pixels = std::byte{0};
        for (auto bit = 0U; bit < 8U; ++bit) {
          auto const src_index = ((x + bit) * f.height) + (y / 8U);
          assert(src_index < bitmaps.size() && "The source byte is not within the bitmap");
          if ((bitmaps[src_index] & (std::byte{1} << (y % 8U))) != std::byte{0}) {
            pixels |= std::byte{0x80} >> bit;
          }
        }

        auto const dest_index = y * bm.stride() + (x / 8U);
        assert(dest_index < bitmap_store.size() && "The destination byte is not within the bitmap store");
        bitmap_store[dest_index] = pixels;
        if constexpr (trace_unpack) {
          auto const get_pixel = [pixels](unsigned bit) {
            return (pixels & (std::byte{1} << bit)) != std::byte{0} ? 'X' : ' ';
          };
          for (auto ctr = 0U; ctr < 8U; ++ctr) {
            std::print("{}", get_pixel(7 - ctr));
          }
        }
      }
    }

    for (; x < width; ++x) {
      auto const src_index = (x * f.height) + (y / 8U);
      assert(src_index < bitmaps.size() && "The source byte is not within the bitmap");
      auto const pixel = bitmaps[src_index] & (std::byte{1} << (y % 8U));
      bm.set(point{.x = static_cast<ordinate>(x), .y = static_cast<ordinate>(y)}, pixel != std::byte{0});
      if constexpr (trace_unpack) {
        std::print("{}", pixel != std::byte{0} ? 'X' : ' ');
      }
    }
    if constexpr (trace_unpack) {
      std::println("|{0}", y == f.baseline ? "<-" : "");
    }
  }
  return bm;
}

}  // end namespace draw
