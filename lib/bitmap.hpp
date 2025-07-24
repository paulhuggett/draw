#ifndef BITMAP_HPP
#define BITMAP_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <print>
#include <ranges>
#include <vector>

namespace draw {

using ordinate = std::uint8_t;
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
  [[nodiscard]] constexpr rect inset(int dy, int dx) const noexcept {
    if (top + dy > bottom - dy || left + dx > right - dx) {
      return {0, 0, 0, 0};
    }
    return {
      .top=static_cast<ordinate>(top + dy),
      .left=static_cast<ordinate>(left + dx),
      .bottom=static_cast<ordinate>(bottom - dy),
      .right=static_cast<ordinate>(right - dy)
    };
  }
};
struct pattern {
  std::array<std::byte, 8> data;
};

class bitmap {
  friend class glyph_cache;

public:
  bitmap(unsigned width, unsigned height, unsigned stride)
      : width_{width}, height_{height}, stride_{stride}, data_{(std::size_t{height * stride})} {}
  bitmap(unsigned width, unsigned height) : bitmap(width, height, (width + 7U) / 8U) {}

  void copy(bitmap const& source, point dest_pos);
  void clear() { std::ranges::fill(data_, std::byte{0}); }
  bool set(point p) {
    if (p.x >= width_ || p.y >= height_) {
      return false;
    }
    auto const index = p.y * stride_ + p.x / 8U;
    assert(index < data_.size());
    data_[index] |= std::byte{0x80} >> (p.x % 8U);
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

  [[nodiscard]] constexpr std::ranges::subrange<std::vector<std::byte>::const_iterator> store() const noexcept {
    return {data_.begin(), data_.end(), data_.size()};
  }
  void dump(std::FILE* stream = stdout) const;

private:
  unsigned width_ = 0;   ///< Width of the bitmap in pixels
  unsigned height_ = 0;  ///< Height of the bitmap in pixels
  unsigned stride_ = 0;  ///< Number of bytes per row
  std::vector<std::byte> data_;

  void line_horizontal(unsigned x0, unsigned x1, unsigned y, std::byte const pattern);
  void line_vertical(unsigned x, unsigned y0, unsigned y1);
  //void line_octant0(point pos, unsigned dx, unsigned dy, int x_direction);
  //void line_octant1(point pos, unsigned dx, unsigned dy, int x_direction);
  void plot_line(unsigned x0, unsigned y0, unsigned x1, unsigned y1);
};

extern pattern const black;
extern pattern const white;
extern pattern const grey;
extern pattern const light_grey;

class glyph_cache;
point draw_string(bitmap& dest, glyph_cache& gc, std::u8string_view s, point pos);
point draw_char(bitmap& dest, glyph_cache& gc, char32_t code_point, point pos);

}  // end namespace draw

#endif  // BITMAP_HPP
