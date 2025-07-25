#include "glyph_cache.hpp"

namespace draw {

bitmap glyph_cache::render(font const& f, std::vector<std::byte>* const bitmap_store, char32_t code_point) {
  auto height = f.height * 8U;

  auto pos = f.glyphs.find(static_cast<std::uint32_t>(code_point));
  if (pos == f.glyphs.end()) {
    pos = f.glyphs.find(white_square);
    if (pos == f.glyphs.end()) {
      // We've got no definition for the requested code point and no definition for U+25A1 (WHITE SQUARE). Last resort
      // is just some whitespace. TODO: how wide to make it? "widest" seems OTT.
      std::ranges::fill(*bitmap_store, std::byte{0});
      return {*bitmap_store, height, f.widest};
    }
  }

  auto width = static_cast<unsigned>(pos->second.size() / f.height);
  bitmap bm{*bitmap_store, width, height};
  for (auto y = std::size_t{0}; y < height; ++y) {
    if constexpr (trace_unpack) {
      std::print("|");
    }
    // TODO: write output one byte directly to the store at a time rather than one pixel via the API.
    for (auto x = 0U; x < width; ++x) {
      auto const src_index = (x * f.height) + (y / 8U);
      assert(src_index < pos->second.size() && "The source byte is not within the bitmap");
      auto const pixel = pos->second[src_index] & (std::byte{1} << (y % 8U));
      bm.set(point{.x = static_cast<ordinate>(x), .y = static_cast<ordinate>(y)}, pixel != std::byte{0});
      if constexpr (trace_unpack) {
        std::print("{}", pixel != std::byte{0} ? 'X' : ' ');
      }
    }
    if constexpr (trace_unpack) {
      std::println("|{0}", y == f.baseline ? "<-" : "");
    }
  }
  return bm;
}

}  // end namespace draw
