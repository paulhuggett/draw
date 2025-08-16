#ifndef DRAW_FONT_HPP
#define DRAW_FONT_HPP

#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>
#include <unordered_map>

namespace draw {

constexpr auto white_square = std::uint32_t{0x25A1};

struct kerning_pair {
  /// Code point of the preceeding glyph
  std::uint32_t preceeding : 21 = 0;
  std::uint32_t pad : 3 = 0;
  std::uint32_t distance : 8 = 0;
};

// The glyphs in a font are always a multiple of 8 pixels tall as given by the font's 'height'
// member. The glyph data consists of an array of bytes which are grouped according to the font height: a
// height of 2 implies groups of 2 bytes each representing 8 pixels; a height of 4 indicates
// groups of 4 bytes give a total of 32 pixels per group. Each group of bytes represents a single
// column of pixels in the font. The least significant bit in each byte holds the pixel value for
// the smallest y position.
struct font {
  std::uint8_t baseline;
  std::uint8_t widest;
  std::uint8_t height;  // in bytes rather than pixels.
  std::uint8_t spacing;

  using glyph = std::tuple<std::span<kerning_pair const>, std::span<std::byte const>>;

  constexpr std::uint16_t width(glyph const& g) const noexcept {
    auto const& bitmap = std::get<std::span<std::byte const>>(g);
    return static_cast<std::uint16_t>(bitmap.size() / this->height);
  }

  // TODO: replace this with iumap<>
  // [https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/include/midi2/adt/iumap.hpp]
  // (or something like it).
  std::unordered_map<std::uint32_t, glyph> glyphs;
};

}  // end namespace draw

#endif  // DRAW_FONT_HPP
