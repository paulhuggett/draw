//===- lib/bitmap.cpp -----------------------------------------------------===//
//*  _     _ _                          *
//* | |__ (_) |_ _ __ ___   __ _ _ __   *
//* | '_ \| | __| '_ ` _ \ / _` | '_ \  *
//* | |_) | | |_| | | | | | (_| | |_) | *
//* |_.__/|_|\__|_| |_| |_|\__,_| .__/  *
//*                             |_|     *
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
#include "draw/mono_bitmap.hpp"

// Standard library
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <optional>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>
#include <version>
#if defined(__cpp_lib_print) && __cpp_lib_print >= 202207L
#include <print>
#endif

// Local includes
#include "draw/font.hpp"
#include "draw/glyph_cache.hpp"
#include "draw/text.hpp"
#include "draw/tracer.hpp"
#include "draw/types.hpp"

namespace {

// TODO: Add a SIMD implementation? Check what the compiler already does.
template <typename T>
void memor(T* DRAW_NONNULL dest, T const* DRAW_NONNULL src, std::size_t len) noexcept {
  for (; len > 0U; --len) {
    *(dest++) |= *(src++);
  }
}

template <typename T>
void transfer(T* const DRAW_NONNULL dest, T const mask, T const v, draw::transfer_mode const mode) noexcept {
  using enum draw::transfer_mode;
  switch (mode) {
  case mode_or: *dest |= v; break;
  case mode_copy: *dest = (*dest & ~mask) | v; break;
  default: assert(false && "unknown transfer mode"); break;
  }
}

void copy_row_aligned(unsigned src_x, unsigned const src_x_end, std::byte const* const DRAW_NONNULL src_row,
                      unsigned const dest_x, std::byte* const DRAW_NONNULL dest_row,
                      draw::transfer_mode const mode) noexcept {
  using namespace draw::literals;
  assert(src_row != nullptr && "Source row must be supplied");
  assert(dest_row != nullptr && "Destination row must be supplied");
  assert(src_x_end >= src_x && "Source X range must be valid");
  using enum draw::transfer_mode;
  assert(src_x % 8U == dest_x % 8U);

  auto const* src = src_row + (src_x / 8U);
  auto* dest = dest_row + (dest_x / 8U);
  auto const len = (src_x_end / 8U) - (src_x / 8U);
  switch (mode) {
  case mode_copy: std::memcpy(dest, src, len); break;
  case mode_or: memor(dest, src, len); break;
  default: assert(false && "unknown transfer mode"); break;
  }
  src_x += len * 8U;
  assert(src_x + 8U > src_x_end && "There should be less than a whole byte left to copy");
  if (src_x < src_x_end) {
    auto const mask = 0xFF_b << (8U - (src_x_end % 8U));
    transfer(dest + len, mask, *(src + len) & mask, mode);
  }
}

// There's less than a byte to copy.
void copy_row_tiny(unsigned src_x, unsigned const src_x_end, std::byte const* const DRAW_NONNULL src_row,
                   unsigned dest_x, std::byte* const DRAW_NONNULL dest_row, draw::transfer_mode const mode) noexcept {
  using namespace draw::literals;
  assert(src_row != nullptr && "Source row must be supplied");
  assert(dest_row != nullptr && "Destination row must be supplied");
  assert(src_x % 8U != dest_x % 8U);
  // There's less than a byte to copy.
  // TODO: don't do this one pixel at a time.
  auto const* const src = src_row + (src_x / 8U);
  for (; src_x < src_x_end; ++src_x, ++dest_x) {
    auto const src_bit = *src & (0x80_b >> (src_x % 8U));
    auto const dest_bit = 0x80_b >> (dest_x % 8U);
    // Conditionally set or clear bits without branching
    // https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
    auto const v = static_cast<std::byte>(-static_cast<unsigned>(src_bit != 0_b)) & dest_bit;
    transfer(dest_row + (dest_x / 8U), dest_bit, v, mode);
  }
}

void copy_row_misaligned(unsigned src_x, unsigned const src_x_end, std::byte const* const DRAW_NONNULL src_row,
                         unsigned dest_x, std::byte* const DRAW_NONNULL dest_row,
                         draw::transfer_mode const mode) noexcept {
  using namespace draw::literals;
  using enum draw::transfer_mode;
  assert(src_row != nullptr && "Source row must be supplied");
  assert(dest_row != nullptr && "Destination row must be supplied");
  assert(src_x % 8U != dest_x % 8U);
  assert(src_x + 8 <= src_x_end);

  auto const* src = src_row + (src_x / 8U);
  auto* dest = dest_row + (dest_x / 8U);

  constexpr draw::tracer<false> trace;  // instantiate with <true> to enable tracing
  trace(std::make_tuple(src_x, src_x_end), src_row);

  auto const m = dest_x % 8U;
  auto const mask_high = 0xFF_b << m;
  auto const mask_low = 0xFF_b >> m;

  if (src_x + 8U <= src_x_end) {
    // The initial partial byte.
    transfer(dest, mask_low, (*src & mask_high) >> m, mode);
    trace("{:08b}'", std::to_underlying(*dest));

    ++dest;
    src_x += 8U - m;
    dest_x += 8U - m;

    // Copying a byte at a time.
    while (src_x + 8U <= src_x_end) {
      transfer(dest, std::byte{0xFFU}, ((*src & ~mask_high) << (8U - m)) | ((*(src + 1) & mask_high) >> m), mode);
      trace("{:08b}'", std::to_underlying(*dest));

      ++dest;
      ++src;
      src_x += 8U;
      dest_x += 8U;
    }
  }
  // The final partial byte. We have fewer than eight bits of the source remaining.
  assert(src_x <= src_x_end);
  if (auto const remaining = src_x_end - src_x; remaining > 0U) {
    assert(remaining <= 8U);

    auto v = 0_b;
    if (remaining > m) {
      // The remaining bits span more than a single byte.
      v = (*src & ~mask_high) << (8U - m);
      auto const mask = ~(0xFF_b >> (remaining - m));
      v |= ((*(src + 1) & mask) >> m);
    } else {
      v = *src << (src_x % 8U);
      v &= 0xFF_b << (8U - remaining);
    }
    transfer(dest, ~mask_low, v, mode);
    trace("{:08b}", std::to_underlying(*dest));
  }
}

void copy_row(unsigned const src_x_init, unsigned const src_x_end, std::byte const* const DRAW_NONNULL src_row,
              unsigned const dest_x, std::byte* const DRAW_NONNULL dest_row, draw::transfer_mode const mode) noexcept {
  assert(src_row != nullptr && "Source row must be supplied");
  assert(dest_row != nullptr && "Destination row must be supplied");
  assert(src_x_init <= src_x_end);
  if (src_x_init % 8U == dest_x % 8U) {
    copy_row_aligned(src_x_init, src_x_end, src_row, dest_x, dest_row, mode);
  } else if (src_x_init + 8U > src_x_end) {
    copy_row_tiny(src_x_init, src_x_end, src_row, dest_x, dest_row, mode);
  } else {
    copy_row_misaligned(src_x_init, src_x_end, src_row, dest_x, dest_row, mode);
  }
}

}  // end anonymous namespace

namespace draw {

#if defined(DRAW_HOSTED) && DRAW_HOSTED
void basic_bitmap<bool>::dump(std::FILE* const stream) const {
#if defined(__cpp_lib_print) && __cpp_lib_print >= 202207L
  auto xb = 0U;  // The x ordinate (in bytes)
  for (auto const d : store_) {
    std::print(stream, "{:08b}", std::to_underlying(d));
    ++xb;
    if (xb >= stride_) {
      // The end of a scan-line.
      std::println(stream, "");
      xb = 0;
    }
  }
#else
  auto xb = 0U;  // The x ordinate (in bytes)
  for (auto const d : store_) {
    auto d2 = std::to_underlying(d);
    for (auto ctr = 0U; ctr < 8U; ++ctr) {
      std::putc((d2 & 0b10000000) != 0 ? '1' : '0', stream);
      d2 <<= 1;
    }

    ++xb;
    if (xb >= stride_) {
      // The end of a scan-line.
      std::putc('\n', stream);
      xb = 0;
    }
  }
#endif  // __cpp_lib_print
}
#endif  // DRAW_HOSTED && __cpp_lib_print

void basic_bitmap<bool>::copy(basic_bitmap<bool> const& source, point const dest_pos, transfer_mode const mode) {
  // An initial gross clipping check.
  if ((dest_pos.x >= static_cast<int>(width_)) || (dest_pos.x + static_cast<int>(source.width()) < 0) ||
      (dest_pos.y >= static_cast<int>(height_)) || (dest_pos.y + static_cast<int>(source.height()) < 0)) {
    return;
  }

  auto dest_y = static_cast<unsigned>(std::max(dest_pos.y, coordinate{0}));
  auto const dirty_top = dest_y;

  auto const src_y_init = dest_pos.y < 0 ? static_cast<unsigned>(-dest_pos.y) : 0U;
  auto const src_y_end = std::min(static_cast<unsigned>(source.height_), src_y_init + height_ - dest_y);

  auto const src_x_init = dest_pos.x >= 0 ? 0U : static_cast<unsigned>(-dest_pos.x);
  assert(width_ - std::max(dest_pos.x, coordinate{0}) >= 0);
  auto const src_x_end = std::min(static_cast<unsigned>(source.width_),
                                  src_x_init + static_cast<unsigned>(width_ - std::max(dest_pos.x, coordinate{0})));

  auto const dest_x = static_cast<unsigned>(std::max(dest_pos.x, coordinate{0}));
  for (auto src_y = src_y_init; src_y < src_y_end; ++src_y, ++dest_y) {
    auto const src_index = src_y * source.stride_;
    auto const dest_index = dest_y * stride_;
    copy_row(src_x_init, src_x_end, &source.store_[src_index], dest_x, &store_[dest_index], mode);
  }

  this->mark_dirty({.top = static_cast<coordinate>(dirty_top),
                    .left = static_cast<coordinate>(dest_x),
                    .bottom = static_cast<coordinate>(dest_y - 1U),
                    .right = static_cast<coordinate>(dest_x + src_x_end - src_x_init - 1U)});
}

void basic_bitmap<bool>::line_horizontal(unsigned x0, unsigned x1, unsigned const y, mono_pen const& pen) noexcept {
  std::byte const pattern = pen.pat[y % 8U];
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
  std::advance(it, (y * stride_) + (x0 / 8U));
  assert(it < store_.end() && "iterator is not within the bitmap");

  this->mark_dirty({.top = static_cast<coordinate>(y),
                    .left = static_cast<coordinate>(x0),
                    .bottom = static_cast<coordinate>(y),
                    .right = static_cast<coordinate>(x1)});

  // Masks used to set the least- and most-significant bits of a byte for the line's left- and right-most pixels
  // respectively.
  auto const mask_low = 0xFF_b >> (x0 % 8U);
  auto const mask_high = 0xFF_b << (7U - (x1 % 8U));

  auto bytes = (x1 / 8U) - (x0 / 8U);
  if (bytes == 0U) {
    // The line lies entirely within a single byte.
    auto const mask = mask_low & mask_high;
    *it = (*it & ~mask) | (mask & pattern);
    return;
  }

  // First part of the line up until a byte boundary.
  *it = (*it & ~mask_low) | (mask_low & pattern);
  ++it;
  --bytes;

  // Set 8 pixels at a time going from left to right.
  for (; bytes > 0U; --bytes) {
    assert(it < store_.end() && "iterator is not within the bitmap");
    *it = pattern;
    ++it;
  }
  // The final part of the line.
  assert(it < store_.end() && "iterator is not within the bitmap");
  *it = (*it & ~mask_high) | (mask_high & pattern);
}

void basic_bitmap<bool>::line_vertical(unsigned const x, unsigned y0, unsigned y1, mono_pen const& pen) noexcept {
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

  auto index = y0 * stride_ + x / 8U;
  auto const bits = 0x80_b >> (x % 8U);
  for (auto y = y0; y < y1; ++y) {
    assert(index < store_.size() && "index is not within the bitmap");
    store_[index] |= bits & pen.pat[y % 8];
    index += stride_;
  }

  this->mark_dirty({.top = static_cast<coordinate>(y0),
                    .left = static_cast<coordinate>(x),
                    .bottom = static_cast<coordinate>(y1 - 1U),
                    .right = static_cast<coordinate>(x)});
}

void basic_bitmap<bool>::line(point p0, point p1, mono_pen const& pen) noexcept {
  using namespace draw::literals;
  if (p0.y == p1.y) {
    if (p0.y >= 0 && p0.y < height_) {
      this->line_horizontal(static_cast<std::uint16_t>(std::max(p0.x, coordinate{0})),
                            static_cast<std::uint16_t>(std::max(p1.x, coordinate{0})), static_cast<std::uint16_t>(p0.y),
                            pen);
    }
    return;
  }
  if (p0.x == p1.x) {
    if (p0.x >= 0 && p0.x < width_) {
      this->line_vertical(static_cast<unsigned>(p0.x), static_cast<unsigned>(std::max(p0.y, coordinate{0})),
                          static_cast<unsigned>(std::max(p1.y, coordinate{0})), pen);
    }
    return;
  }

  auto const sx = p0.x < p1.x ? coordinate{1} : coordinate{-1};
  auto const sy = p0.y < p1.y ? coordinate{1} : coordinate{-1};
  auto const dx = std::abs(static_cast<int>(p1.x) - static_cast<int>(p0.x));
  auto const dy = -std::abs(static_cast<int>(p1.y) - static_cast<int>(p0.y));
  auto err = dx + dy;

  for (;;) {
    this->set({.x = p0.x, .y = p0.y}, pen);
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
