#include "bitmap.hpp"

#include <cassert>
#include <cstring>
#include <print>
#include <utility>

#include "glyph_cache.hpp"
#include "icubaby.hpp"

using namespace draw::literals;
using draw::bitmap;

namespace {

void memor(std::byte* dest, std::byte const* src, std::size_t len) {
  for (; len > 0; --len) {
    *(dest++) |= *(src++);
  }
}

void copy_row_aligned(unsigned src_x, unsigned src_x_end, std::byte const* const src_row, unsigned dest_x,
                      std::byte* const dest_row, bitmap::transfer_mode mode) {
  using enum bitmap::transfer_mode;
  assert(src_x % 8U == dest_x % 8U);

  auto const* src = src_row + (src_x / 8U);
  auto* dest = dest_row + (dest_x / 8U);
  auto const len = (src_x_end / 8U) - (src_x / 8U);
  switch (mode) {
  case mode_copy: std::memcpy(dest, src, len); break;
  case mode_or: memor(dest, src, len); break;
  default: assert(false && "unknown transfer mode"); break;
  }
  dest += len;
  src += len;
  src_x += len * 8U;
  assert(src_x + 8U > src_x_end && "There should be less than a whole byte left to copy");
  if (src_x < src_x_end) {
    auto const mask = 0xFF_b << (8U - (src_x_end % 8U));
    switch (mode) {
    case mode_or: *dest |= *src & mask; break;
    case mode_copy: *dest = (*src & mask) | (*dest & ~mask); break;
    default: assert(false && "unknown transfer mode"); break;
    }
  }
  return;
}

void copy_row_tiny(unsigned src_x, unsigned src_x_end, std::byte const* const src_row, unsigned dest_x,
                   std::byte* const dest_row, bitmap::transfer_mode mode) {
  assert(src_x % 8U != dest_x % 8U);
  // There's less than a byte to copy.
  // TODO: don't do this one pixel at a time.
  auto const* src = src_row + (src_x / 8U);
  for (; src_x < src_x_end; ++src_x, ++dest_x) {
    auto const src_bit = *src & (0x80_b >> (src_x % 8U));
    auto const dest_bit = 0x80_b >> (dest_x % 8U);

    auto* const dest = dest_row + (dest_x / 8U);
    auto out = (static_cast<std::byte>(-static_cast<unsigned>(src_bit != 0_b)) & dest_bit);
    switch (mode) {
    case bitmap::transfer_mode::mode_or: *dest |= out; break;
    case bitmap::transfer_mode::mode_copy: *dest = (*dest & ~dest_bit) | out; break;
    default: assert(false && "unknown transfer mode"); break;
    }
  }
}

template <bool Trace> void trace_source(unsigned src_x, unsigned src_x_end, std::byte const* const src_row);
template <>
[[maybe_unused]] void trace_source<true>(unsigned src_x, unsigned src_x_end, std::byte const* const src_row) {
  for (auto x = src_x; x < src_x_end; ++x) {
    if (x % 8 == 0) {
      std::print("'");
    }
    std::print("{:c}", (src_row[x / 8] & (0x80_b >> (x % 8))) != std::byte{0} ? '1' : '0');
  }
  std::print("    ");
}
template <> [[maybe_unused]] void trace_source<false>(unsigned, unsigned, std::byte const* const) {
  // Just do nothing.
}

void transfer(std::byte* const dest, std::byte mask, std::byte v, bitmap::transfer_mode mode) {
  using enum bitmap::transfer_mode;
  switch (mode) {
  case mode_or: *dest |= v; break;
  case mode_copy: *dest = (*dest & ~mask) | v; break;
  default: assert(false && "unknown transfer mode"); break;
  }
}

void copy_row_misaligned(unsigned src_x, unsigned src_x_end, std::byte const* const src_row, unsigned dest_x,
                         std::byte* const dest_row, bitmap::transfer_mode mode) {
  using enum bitmap::transfer_mode;
  assert(src_x % 8U != dest_x % 8U);
  assert(src_x + 8 <= src_x_end);

  auto const* src = src_row + (src_x / 8U);
  auto* dest = dest_row + (dest_x / 8U);

  constexpr bool trace = false;
  trace_source<trace>(src_x, src_x_end, src_row);

  auto const m = dest_x % 8U;
  auto const mask_high = 0xFF_b << m;
  auto const mask_low = 0xFF_b >> m;

  if (src_x + 8U <= src_x_end) {
    // The initial partial byte.
    transfer(dest, mask_low, (*src & mask_high) >> m, mode);
    if constexpr (trace) {
      std::print("{:08b}'", std::to_underlying(*dest));
    }

    ++dest;
    src_x += 8U - m;
    dest_x += 8U - m;

    // Copying a byte at a time.
    while (src_x + 8U <= src_x_end) {
      transfer(dest, std::byte{0xFF}, ((*src & ~mask_high) << (8U - m)) | ((*(src + 1) & mask_high) >> m), mode);
      if constexpr (trace) {
        std::print("{:08b}'", std::to_underlying(*dest));
      }

      ++dest;
      ++src;
      src_x += 8U;
      dest_x += 8U;
    }
  }
  // The final partial byte. We have fewer than eight bits of the source remaining.
  assert(src_x <= src_x_end);
  if (auto const remaining = src_x_end - src_x; remaining > 0) {
    assert(remaining <= 8);

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
  }

  if constexpr (trace) {
    std::println("{:08b}", std::to_underlying(*dest));
  }
}

void copy_row(unsigned src_x_init, unsigned src_x_end, std::byte const* const src_row, unsigned dest_x,
              std::byte* const dest_row, bitmap::transfer_mode mode) {
  assert(src_x_init <= src_x_end);
  if (src_x_init % 8U == dest_x % 8U) {
    copy_row_aligned(src_x_init, src_x_end, src_row, dest_x, dest_row, mode);
  } else {
    if (src_x_init + 8 > src_x_end) {
      copy_row_tiny(src_x_init, src_x_end, src_row, dest_x, dest_row, mode);
    } else {
      copy_row_misaligned(src_x_init, src_x_end, src_row, dest_x, dest_row, mode);
    }
  }
}

}  // namespace

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

void bitmap::copy(bitmap const& source, point dest_pos, transfer_mode mode) {
  // An initial gross clipping check.
  if ((dest_pos.x >= static_cast<int>(width_)) || (dest_pos.x + static_cast<int>(source.width()) < 0) ||
      (dest_pos.y >= static_cast<int>(height_)) || (dest_pos.y + static_cast<int>(source.height()) < 0)) {
    return;
  }

  auto dest_y = static_cast<unsigned>(std::max(dest_pos.y, ordinate{0}));
  auto const src_y_init = dest_pos.y < 0 ? static_cast<unsigned>(-dest_pos.y) : 0U;
  auto const src_y_end = std::min(static_cast<unsigned>(source.height_), src_y_init + height_ - dest_y);

  auto const src_x_init = dest_pos.x >= 0 ? 0U : static_cast<unsigned>(-dest_pos.x);
  assert(width_ - std::max(dest_pos.x, ordinate{0}) >= 0);
  auto const src_x_end = std::min(static_cast<unsigned>(source.width_),
                                  src_x_init + static_cast<unsigned>(width_ - std::max(dest_pos.x, ordinate{0})));

  auto dest_x = static_cast<unsigned>(std::max(dest_pos.x, ordinate{0}));
  for (auto src_y = src_y_init; src_y < src_y_end; ++src_y, ++dest_y) {
    copy_row(src_x_init, src_x_end, &source.store_[src_y * source.stride_], dest_x, &store_[dest_y * stride_], mode);
  }
}

void bitmap::line_horizontal(std::uint16_t x0, std::uint16_t x1, std::uint16_t const y, std::byte const pattern) {
  if (x0 > x1) {
    std::swap(x0, x1);  // Ensure that we always go from lower to higher addresses.
  }
  // A gross clipping check.
  if (x0 >= width_ || y >= height_) {
    return;
  }
  // Clamp x1 to the bitmap's right edge.
  x1 = std::min(x1, static_cast<std::uint16_t>(width_ - 1U));
  auto it = store_.begin() + y * stride_ + x0 / 8U;
  assert(it < store_.end() && "iterator is not within the bitmap");

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
  for (; bytes > 0; --bytes) {
    assert(it < store_.end() && "iterator is not within the bitmap");
    *it = pattern;
    ++it;
  }
  // The final part of the line.
  assert(it < store_.end() && "iterator is not within the bitmap");
  *it = (*it & ~mask_high) | (mask_high & pattern);
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
  auto const bits = 0x80_b >> (x % 8);
  for (auto y = y0; y < y1; ++y) {
    assert(index < store_.size() && "index is not within the bitmap");
    store_[index] |= bits;
    index += stride_;
  }
}

void bitmap::line(point p0, point p1) {
  if (p0.y == p1.y) {
    if (p0.y >= 0 && p0.y < height_) {
      this->line_horizontal(static_cast<std::uint16_t>(std::max(p0.x, ordinate{0})),
                            static_cast<std::uint16_t>(std::max(p1.x, ordinate{0})), static_cast<std::uint16_t>(p0.y),
                            0xFF_b);
    }
    return;
  }
  if (p0.x == p1.x) {
    if (p0.x >= 0 && p0.x < width_) {
      this->line_vertical(static_cast<std::uint16_t>(p0.x), static_cast<std::uint16_t>(std::max(p0.y, ordinate{0})),
                          static_cast<std::uint16_t>(std::max(p1.y, ordinate{0})));
    }
    return;
  }

  auto const sx = p0.x < p1.x ? ordinate{1} : ordinate{-1};
  auto const sy = p0.y < p1.y ? ordinate{1} : ordinate{-1};
  auto const dx = std::abs(static_cast<int>(p1.x) - static_cast<int>(p0.x));
  auto const dy = -std::abs(static_cast<int>(p1.y) - static_cast<int>(p0.y));
  auto err = dx + dy;

  for (;;) {
    this->set(point{.x = p0.x, .y = p0.y}, true);
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

pattern const black{.data = {0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b}};
pattern const white{.data = {0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b}};
pattern const gray{.data = {0xAA_b, 0x55_b, 0xAA_b, 0x55_b, 0xAA_b, 0x55_b, 0xAA_b, 0x55_b}};
pattern const light_gray{.data = {0x88_b, 0x42_b, 0x88_b, 0x42_b, 0x88_b, 0x42_b, 0x88_b, 0x42_b}};

void bitmap::paint_rect(rect const& r, pattern const& pat) {
  if (r.bottom < r.top || r.right < r.left || r.bottom < 0 || r.right < 0) {
    return;
  }
  if (r.top >= 0 && static_cast<unsigned>(r.top) >= height_) {
    return;
  }
  auto const x0 = static_cast<std::uint16_t>(std::max(r.left, ordinate{0}));
  auto const x1 = static_cast<std::uint16_t>(std::max(r.right, ordinate{0}));
  auto const y0 = static_cast<std::uint16_t>(std::max(r.top, ordinate{0}));
  auto const y1 = std::min(static_cast<std::uint16_t>(r.bottom), static_cast<std::uint16_t>(height_ - 1U));
  for (auto y = y0; y <= y1; ++y) {
    this->line_horizontal(x0, x1, y, pat.data[y % 8]);
  }
}

std::uint16_t bitmap::char_width(glyph_cache const& gc, char32_t code_point) {
  font::glyph const* const g = gc.find_glyph(code_point);
  assert(g != nullptr);
  return gc.get_font()->width(*g);
}

void bitmap::draw_char(glyph_cache& gc, char32_t code_point, point pos) {
  if (pos.x > this->width() || pos.y > this->height()) {
    return;
  }
  this->copy(gc.get(code_point), pos, transfer_mode::mode_or);
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

template <typename DrawFn>
static ordinate scan_code_point(ordinate x, glyph_cache& gc, char32_t code_point,
                                std::optional<char32_t> prev_code_point, DrawFn draw) {
  font::glyph const* const g = gc.find_glyph(code_point);
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

point bitmap::draw_string(glyph_cache& gc, std::u8string_view s, point pos) {
  ordinate const new_x = scan_string(gc, s, [this, &gc, &pos](char32_t code_point, ordinate x) {
    this->draw_char(gc, code_point, point{.x = static_cast<ordinate>(pos.x + x), .y = pos.y});
  });
  return point{.x = static_cast<ordinate>(pos.x + new_x), .y = pos.y};
}

ordinate string_width(glyph_cache& gc, std::u8string_view s) {
  return scan_string(gc, s, [](char32_t /*code_point*/, ordinate /*x*/) {});
}

}  // end namespace draw
