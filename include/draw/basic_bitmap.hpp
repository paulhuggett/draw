#ifndef BASIC_BITMAP_HPP
#define BASIC_BITMAP_HPP

#include <optional>
#include <span>
#if defined(DRAW_HOSTED) && DRAW_HOSTED
#include <cstdio>
#endif

#include "draw/types.hpp"

namespace draw {

extern pattern const black;
extern pattern const white;
extern pattern const gray;
extern pattern const light_gray;

struct font;
class glyph_cache;

enum class transfer_mode : std::uint8_t { mode_copy, mode_or };

template <typename Color>
struct basic_pen {
  ::draw::pattern pattern = black;
  Color foreground;
  Color background;
};

template <typename Color>
class basic_bitmap;

template <typename Color, typename Pen>
void generic_frame_rect(basic_bitmap<Color>& bm, rect const& r, Pen const& pen) noexcept {
  if (r.bottom < r.top || r.right < r.left || r.bottom < 0 || r.right < 0) {
    return;
  }
  // The top and bottom lines
  bm.line({.x = r.left, .y = r.top}, {.x = r.right, .y = r.top}, pen);
  bm.line({.x = r.left, .y = r.bottom}, {.x = r.right, .y = r.bottom}, pen);
  // The left and right lines
  bm.line({.x = r.left, .y = r.top}, {.x = r.left, .y = r.bottom}, pen);
  bm.line({.x = r.right, .y = r.top}, {.x = r.right, .y = r.bottom}, pen);
}

template <typename Color, typename Pen>
void generic_paint_rect(basic_bitmap<Color>& bm, rect const& r, Pen const& pen) noexcept {
  if (r.bottom < r.top || r.right < r.left || r.bottom < 0 || r.right < 0) {
    return;
  }
  auto const h = bm.height();
  if (r.top >= 0 && static_cast<unsigned>(r.top) >= h) {
    return;
  }
  for (auto y = r.top; y <= r.bottom; ++y) {
    bm.line({.x = r.left, .y = y}, {.x = r.right, .y = y}, pen);
  }
}

template <typename Color>
class basic_bitmap {
  friend class glyph_cache;

public:
  [[nodiscard]] static constexpr std::uint16_t required_stride(std::uint16_t const width) noexcept;
  /// Returns the store size required for a bitmap with the supplied dimensions.
  /// \param width  The desired width of the bitmap in pixels.
  /// \param height  The desired height of the bitmap in pixels.
  /// \returns The size, in bytes, of the required frame store for a bitmap with the supplied dimensions.
  [[nodiscard]] static constexpr std::size_t required_store_size(std::uint16_t width, std::uint16_t height) noexcept;

  void copy(basic_bitmap const& source, point dest_pos, transfer_mode mode) noexcept;
  void clear();
  /// Sets or clears an individual pixel.
  /// \param p The pixel to be set
  /// \param new_state The desired state of the pixel
  constexpr void set(point p, bool new_state);
  /// Draws a straight line from p0 to p1.
  /// \param p0  Coordinate of one end of the line
  /// \param p1  Coordinate of the other end of the line
  /// \param pen  The pen with which the line is drawn
  template <typename Pen>
  void line(point p0, point p1, Pen const& pen);

  /// Draws a rectangle parallel to the bitmap with the specified bounds and pen
  /// \param r  The bounds of the rectangle drawn
  /// \param pen  The pen with which the rectangle is drawn
  template <typename Pen>
  void frame_rect(rect const& r, Pen const& pen);

  /// Fills a rectangle parallel to the bitmap with the specified bounds and pen
  /// \param r  The bounds of the rectangle drawn
  /// \param pen  The pen with which the rectangle is drawn
  template <typename Pen>
  void paint_rect(rect const& r, Pen const& pen);

  [[nodiscard]] constexpr std::uint16_t width() const noexcept;
  [[nodiscard]] constexpr std::uint16_t height() const noexcept;
  [[nodiscard]] constexpr std::uint16_t stride() const noexcept;
  [[nodiscard]] constexpr rect bounds() const noexcept;
  [[nodiscard]] constexpr std::optional<rect> const& dirty() const noexcept;
  constexpr void clean() noexcept;

#if defined(DRAW_HOSTED) && DRAW_HOSTED
  void dump(std::FILE* stream = stdout) const;
#endif
};

}  // end namespace draw

#endif  // BASIC_BITMAP_HPP
