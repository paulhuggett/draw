//===- lib/glyph_cache.cpp ------------------------------------------------===//
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
#include "draw/glyph_cache.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>

// icubaby dependency
#include "icubaby/icubaby.hpp"

// Local includes
#include "draw/bitmap.hpp"
#include "draw/tracer.hpp"
#include "draw/types.hpp"

namespace {

[[maybe_unused, nodiscard]] constexpr char get_pixel_for_trace(std::byte const pixels, unsigned const bit) {
  return (pixels & (std::byte{1U} << bit)) != std::byte{0U} ? 'X' : ' ';
}

}  // end anonymous namespace

namespace draw {

bitmap const& glyph_cache::get(font const& f, char32_t const code_point) {
  auto const key =
      (static_cast<std::uint32_t>(f.id) << icubaby::code_point_bits) | static_cast<std::uint32_t>(code_point);
  return cache_.access(key, [this, &f, code_point](std::uint32_t const /*key*/, std::size_t const index) {
    // Called when a glyph was not found in the cache.
    using difference_type = decltype(store_)::difference_type;
    auto const begin = std::begin(store_) + static_cast<difference_type>(index * store_size_);
    auto const end = begin + static_cast<difference_type>(store_size_);
    assert(end <= std::end(store_));
    return glyph_cache::render(f, code_point, std::span{begin, end});
  });
}

bitmap glyph_cache::render(font const& f, char32_t const code_point, std::span<std::byte> bitmap_store) {
  /// Enable to inspect the unpacking and rotation of the font data.
  // ReSharper disable once CppTooWideScope
  constexpr tracer<false> trace;

  auto const height = static_cast<std::uint16_t>(f.height * 8U);
  auto const* const glyph = f.find_glyph(code_point);

  auto const& bitmaps = glyph->bm;
  auto const width = f.width(*glyph);
  bitmap bm{bitmap_store, width, height};
  for (auto y = std::size_t{0U}; y < height; ++y) {
    trace("|");

    auto x = 0U;

    if (width >= 8U) {
      // Assign whole bytes.
      for (; x < (width & ~0b111U); x += 8U) {
        auto pixels = std::byte{0U};
        for (auto bit = 0U; bit < 8U; ++bit) {
          auto const src_index = ((x + bit) * f.height) + (y / 8U);
          assert(src_index < bitmaps.size() && "The source byte is not within the bitmap");
          if ((bitmaps[src_index] & (std::byte{1U} << (y % 8U))) != std::byte{0U}) {
            pixels |= std::byte{0x80U} >> bit;
          }
        }

        auto const dest_index = y * bm.stride() + (x / 8U);
        assert(dest_index < bitmap_store.size() && "The destination byte is not within the bitmap store");
        bitmap_store[dest_index] = pixels;
        for (auto ctr = 0U; ctr < 8U; ++ctr) {
          trace("{}", get_pixel_for_trace(pixels, 7 - ctr));
        }
      }
    }

    for (; x < width; ++x) {
      auto const src_index = (x * f.height) + (y / 8U);
      assert(src_index < bitmaps.size() && "The source byte is not within the bitmap");
      auto const pixel = bitmaps[src_index] & (std::byte{1U} << (y % 8U));
      bm.set(point{.x = static_cast<coordinate>(x), .y = static_cast<coordinate>(y)}, pixel != std::byte{0});
      trace("{}", pixel != std::byte{0U} ? 'X' : ' ');
    }
    trace("|{0}\n", y == f.baseline ? "<-" : "");
  }
  return bm;
}

}  // end namespace draw
