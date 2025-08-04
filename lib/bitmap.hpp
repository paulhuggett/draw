#ifndef BITMAP_HPP
#define BITMAP_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdlib>
#include <limits>
#include <memory>
#include <ranges>
#include <span>
#include <tuple>

#include "types.hpp"

struct font;

namespace draw {

class glyph_cache;

class bitmap {
  friend class glyph_cache;

public:
  constexpr bitmap() noexcept = default;
  constexpr bitmap(std::span<std::byte> store, std::uint16_t width, std::uint16_t height, std::uint16_t stride) noexcept
      : store_{store}, width_{width}, height_{height}, stride_{stride} {
    assert(store.size() >= this->actual_store_size());
    constexpr auto max = static_cast<std::uint16_t>(std::numeric_limits<ordinate>::max());
    assert(width <= max);
    assert(height <= max);
  }
  constexpr bitmap(std::span<std::byte> store, std::uint16_t width, std::uint16_t height) noexcept
      : bitmap(store, width, height, required_stride(width)) {}

  static constexpr std::uint16_t required_stride(std::uint16_t width) noexcept {
    assert(width <= static_cast<std::uint16_t>(std::numeric_limits<ordinate>::max()));
    return (width + 7U) / 8U;
  }
  /// Returns the store size required for a bitmap with the supplied dimensions.
  static constexpr std::size_t required_store_size(std::uint16_t width, std::uint16_t height) noexcept {
    assert(height <= static_cast<std::uint16_t>(std::numeric_limits<ordinate>::max()));
    return required_stride(width) * height;
  }

  enum class transfer_mode { mode_copy, mode_or };
  void copy(bitmap const& source, point dest_pos, transfer_mode mode);
  void clear() { std::ranges::fill(this->store(), std::byte{0}); }
  bool set(point p, bool new_state);
  /// \param p0  Coordinate of one end of the line
  /// \param p1  Coordinate of the other end of the line
  void line(point p0, point p1);

  void frame_rect(rect const& r);
  void paint_rect(rect const& r, pattern const& pat);

  void draw_char(glyph_cache& gc, char32_t code_point, point pos);
  point draw_string(glyph_cache& gc, std::u8string_view s, point pos);

  [[nodiscard]] constexpr std::uint16_t width() const noexcept { return width_; }
  [[nodiscard]] constexpr std::uint16_t height() const noexcept { return height_; }
  [[nodiscard]] constexpr std::uint16_t stride() const noexcept { return stride_; }
  [[nodiscard]] constexpr rect bounds() const noexcept {
    return {.top = 0, .left = 0, .bottom = static_cast<ordinate>(height()), .right = static_cast<ordinate>(width())};
  }
  [[nodiscard]] constexpr std::span<std::byte const> store() const noexcept { return store_; }
  [[nodiscard]] constexpr std::span<std::byte> store() noexcept { return store_; }

  void dump(std::FILE* stream = stdout) const;

private:
  std::span<std::byte> store_;
  std::uint16_t width_ = 0;   ///< Width of the bitmap in pixels
  std::uint16_t height_ = 0;  ///< Height of the bitmap in pixels
  std::uint16_t stride_ = 0;  ///< Number of bytes per row

  void tranfer(std::byte& dest, std::byte const src_pixel, std::byte const dest_pixel, transfer_mode mode);
  constexpr std::size_t actual_store_size() const { return stride_ * height_; }
  void line_horizontal(std::uint16_t x0, std::uint16_t x1, std::uint16_t y, std::byte const pattern);
  void line_vertical(std::uint16_t x, std::uint16_t y0, std::uint16_t y1);
};

inline bool bitmap::set(point const p, bool const new_state) {
  if (p.x < 0 || p.y < 0) {
    return false;
  }
  auto const x = static_cast<unsigned>(p.x);
  auto const y = static_cast<unsigned>(p.y);
  if (x >= width_ || y >= height_) {
    return false;
  }
  auto const index = y * stride_ + x / 8U;
  assert(index < this->actual_store_size());
  auto& b = store_[index];
  auto const bit = std::byte{0x80} >> (x % 8U);
  // TODO: Checkout <https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching>
  if (new_state) {
    b |= bit;
  } else {
    b &= ~bit;
  }
  return true;
}

std::tuple<std::unique_ptr<std::byte[]>, draw::bitmap> create_bitmap_and_store(std::uint16_t width,
                                                                               std::uint16_t height);

extern pattern const black;
extern pattern const white;
extern pattern const gray;
extern pattern const light_gray;

ordinate string_width(glyph_cache& gc, std::u8string_view s);

}  // end namespace draw

#endif  // BITMAP_HPP
