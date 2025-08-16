// Draw
#include "bitmap.hpp"
#include "glyph_cache.hpp"
#include "sans16.hpp"
#include "sans32.hpp"
#include "types.hpp"

using draw::bitmap;
using draw::font;
using draw::ordinate;
using draw::point;

namespace {

std::vector<char32_t> sorted_code_points(draw::glyph_cache const& gc) {
  font const* const f = gc.get_font();

  std::vector<char32_t> code_points;
  code_points.reserve(std::size(f->glyphs));
  for (auto const& kvp : f->glyphs) {
    if (kvp.first > ' ') {
      code_points.push_back(kvp.first);
    }
  }
  std::ranges::sort(code_points);
  return code_points;
}

}  // end anonymous namespace

int main() {
  constexpr auto frame_width = ordinate{128};
  constexpr auto frame_height = ordinate{32};
  std::array<std::byte, bitmap::required_store_size(frame_width, frame_height)> frame_store{};
  bitmap bm{frame_store, frame_width, frame_height};
  draw::glyph_cache gc{sans16};

  point pos;
  for (auto const key : sorted_code_points(gc)) {
    auto const width = bm.char_width(gc, key) + 1;
    if (pos.x + width > bm.width()) {
      pos.x = 0;
      pos.y += gc.get_font()->height * 8;
      if (pos.y >= bm.height()) {
        break;
      }
    }

    bm.draw_char(gc, key, pos);
    pos.x += width;
  }
  bm.dump();
}
