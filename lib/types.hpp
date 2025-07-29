#ifndef TYPES_HPP
#define TYPES_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

namespace draw {

namespace literals {

consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}

}  // end namespace literals

using ordinate = std::int16_t;

struct point {
  ordinate x = 0;
  ordinate y = 0;
};

struct rect {
  ordinate top = 0;
  ordinate left = 0;
  ordinate bottom = 0;
  ordinate right = 0;

  constexpr bool operator==(rect const &) const noexcept = default;

  [[nodiscard]] constexpr ordinate width() const noexcept { return right > left ? right - left : 0; }
  [[nodiscard]] constexpr ordinate height() const noexcept { return bottom > top ? bottom - top : 0; }
  [[nodiscard]] constexpr bool empty() const noexcept { return bottom <= top || right <= left; }

  /// Shrinks or expands the rectangle.
  ///
  /// The left and right sides are moved in by dx; the top and bottom in by dy.
  /// If the resulting width or height becomes less than 1, the empty rectangle iis set to the empty rectangle.
  ///
  /// \param dx  The distance by which the left position is incremented and the right decremented
  /// \param dy  The distance by which the top position is incremented and the bottom decremented
  /// \returns  The new rectangle
  [[nodiscard]] constexpr rect inset(ordinate dx, ordinate dy) const noexcept {
    auto t = top + dy;
    auto l = left + dx;
    auto b = bottom - dy;
    auto r = right - dy;
    if (b <= t || r <= l) {
      t = l = b = r = 0;
    }
    return {
        .top = static_cast<ordinate>(t),
        .left = static_cast<ordinate>(l),
        .bottom = static_cast<ordinate>(b),
        .right = static_cast<ordinate>(r),
    };
  }
};

struct pattern {
  std::array<std::byte, 8> data;
};

}  // end namespace draw

#endif  // TYPES_HPP
