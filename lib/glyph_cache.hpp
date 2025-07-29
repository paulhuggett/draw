#ifndef GLYPH_CACHE_HPP
#define GLYPH_CACHE_HPP

#include <cassert>
#include <optional>
#include <print>
#include <vector>

#include "bitmap.hpp"
#include "font.hpp"

namespace draw {

class glyph_cache {
  /// Enable to inspect the unpacking and rotation of the font data.
  static constexpr bool trace_unpack = false;

public:
  explicit constexpr glyph_cache(font const& f)
      : font_{&f}, bitmap_store_{std::size_t{this->stride(f) * this->pixel_height(f)}} {}

  [[nodiscard]] bitmap const& get(char32_t code_point) {
    if (code_point == code_point_) {
      return cache_bm_;
    }
    cache_bm_ = this->render(*font_, &bitmap_store_, code_point);
    code_point_ = code_point;
    return cache_bm_;
  }

  [[nodiscard]] constexpr auto spacing() const noexcept { return font_->spacing; }
  [[nodiscard]] font const* get_font() const noexcept { return font_; }

private:
  /// Renders an individual glyph into the supplied bitmap.
  static bitmap render(font const& f, std::vector<std::byte>* const bitmap_store, char32_t code_point);

  [[nodiscard]] static constexpr unsigned stride(font const& f) { return (f.widest + 7U) / 8U; }
  [[nodiscard]] static constexpr unsigned pixel_height(font const& f) { return f.height * 8U; }

  font const* font_;
  // just one entry ATM.

  std::optional<std::uint32_t> code_point_;
  std::vector<std::byte> bitmap_store_;
  /// A bitmap that is large enough to contain the largest glyph in the font.
  bitmap cache_bm_;
};

} // end namespace draw

#endif  // GLYPH_CACHE_HPP
