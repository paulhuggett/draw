#include "bitmap.hpp"

#include <cassert>
#include <utility>

#include "glyph_cache.hpp"
#include "icubaby.hpp"

namespace draw {

void bitmap::dump(std::FILE* const stream) const {
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
  std::println(stream, "{:{}}^", "", width_);
}

// TODO: this code is functional but should be copying byte-by-byte rather than pixel-by-pixel where possible.
void bitmap::copy(bitmap const& source, point dest_pos) {
  auto dest_y = dest_pos.y;
  for (auto src_y = 0U; src_y < source.height_; ++src_y, ++dest_y) {
    if (dest_y >= height_) {
      return;
    }
    auto dest_x = dest_pos.x;
    for (auto src_x = 0U; src_x < source.width_; ++src_x, ++dest_x) {
      if (dest_x >= width_) {
        break;
      }

      auto const src_index = (src_y * source.stride_) + (src_x / 8U);
      assert(src_index < source.store_.size());
      auto const dest_index = (dest_y * stride_) + (dest_x / 8U);
      assert(dest_index < store_.size());
      auto const src_pixel = source.store_[src_index] & (std::byte{0x80} >> (src_x % 8U));
      auto const dest_value = std::byte{0x80} >> (dest_x % 8U);
      if (true) {
        // This is "or" mode
        if (src_pixel != std::byte{0}) {
          store_[dest_index] |= dest_value;
        }
      } else {
        // "copy" mode.
        if (src_pixel != std::byte{0}) {
          store_[dest_index] |= dest_value;
        } else {
          store_[dest_index] &= ~dest_value;
        }
      }
    }
  }
}

void bitmap::line_horizontal(unsigned x0, unsigned x1, unsigned y, std::byte const pattern) {
  if (x0 > x1) {
    std::swap(x0, x1);
  }
  if (x0 >= width_ || y >= height_) {
    return;
  }
  x1 = std::min(x1 + 1U, width_);
  auto byte_index = y * stride_ + x0 / 8U;
  assert(byte_index < store_.size() && "index is not within the bitmap");
  if (x0 / 8U == x1 / 8U) {
    auto mask0 = static_cast<std::byte>((1U << (8 - (x0 % 8))) - 1U);
    mask0 &= ~static_cast<std::byte>((1U << (8 - (x1 % 8))) - 1U);
    auto& b = store_[byte_index];
    b = (b & ~mask0) | (mask0 & pattern);
    return;
  }

  {
    auto const mask0 = static_cast<std::byte>((1U << (8 - (x0 % 8))) - 1U);
    auto& b = store_[byte_index];
    b = (b & ~mask0) | (mask0 & pattern);
  }

  auto xbyte = (x0 / 8U) + 1U;
  auto last_xbyte = x1 / 8U;
  ++byte_index;
  for (; xbyte < last_xbyte; ++xbyte) {
    assert(byte_index < store_.size() && "index is not within the bitmap");
    store_[byte_index] = pattern;
    ++byte_index;
  }

  if (auto const num_bits = x1 % 8U; num_bits > 0U) {
    assert(byte_index < store_.size() && "index is not within the bitmap");
    auto const mask1 = static_cast<std::byte>((1U << num_bits) - 1U) << (8 - num_bits);
    auto& b = store_[byte_index];
    b = (b & ~mask1) | (mask1 & pattern);
  }
}

void bitmap::line_vertical(unsigned x, unsigned y0, unsigned y1) {
  if (x >= width_) {
    return;
  }
  if (y0 > y1) {
    std::swap(y0, y1);
  }
  if (y0 >= height_) {
    return;
  }
  y1 = std::min(y1 + 1U, height_);
  assert(y0 < y1);

  auto index = y0 * stride_ + x / 8;
  auto const bits = std::byte{0x80} >> (x % 8);
  for (auto y = y0; y < y1; ++y) {
    assert(index < store_.size() && "index is not within the bitmap");
    store_[index] |= bits;
    index += stride_;
  }
}

void bitmap::line(point p0, point p1) {
  /* Save half the line-drawing cases by swapping p0.y with p1.y
   and X0 with p1.x if p0.y is greater than p1.y. As a result, DeltaY
   is always > 0, and only the octant 0-3 cases need to be
   handled. */
  if (p0.y > p1.y) {
    std::swap(p0.y, p1.y);
    std::swap(p0.x, p1.x);
  }
  if (p0.y == p1.y) {
    this->line_horizontal(p0.x, p1.x, p0.y, std::byte{0xFF});
    return;
  }
  if (p0.x == p1.x) {
    this->line_vertical(p0.x, p0.y, p1.y);
    return;
  }

  unsigned x0 = p0.x;
  unsigned y0 = p0.y;
  unsigned x1 = p1.x;
  unsigned y1 = p1.y;
  int sx = p0.x < p1.x ? 1 : -1;
  int sy = p0.y < p1.y ? 1 : -1;
  auto const dx = std::abs(static_cast<int>(p1.x) - static_cast<int>(p0.x));
  auto const dy = -std::abs(static_cast<int>(p1.y) - static_cast<int>(p0.y));
  auto err = dx + dy;

  for (;;) {
    this->set(point{.x = static_cast<ordinate>(p0.x), .y = static_cast<ordinate>(p0.y)}, true);
    auto e2 = err * 2;
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

void bitmap::frame_rect(rect const& r) {
  if (r.right < r.left || r.bottom < r.top) {
    return;
  }
  // The top and bottom lines
  this->line(point{.x = r.left, .y = r.top}, point{.x = r.right, .y = r.top});
  this->line(point{.x = r.left, .y = r.bottom}, point{.x = r.right, .y = r.bottom});
  // The left and right lines
  this->line(point{.x = r.left, .y = r.top}, point{.x = r.left, .y = r.bottom});
  this->line(point{.x = r.right, .y = r.top}, point{.x = r.right, .y = r.bottom});
}

pattern const black{.data = {std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF},
                             std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}}};
pattern const white{.data = {std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
                             std::byte{0x00}, std::byte{0x00}, std::byte{0x00}}};
pattern const gray{.data = {std::byte{0xAA}, std::byte{0x55}, std::byte{0xAA}, std::byte{0x55}, std::byte{0xAA},
                            std::byte{0x55}, std::byte{0xAA}, std::byte{0x55}}};
pattern const light_gray{.data = {std::byte{0x88}, std::byte{0x42}, std::byte{0x88}, std::byte{0x42}, std::byte{0x88},
                                  std::byte{0x42}, std::byte{0x88}, std::byte{0x42}}};

void bitmap::paint_rect(rect const& r, pattern const& pat) {
  if (r.bottom < r.top || r.right < r.left || r.top >= height_) {
    return;
  }
  auto const y0 = static_cast<unsigned>(r.top);
  auto const y1 = std::min(static_cast<unsigned>(r.bottom), height_ - 1U);
  for (auto y = y0; y <= y1; ++y) {
    this->line_horizontal(r.left, r.right, y, pat.data[y % 8]);
  }
}

point draw_char(bitmap& dest, glyph_cache& gc, char32_t code_point, point pos) {
  bitmap const& bm = gc.get(code_point);
  dest.copy(bm, pos);
  return point{static_cast<decltype(point::x)>(pos.x + bm.width() + gc.spacing()), pos.y};
}

point draw_string(bitmap& dest, glyph_cache& gc, std::u8string_view s, point pos) {
  icubaby::t8_32 transcoder;
  std::array<char32_t, 1> code_point_buffer{};

  auto const begin = std::begin(code_point_buffer);
  for (auto const cu : s) {
    if (auto const it = transcoder(cu, begin); it != begin) {
      // We have a code point.
      assert(it == begin + 1);
      pos = draw_char(dest, gc, *begin, pos);
    }
  }
  if (auto const it = transcoder.end_cp(begin); it != begin) {
    assert(it == begin + 1);
    pos = draw_char(dest, gc, *begin, pos);
  }

  return pos;
}

std::tuple<std::unique_ptr<std::byte[]>, draw::bitmap> create_bitmap_and_store(unsigned width, unsigned height) {
  auto const size = bitmap::required_store_size(width, height);
  auto store = std::make_unique<std::byte[]>(size);
  auto* const ptr = store.get();
  return std::tuple(std::move(store), bitmap{std::span{ptr, ptr + size}, width, height});
}

}  // end namespace draw
