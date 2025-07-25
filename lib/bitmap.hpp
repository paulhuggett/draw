#ifndef BITMAP_HPP
#define BITMAP_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <print>
#include <ranges>
#include <tuple>

struct font;

namespace draw {

using ordinate = std::uint16_t;
struct point {
  ordinate x = 0;
  ordinate y = 0;
};
struct rect {
  ordinate top;
  ordinate left;
  ordinate bottom;
  ordinate right;

  [[nodiscard]] constexpr ordinate width() const noexcept { return right > left ? right - left : 0; }
  [[nodiscard]] constexpr ordinate height() const noexcept { return bottom > top ? bottom - top : 0; }
  [[nodiscard]] constexpr bool empty() const noexcept { return top > bottom || left > right; }

  /// Shrinks or expands the rectangle.
  ///
  /// The left and right sides are moved in by dx; the top and bottom in by dy.
  /// If the resulting width or height becomes less than 1, the empty rectangle iis set to the empty rectangle.
  [[nodiscard]] constexpr rect inset(int dx, int dy) const noexcept {
    auto t = top + dy;
    auto l = left + dx;
    auto b = bottom - dy;
    auto r = right - dy;
    if (t > b || l > r) {
      t = l = b = r = 0;
    }
    return {
        static_cast<ordinate>(t),
        static_cast<ordinate>(l),
        static_cast<ordinate>(b),
        static_cast<ordinate>(r),
    };
  }
};
struct pattern {
  std::array<std::byte, 8> data;
};

class bitmap {
  friend class glyph_cache;

public:
  bitmap() = default;
  bitmap(std::span<std::byte> store, unsigned width, unsigned height, unsigned stride)
      : store_{store}, width_{width}, height_{height}, stride_{stride} {
    assert(store.size() >= this->actual_store_size());
  }
  bitmap(std::span<std::byte> store, unsigned width, unsigned height)
      : bitmap(store, width, height, (width + 7U) / 8U) {}

  /// Returns the store size required for a bitmap with the supplied dimensions.
  static constexpr std::size_t required_store_size(unsigned width, unsigned height) noexcept {
    return (width + 7U) / 8U * height;
  }

  void copy(bitmap const& source, point dest_pos);
  void clear() { std::ranges::fill(this->store(), std::byte{0}); }
  bool set(point p, bool new_state) {
    if (p.x >= width_ || p.y >= height_) {
      return false;
    }
    auto const index = p.y * stride_ + p.x / 8U;
    assert(index < this->actual_store_size());
    auto& b = store_[index];
    auto const bit = std::byte{0x80} >> (p.x % 8U);
    if (new_state) {
      b |= bit;
    } else {
      b &= ~bit;
    }
    return true;
  }
  /// \param p0  Coordinate of one end of the line
  /// \param p1  Coordinate of the other end of the line
  void line(point p0, point p1);

  void frame_rect(rect const& r);
  void paint_rect(rect const& r, pattern const& pat);

  [[nodiscard]] constexpr unsigned width() const noexcept { return width_; }
  [[nodiscard]] constexpr unsigned height() const noexcept { return height_; }
  [[nodiscard]] constexpr unsigned stride() const noexcept { return stride_; }

  [[nodiscard]] constexpr std::span<std::byte const> store() const noexcept { return store_; }
  [[nodiscard]] constexpr std::span<std::byte> store() noexcept { return store_; }

  void dump(std::FILE* stream = stdout) const;

private:
  std::span<std::byte> store_;
  unsigned width_ = 0;   ///< Width of the bitmap in pixels
  unsigned height_ = 0;  ///< Height of the bitmap in pixels
  unsigned stride_ = 0;  ///< Number of bytes per row

  constexpr std::size_t actual_store_size() const { return stride_ * height_; }
  void line_horizontal(unsigned x0, unsigned x1, unsigned y, std::byte const pattern);
  void line_vertical(unsigned x, unsigned y0, unsigned y1);
};

std::tuple<std::unique_ptr<std::byte[]>, draw::bitmap> create_bitmap_and_store(unsigned width, unsigned height);

extern pattern const black;
extern pattern const white;
extern pattern const gray;
extern pattern const light_gray;

class glyph_cache;
point draw_char(bitmap& dest, glyph_cache& gc, char32_t code_point, point pos);
point draw_string(bitmap& dest, glyph_cache& gc, std::u8string_view s, point pos);

}  // end namespace draw

#endif  // BITMAP_HPP
