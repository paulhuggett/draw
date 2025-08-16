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
  explicit constexpr glyph_cache(font const& f) noexcept : font_{&f} {
    store_.resize(cache_.max_size() * glyph_cache::store_size(f));
  }

  [[nodiscard]] bitmap const& get(char32_t code_point);

  [[nodiscard]] font::glyph const* find_glyph(char32_t code_point) const;
  [[nodiscard]] constexpr font const* get_font() const noexcept { return font_; }
  [[nodiscard]] constexpr std::uint8_t spacing() const noexcept { return font_->spacing; }

private:
  /// Renders an individual glyph into the supplied bitmap.
  [[nodiscard]] bitmap render(std::span<std::byte> bitmap_store, char32_t code_point);

  [[nodiscard]] static constexpr std::size_t store_size(font const& f) noexcept {
    std::size_t const stride = (f.widest + 7U) / 8U;
    std::size_t const pixel_height = f.height * 8U;
    return stride * pixel_height;
  }
  font const* font_;
  /// A bitmap that is large enough to contain the largest glyph in the font.
#if 0
  // TODO: don't allocate a vector every time. Separate the store.
  struct entry {
    std::vector<std::byte> store;
    bitmap bm;
  };
#endif
  std::vector<std::byte> store_;
  plru_cache<std::uint32_t, bitmap, 8, 2> cache_;
};

}  // end namespace draw

#endif  // DRAW_GLYPH_CACHE_HPP
