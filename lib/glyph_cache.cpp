#include "glyph_cache.hpp"

#include <cassert>
#include <print>

namespace draw {

font::glyph const* glyph_cache::find_glyph(char32_t code_point) const {
  auto pos = font_->glyphs.find(static_cast<std::uint32_t>(code_point));
  auto end = font_->glyphs.end();
  if (pos == end) {
    pos = font_->glyphs.find(white_square);
    if (pos == end) {
      // We've got no definition for the requested code point and no definition for U+25A1 (WHITE SQUARE). Last resort
      // is just some whitespace.
      pos = font_->glyphs.begin();
    }
  }
  return &pos->second;
}

bitmap glyph_cache::render(std::vector<std::byte>* const bitmap_store, char32_t code_point) {
  auto height = static_cast<std::uint16_t>(font_->height * 8);

  font::glyph const* glyph = this->find_glyph(code_point);

  auto const& bitmaps = std::get<std::span<std::byte const>>(*glyph);
  // auto const width = static_cast<std::uint16_t>(bitmaps.size() / f.height);
  auto const width = font_->width(*glyph);
  bitmap bm{*bitmap_store, width, height};
  for (auto y = std::size_t{0}; y < height; ++y) {
    if constexpr (trace_unpack) {
      std::print("|");
    }

    auto x = 0U;

    if (width >= 8) {
      // Assign whole bytes.
      for (; x < (width & ~0b111U); x += 8) {
        auto pixels = std::byte{0};
        for (auto bit = 0U; bit < 8U; ++bit) {
          auto const src_index = ((x + bit) * font_->height) + (y / 8U);
          assert(src_index < bitmaps.size() && "The source byte is not within the bitmap");
          if ((bitmaps[src_index] & (std::byte{1} << (y % 8U))) != std::byte{0}) {
            pixels |= std::byte{0x80} >> bit;
          }
        }

        auto const dest_index = y * bm.stride() + (x / 8U);
        assert(dest_index < bitmap_store->size() && "The destination byte is not within the bitmap store");
        (*bitmap_store)[dest_index] = pixels;
        if constexpr (trace_unpack) {
          auto const get_pixel = [pixels](unsigned bit) {
            return (pixels & (std::byte{1} << bit)) != std::byte{0} ? 'X' : ' ';
          };
          for (auto ctr = 0U; ctr < 8U; ++ctr) {
            std::print("{}", get_pixel(7 - ctr));
          }
        }
      }
    }

    for (; x < width; ++x) {
      auto const src_index = (x * font_->height) + (y / 8U);
      assert(src_index < bitmaps.size() && "The source byte is not within the bitmap");
      auto const pixel = bitmaps[src_index] & (std::byte{1} << (y % 8U));
      bm.set(point{.x = static_cast<ordinate>(x), .y = static_cast<ordinate>(y)}, pixel != std::byte{0});
      if constexpr (trace_unpack) {
        std::print("{}", pixel != std::byte{0} ? 'X' : ' ');
      }
    }
    if constexpr (trace_unpack) {
      std::println("|{0}", y == font_->baseline ? "<-" : "");
    }
  }
  return bm;
}

}  // end namespace draw
