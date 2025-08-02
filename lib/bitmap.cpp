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
void bitmap::copy(bitmap const& source, point dest_pos, transfer_mode mode) {
  using uordinate = std::make_unsigned_t<ordinate>;
  // An initial gross clipping check.
  if ((dest_pos.x > static_cast<int>(width_)) || (dest_pos.x + static_cast<int>(source.width()) < 0) ||
      (dest_pos.y > static_cast<int>(height_)) || (dest_pos.y + static_cast<int>(source.height()) < 0)) {
    return;
  }

  auto dest_y = std::max(dest_pos.y, ordinate{0});
  auto const src_y_init = dest_pos.y < 0 ? static_cast<uordinate>(-dest_pos.y) : uordinate{0};
  auto const src_y_end = std::min(static_cast<int>(source.height_), src_y_init + height_ - dest_y);

  auto const src_x_init = dest_pos.x >= 0 ? 0 : -dest_pos.x;
  auto const src_x_end =
      std::min(static_cast<int>(source.width_), src_x_init + width_ - std::max(dest_pos.x, ordinate{0}));

  for (auto src_y = src_y_init; src_y < src_y_end; ++src_y, ++dest_y) {
    auto dest_x = std::max(dest_pos.x, ordinate{0});

    for (auto src_x = src_x_init; src_x < src_x_end; ++src_x, ++dest_x) {
      assert(src_x >= 0);
      auto const src_index = (src_y * source.stride_) + (src_x / 8U);
      assert(src_index < source.store_.size());
      auto const src_pixel = source.store_[src_index] & (std::byte{0x80} >> (src_x % 8U));

      auto const dest_index = (dest_y * stride_) + (dest_x / 8U);
      assert(dest_index < store_.size());
      auto const dest_pixel = std::byte{0x80} >> (dest_x % 8U);

      switch (mode) {
      case transfer_mode::mode_or:
        if (src_pixel != std::byte{0}) {
          store_[dest_index] |= dest_pixel;
        }
        break;
      case transfer_mode::mode_copy:
        if (src_pixel != std::byte{0}) {
          store_[dest_index] |= dest_pixel;
        } else {
          store_[dest_index] &= ~dest_pixel;
        }
        break;
      default: assert(false && "unknown transfer mode"); break;
      }
    }
  }
}

void bitmap::line_horizontal(std::uint16_t x0, std::uint16_t x1, std::uint16_t y, std::byte const pattern) {
  if (x0 > x1) {
    std::swap(x0, x1);
  }
  if (x0 >= width_ || y >= height_) {
    return;
  }
  x1 = std::min(static_cast<std::uint16_t>(x1 + 1U), width_);
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

void bitmap::line_vertical(std::uint16_t x, std::uint16_t y0, std::uint16_t y1) {
  if (x >= width_) {
    return;
  }
  if (y0 > y1) {
    std::swap(y0, y1);
  }
  if (y0 >= height_) {
    return;
  }
  y1 = std::min(static_cast<std::uint16_t>(y1 + 1U), height_);
  assert(y0 < y1);

  auto index = static_cast<unsigned>(y0 * stride_ + x / 8);
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

  auto const sx = p0.x < p1.x ? 1 : -1;
  auto const sy = p0.y < p1.y ? 1 : -1;
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

std::tuple<std::unique_ptr<std::byte[]>, draw::bitmap> create_bitmap_and_store(std::uint16_t width,
                                                                               std::uint16_t height) {
  auto const size = bitmap::required_store_size(width, height);
  auto store = std::make_unique<std::byte[]>(size);
  auto* const ptr = store.get();
  return std::tuple(std::move(store), bitmap{std::span{ptr, ptr + size}, width, height});
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
  if (r.bottom < r.top || r.right < r.left || r.top < 0) {
    return;
  }
  if (static_cast<unsigned>(r.top) >= height_) {
    return;
  }
  auto const y0 = static_cast<unsigned>(r.top);
  auto const y1 = std::min(static_cast<unsigned>(r.bottom), height_ - 1U);
  for (auto y = y0; y <= y1; ++y) {
    this->line_horizontal(r.left, r.right, y, pat.data[y % 8]);
  }
}

static ordinate glyph_spacing(glyph_cache& gc, font::glyph const& g, std::optional<char32_t> prev_code_point) {
  if (!prev_code_point.has_value()) {
    return 0;
  }
  ordinate space = gc.spacing();

  auto const kerning_pairs = std::get<std::span<kerning_pair const>>(g);
  auto const kerning_pairs_end = std::end(kerning_pairs);
  if (auto const kern_pos =
          std::find_if(std::begin(kerning_pairs), kerning_pairs_end,
                       [&prev_code_point](kerning_pair const& kp) { return kp.preceeding == prev_code_point; });
      kern_pos != kerning_pairs_end) {
    space -= kern_pos->distance;
  }
  return space;
}

void draw_char(bitmap& dest, glyph_cache& gc, char32_t code_point, point pos) {
  if (pos.x > dest.width() || pos.y > dest.height()) {
    return;
  }
  bitmap const& bm = gc.get(code_point);
  dest.copy(bm, pos, bitmap::transfer_mode::mode_or);
}

template <typename DrawFn>
static ordinate scan_code_point(ordinate x, glyph_cache& gc, char32_t code_point,
                                std::optional<char32_t> prev_code_point, DrawFn draw) {
  font::glyph const* g = gc.find_glyph(code_point);
  x += glyph_spacing(gc, *g, prev_code_point);
  draw(code_point, x);
  x += gc.get_font()->width(*g);
  return x;
}

template <typename DrawFn> static ordinate scan_string(glyph_cache& gc, std::u8string_view s, DrawFn draw) {
  auto x = ordinate{0};

  std::optional<char32_t> prev_cp;

  icubaby::t8_32 transcoder;
  std::array<char32_t, 1> code_point_buffer{};

  auto const begin = std::begin(code_point_buffer);
  for (auto const cu : s) {
    if (auto const it = transcoder(cu, begin); it != begin) {
      // We have a code point.
      assert(std::distance(begin, it) == 1);
      x = scan_code_point(x, gc, *begin, prev_cp, draw);
      prev_cp = *begin;
    }
  }
  if (auto const it = transcoder.end_cp(begin); it != begin) {
    // We have a code point.
    assert(std::distance(begin, it) == 1);
    x = scan_code_point(x, gc, *begin, prev_cp, draw);
  }

  return x;
}

point draw_string(bitmap& dest, glyph_cache& gc, std::u8string_view s, point pos) {
  ordinate const new_x = scan_string(gc, s, [&gc, &dest, &pos](char32_t code_point, ordinate x) {
    draw_char(dest, gc, code_point, point{.x = static_cast<ordinate>(pos.x + x), .y = pos.y});
  });
  return point{.x = static_cast<ordinate>(pos.x + new_x), .y = pos.y};
}

ordinate string_width(glyph_cache& gc, std::u8string_view s) {
  return scan_string(gc, s, [](char32_t /*code_point*/, ordinate /*x*/) {});
}

}  // end namespace draw
