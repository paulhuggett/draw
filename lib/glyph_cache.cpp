#include "glyph_cache.hpp"

namespace draw {

void glyph_cache::render(char32_t code_point, bitmap* const bm) {
  assert(bm->height() == this->pixel_height());
  assert(bm->stride() == this->stride());
  assert(bm->data_.size() == this->pixel_height() * this->stride());

  auto pos = font_->glyphs.find(static_cast<std::uint32_t>(code_point));
  if (pos == font_->glyphs.end()) {
    pos = font_->glyphs.find(white_square);
    if (pos == font_->glyphs.end()) {
      // We've got no definition for the requested code point and no definition for U+25A1 (WHITE SQUARE). Last resort
      // is just some whitespace.
      std::ranges::fill(bm->data_, std::byte{0});
      bm->width_ = font_->widest;
      return;
    }
  }

  bm->width_ = static_cast<unsigned>(pos->second.size() / font_->height);
  for (auto y = std::size_t{0}; y < this->pixel_height(); ++y) {
    if constexpr (trace_unpack) {
      std::print("|");
    }
    for (auto x = 0U; x < bm->width_; ++x) {
      auto const src_index = (x * font_->height) + (y / 8U);
      assert(src_index < pos->second.size() && "The source byte is not within the bitmap");
      auto const pixel = pos->second[src_index] & (std::byte{1} << (y % 8U));
      if constexpr (trace_unpack) {
        std::print("{}", pixel != std::byte{0} ? 'X' : ' ');
      }
      auto const dest_index = (y * this->stride()) + (x / 8U);
      assert(dest_index < bm->data_.size() && "The destination byte is not within the bitmap");
      auto& dest_byte = bm->data_[dest_index];
      auto dest_value = std::byte{0x80} >> (x % 8U);
      if (pixel != std::byte{0}) {
        dest_byte |= dest_value;
      } else {
        dest_byte &= ~dest_value;
      }
    }
    if constexpr (trace_unpack) {
      std::println("|{0}", y == font_->baseline ? "<-" : "");
    }
  }
}

}  // end namespace draw
