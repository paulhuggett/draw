//===- include/draw/bitmap32.hpp --------------------------*- mode: C++ -*-===//
//*  _     _ _                        _________   *
//* | |__ (_) |_ _ __ ___   __ _ _ __|___ /___ \  *
//* | '_ \| | __| '_ ` _ \ / _` | '_ \ |_ \ __) | *
//* | |_) | | |_| | | | | | (_| | |_) |__) / __/  *
//* |_.__/|_|\__|_| |_| |_|\__,_| .__/____/_____| *
//*                             |_|               *
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

#include "draw/rgba_bitmap.hpp"

#include <cassert>
#include <ranges>

#include "draw/mono_bitmap.hpp"

namespace {

template <std::output_iterator<draw::rgba_premult> Dest>
Dest copy_n(Dest dest, unsigned const n, std::byte const src,
            std::tuple<draw::rgba_premult, draw::rgba_premult> const& colors) noexcept {
  assert(n <= 8);
  for (auto bit = 0U; bit < n; ++bit) {
    auto const is_set = static_cast<unsigned>(src) & (0x80U >> bit);
    *dest = dest->composite(is_set ? std::get<0>(colors) : std::get<1>(colors));
    ++dest;
  }
  return dest;
}

}  // end anonymous namespace

namespace draw {

void basic_bitmap<rgba>::copy(basic_bitmap<bool> const& source, point dest_pos, rgba const& foreground,
                              rgba const& background) noexcept {
  // An initial gross clipping check.
  if ((dest_pos.x >= static_cast<int>(width_)) || (dest_pos.x + static_cast<int>(source.width()) < 0) ||
      (dest_pos.y >= static_cast<int>(height_)) || (dest_pos.y + static_cast<int>(source.height()) < 0)) {
    return;
  }
  auto const w =
      static_cast<coordinate>(std::min(static_cast<std::uint16_t>(this->width() - dest_pos.x), source.width()));
  auto const h =
      static_cast<coordinate>(std::min(static_cast<std::uint16_t>(this->height() - dest_pos.y), source.height()));
  assert(w > 0 && h > 0);

  this->mark_dirty({.top = std::max(dest_pos.y, coordinate{0}),
                    .left = std::max(dest_pos.x, coordinate{0}),
                    .bottom = std::max(static_cast<coordinate>(dest_pos.y + h - 1), coordinate{0}),
                    .right = std::max(static_cast<coordinate>(dest_pos.x + w - 1), coordinate{0})});
  std::tuple const colors{rgba_premult{foreground}, rgba_premult{background}};

  auto const init_src_x = dest_pos.x < 0 ? -dest_pos.x : 0;
  auto const init_src_y = dest_pos.y < 0 ? -dest_pos.y : 0;
  dest_pos.x = std::max(dest_pos.x, coordinate{0});
  dest_pos.y = std::max(dest_pos.y, coordinate{0});

  for (auto src_y = init_src_y; src_y < h; ++src_y) {
    auto src = source.store().begin();
    std::advance(src, src_y * source.stride() + init_src_x / 8);

    auto dest = this->store().begin();
    std::advance(dest, dest_pos.y * this->stride() + dest_pos.x);

    for (auto src_x = init_src_x; src_x < w / 8; ++src_x) {
      if (*src == std::byte{0xFF} || *src == std::byte{0x00}) {
        auto const& color = *src == std::byte{0xFF} ? std::get<0>(colors) : std::get<1>(colors);
        dest = std::ranges::transform(dest, dest + 8U, dest, [&color](rgba_premult const& in) {
                 return in.composite(color);
               }).out;
      } else {
        dest = copy_n(dest, 8U, *src, colors);
      }
      ++src;
    }

    assert(w >= init_src_x);
    if (auto const n = static_cast<unsigned>((w - init_src_x)) % 8U; n > 0U) {
      assert(src >= source.store().begin() && src < source.store().end());
      assert(dest >= this->store().begin() && dest < this->store().end());
      dest = copy_n(dest, n, *src << init_src_x, colors);
    }
    ++dest_pos.y;
  }
}

void basic_bitmap<rgba>::line_horizontal(unsigned x0, unsigned x1, unsigned const y,
                                         rgba_premult_pen const& pen) noexcept {
  using namespace draw::literals;
  if (x0 > x1) {
    std::swap(x0, x1);  // Ensure that we always go from lower to higher addresses.
  }
  // A gross clipping check.
  if (x0 >= width_ || y >= height_) {
    return;
  }
  // Clamp x1 to the bitmap's right edge.
  x1 = std::min(x1, width_ - 1U);
  auto it = store_.begin();
  std::advance(it, (y * stride_) + x0);
  assert(it < store_.end() && "iterator is not within the bitmap");

  this->mark_dirty({.top = static_cast<coordinate>(y),
                    .left = static_cast<coordinate>(x0),
                    .bottom = static_cast<coordinate>(y),
                    .right = static_cast<coordinate>(x1)});

  auto const pattern = pen.pat[y % 8];
  for (auto x = x0; x <= x1; ++x, ++it) {
    assert(it < store_.end() && "iterator is not within the bitmap");
    auto const mask = std::byte{1} << (x % 8U);
    auto const& color = (pattern & mask) != std::byte{0} ? pen.foreground : pen.background;
    *it = it->composite(color);
  }
}

void basic_bitmap<rgba>::line_vertical(unsigned const x, unsigned y0, unsigned y1,
                                       rgba_premult_pen const& pen) noexcept {
  using namespace draw::literals;
  if (x >= width_) {
    return;
  }
  if (y0 > y1) {
    std::swap(y0, y1);
  }
  if (y0 >= height_) {
    return;
  }
  y1 = std::min(y1 + 1U, static_cast<unsigned>(height_));
  assert(y0 < y1);

  auto index = y0 * stride_ + x;
  auto const mask = std::byte{1} << (x % 8U);
  for (auto y = y0; y < y1; ++y) {
    auto const pattern = pen.pat[y % 8];
    auto const& color = (pattern & mask) != std::byte{0} ? pen.foreground : pen.background;
    assert(index < store_.size() && "index is not within the bitmap");
    store_[index] = store_[index].composite(color);
    index += stride_;
  }

  this->mark_dirty({.top = static_cast<coordinate>(y0),
                    .left = static_cast<coordinate>(x),
                    .bottom = static_cast<coordinate>(y1 - 1U),
                    .right = static_cast<coordinate>(x)});
}

void basic_bitmap<rgba>::line(point p0, point p1, rgba_pen const& pen) noexcept {
  using namespace draw::literals;
  auto const pen_pm = rgba_premult_pen{pen};
  if (p0.y == p1.y) {
    if (p0.y >= 0 && p0.y < height_) {
      this->line_horizontal(static_cast<std::uint16_t>(std::max(p0.x, coordinate{0})),
                            static_cast<std::uint16_t>(std::max(p1.x, coordinate{0})), static_cast<std::uint16_t>(p0.y),
                            pen_pm);
    }
    return;
  }
  if (p0.x == p1.x) {
    if (p0.x >= 0 && p0.x < width_) {
      this->line_vertical(static_cast<unsigned>(p0.x), static_cast<unsigned>(std::max(p0.y, coordinate{0})),
                          static_cast<unsigned>(std::max(p1.y, coordinate{0})), pen_pm);
    }
    return;
  }

  auto const sx = p0.x < p1.x ? coordinate{1} : coordinate{-1};
  auto const sy = p0.y < p1.y ? coordinate{1} : coordinate{-1};
  auto const dx = std::abs(static_cast<int>(p1.x) - static_cast<int>(p0.x));
  auto const dy = -std::abs(static_cast<int>(p1.y) - static_cast<int>(p0.y));
  auto err = dx + dy;

  for (;;) {
    this->set({.x = p0.x, .y = p0.y}, pen_pm);
    auto const e2 = err * 2;
    if (e2 >= dy) {
      if (p0.x == p1.x) {
        break;
      }
      err += dy;
      p0.x += sx;
    }

    if (e2 <= dx) {
      if (p0.y == p1.y) {
        break;
      }
      err += dx;
      p0.y += sy;
    }
  }
}

}  // end namespace draw
