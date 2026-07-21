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

#include "draw/bitmap32.hpp"

namespace draw {

void bitmap32::line_horizontal(unsigned x0, unsigned x1, unsigned const y, rgba_premult const& color,
                               std::byte const pattern) {
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

  for (auto x = x0; x <= x1; ++x, ++it) {
    assert(it < store_.end() && "iterator is not within the bitmap");
    auto const mask = std::byte{1} << (x % 8U);
    if ((pattern & mask) != std::byte{0}) {
      it->composite(color);
    }
  }
}

void bitmap32::line_vertical(unsigned const x, unsigned y0, unsigned y1, rgba_premult const& color) {
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
  for (auto y = y0; y < y1; ++y) {
    assert(index < store_.size() && "index is not within the bitmap");
    store_[index].composite(color);
    index += stride_;
  }

  this->mark_dirty({.top = static_cast<coordinate>(y0),
                    .left = static_cast<coordinate>(x),
                    .bottom = static_cast<coordinate>(y1 - 1U),
                    .right = static_cast<coordinate>(x)});
}

void bitmap32::line(point p0, point p1, rgba const& color) {
  using namespace draw::literals;
  auto const colorpm = rgba_premult{color};
  if (p0.y == p1.y) {
    if (p0.y >= 0 && p0.y < height_) {
      this->line_horizontal(static_cast<std::uint16_t>(std::max(p0.x, coordinate{0})),
                            static_cast<std::uint16_t>(std::max(p1.x, coordinate{0})), static_cast<std::uint16_t>(p0.y),
                            colorpm, 0xFF_b);
    }
    return;
  }
  if (p0.x == p1.x) {
    if (p0.x >= 0 && p0.x < width_) {
      this->line_vertical(static_cast<unsigned>(p0.x), static_cast<unsigned>(std::max(p0.y, coordinate{0})),
                          static_cast<unsigned>(std::max(p1.y, coordinate{0})), colorpm);
    }
    return;
  }

  auto const sx = p0.x < p1.x ? coordinate{1} : coordinate{-1};
  auto const sy = p0.y < p1.y ? coordinate{1} : coordinate{-1};
  auto const dx = std::abs(static_cast<int>(p1.x) - static_cast<int>(p0.x));
  auto const dy = -std::abs(static_cast<int>(p1.y) - static_cast<int>(p0.y));
  auto err = dx + dy;

  for (;;) {
    this->set({.x = p0.x, .y = p0.y}, colorpm);
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
