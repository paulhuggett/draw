#ifndef DRAW_GLYPH_CACHE_HPP
#define DRAW_GLYPH_CACHE_HPP

#include <cstddef>
#include <vector>

#include "bitmap.hpp"
#include "font.hpp"
#include "plru_cache.hpp"

namespace draw {

class glyph_cache {
  /// Enable to inspect the unpacking and rotation of the font data.
  static constexpr bool trace_unpack = false;

public:
  explicit constexpr glyph_cache(font const& f) noexcept : font_{&f} {}

  [[nodiscard]] bitmap const& get(char32_t code_point);

  [[nodiscard]] font::glyph const* find_glyph(char32_t code_point) const;
  [[nodiscard]] font const* get_font() const noexcept { return font_; }
  [[nodiscard]] constexpr std::uint8_t spacing() const noexcept { return font_->spacing; }

private:
  /// Renders an individual glyph into the supplied bitmap.
  [[nodiscard]] bitmap render(std::vector<std::byte>* const bitmap_store, char32_t code_point);

  [[nodiscard]] static constexpr unsigned stride(font const& f) { return (f.widest + 7U) / 8U; }
  [[nodiscard]] static constexpr unsigned pixel_height(font const& f) { return f.height * 8U; }

  font const* font_;
  /// A bitmap that is large enough to contain the largest glyph in the font.
  // TODO: don't allocate a vector every time. Separate the store.
  struct entry {
    std::vector<std::byte> store;
    bitmap bm;
  };
  plru_cache<std::uint32_t, entry, 8, 4> cache_;
};

} // end namespace draw

#endif  // DRAW_GLYPH_CACHE_HPP
