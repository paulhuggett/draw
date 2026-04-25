//===- demo/time.cpp ------------------------------------------------------===//
//*  _   _                 *
//* | |_(_)_ __ ___   ___  *
//* | __| | '_ ` _ \ / _ \ *
//* | |_| | | | | | |  __/ *
//*  \__|_|_| |_| |_|\___| *
//*                        *
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
#include <chrono>
#include <format>
#include <string_view>
#include <vector>

// Draw
#include "draw/bitmap.hpp"
#include "draw/glyph_cache.hpp"
#include "draw/sans16.hpp"
#include "draw/sans32.hpp"
#include "draw/types.hpp"

using namespace std::literals;
using namespace draw::literals;
using draw::bitmap;
using draw::coordinate;
using draw::gray;
using draw::point;
using draw::rect;

int main() {
  using namespace std::chrono_literals;

  constexpr auto frame_width = std::uint16_t{128U};
  constexpr auto frame_height = std::uint16_t{32U};
  constexpr auto& font = sans32;

  // Storage for the glyph cache. Allocate enough space for the largest glyph in the font.
  static std::array<std::byte, draw::glyph_cache::get_size(sans32)> glyph_cache_store;
  // Now the glyph cache itself.
  static draw::glyph_cache gc{font, glyph_cache_store};

  // Storage for the output bitmap.
  static std::array<std::byte, bitmap::required_store_size(frame_width, frame_height)> frame_store{};
  // ... and the bitmap itself.
  bitmap frame_buffer{frame_store, frame_width, frame_height};

  std::array<char8_t, 8U> time_str{u8'\0'};
  auto const first = time_str.begin();
  auto const last = std::format_to_n(first, time_str.size(), "{:%H:%M:%S}",
                                     std::chrono::round<std::chrono::seconds>(std::chrono::system_clock::now()))
                        .out;
  frame_buffer.draw_string(gc, font, std::u8string_view{first, last}, point{.x = 0, .y = 0});
  frame_buffer.dump();
}
