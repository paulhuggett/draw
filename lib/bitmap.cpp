#include "bitmap.hpp"

#include <cassert>
#include <utility>

#include "glyph_cache.hpp"
#include "icubaby.hpp"

namespace draw {

void bitmap::dump(std::FILE* const stream) const {
  auto xb = 0U;  // The x ordinate (in bytes)
  for (auto const d : data_) {
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
      assert(src_index < source.data_.size());
      auto const dest_index = (dest_y * stride_) + (dest_x / 8U);
      assert(dest_index < data_.size());
      auto const src_pixel = source.data_[src_index] & (std::byte{0x80} >> (src_x % 8U));
      auto const dest_value = std::byte{0x80} >> (dest_x % 8U);
      if (src_pixel != std::byte{0}) {
        data_[dest_index] |= dest_value;
      } else {
        data_[dest_index] &= ~dest_value;
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
  x1 = std::min(x1, width_ - 1U);
  auto byte_index = y * stride_ + x0 / 8U;
  assert(byte_index < data_.size() && "index is not within the bitmap");
  {
    auto const mask0 = static_cast<std::byte>((1U << (8 - (x0 % 8))) - 1U);
    auto& b = data_[byte_index];
    b = (b & ~mask0) | (mask0 & pattern);
  }

  auto xbyte = (x0 / 8U) + 1U;
  auto last_xbyte = (x1 + 1U) / 8U;
  ++byte_index;
  for (; xbyte < last_xbyte; ++xbyte) {
    assert(byte_index < data_.size() && "index is not within the bitmap");
    data_[byte_index] = pattern;  // std::byte{0xFF};
    ++byte_index;
  }

  if (auto const num_bits = (x1 + 1U) % 8U; num_bits > 0U) {
    assert(byte_index < data_.size() && "index is not within the bitmap");
    auto const mask1 = static_cast<std::byte>((1U << num_bits) - 1U) << (8 - num_bits);
    auto& b = data_[byte_index];
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
  y1 = std::min(y1, height_);
  assert(y0 <= y1);

  auto index = y0 * stride_ + x / 8;
  auto const bits = std::byte{0x80} >> (x % 8);
  for (auto y = y0; y <= y1; ++y) {
    assert(index < data_.size() && "index is not within the bitmap");
    data_[index] |= bits;
    index += stride_;
  }
}

void bitmap::plot_line(unsigned x0, unsigned y0, unsigned x1, unsigned y1) {
  int dx = std::abs((int)x1 - (int)x0);
  int sx = x0 < x1 ? 1 : -1;
  int dy = -std::abs((int)y1 - (int)y0);
  int sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;

  for (;;) {
    this->set(point{.x = static_cast<ordinate>(x0), .y = static_cast<ordinate>(y0)});
    auto e2 = err * 2;
    if (e2 >= dy) {
      if (x0 == x1) {
        break;
      }
      err += dy;
      x0 += sx;
    }

    if (e2 <= dx) {
      if (y0 == y1) {
        break;
      }
      err += dx;
      y0 += sy;
    }
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
  int dx = std::abs(static_cast<int>(x1) - static_cast<int>(x0));
  int sx = x0 < x1 ? 1 : -1;
  int dy = -std::abs(static_cast<int>(y1) - static_cast<int>(y0));
  int sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;

  for (;;) {
    this->set(point{.x = static_cast<ordinate>(x0), .y = static_cast<ordinate>(y0)});
    auto e2 = err * 2;
    if (e2 >= dy) {
      if (x0 == x1) {
        break;
      }
      err += dy;
      x0 += sx;
    }

    if (e2 <= dx) {
      if (y0 == y1) {
        break;
      }
      err += dx;
      y0 += sy;
    }
  }
}

void bitmap::frame_rect(rect const& r) {
  auto x0 = r.left;
  auto x1 = r.right;
  auto y0 = r.top;
  auto y1 = r.bottom;
  // Normalize the rectangle.
  if (x0 > x1) {
    std::swap(x0, x1);
  }
  if (y0 > y1) {
    std::swap(y0, y1);
  }

  // Draw the top and bottom lines.
  this->line(point{.x = x0, .y = y0}, point{.x = x1, .y = y0});
  this->line(point{.x = x0, .y = y1}, point{.x = x1, .y = y1});

  // Draw the left and right lines.
  this->line(point{.x = x0, .y = y0}, point{.x = x0, .y = y1});
  this->line(point{.x = x1, .y = y0}, point{.x = x1, .y = y1});
}

pattern const black{.data = {std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF},
                             std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}}};
pattern const white{.data = {std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
                             std::byte{0x00}, std::byte{0x00}, std::byte{0x00}}};
pattern const grey{.data = {std::byte{0xAA}, std::byte{0x55}, std::byte{0xAA}, std::byte{0x55}, std::byte{0xAA},
                            std::byte{0x55}, std::byte{0xAA}, std::byte{0x55}}};
pattern const light_grey{.data = {std::byte{0x88}, std::byte{0x42}, std::byte{0x88}, std::byte{0x42}, std::byte{0x88},
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

}  // end namespace draw
