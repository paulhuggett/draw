#ifndef GLYPH_CACHE_HPP
#define GLYPH_CACHE_HPP

#include <cassert>
#include <print>
#include <vector>

#include "bitmap.hpp"
#include "font.hpp"

namespace draw {

class glyph_cache {
  /// Enable to inspect the unpacking and rotation of the font data.
  static constexpr bool trace_unpack = false;

public:
  explicit constexpr glyph_cache(font const& f) : font_{&f}, cache_bm_{0, this->pixel_height(), this->stride()} {}

  bitmap const& get(char32_t code_point) {
    if (code_point == code_point_) {
      return cache_bm_;
    }
    this->render(code_point, &cache_bm_);
    code_point_ = code_point;
    return cache_bm_;
  }

  constexpr unsigned spacing() const { return font_->spacing; }

private:
  /// Renders an individual glyph into the supplied bitmap.
  void render(char32_t code_point, bitmap* const bm);

  [[nodiscard]] constexpr unsigned stride() const { return (font_->widest + 7U) / 8U; }
  [[nodiscard]] constexpr unsigned pixel_height() const { return font_->height * 8U; }

  font const* font_;
  // just one entry ATM.

  std::uint32_t code_point_ = 0xFFFF'FFFF;
  /// A bitmap that is large enough to contain the largest glyph in the font.
  bitmap cache_bm_;
};

} // end namespace draw

#endif  // GLYPH_CACHE_HPP
