#ifndef FONT_HPP
#define FONT_HPP

#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>
#include <unordered_map>

constexpr auto white_square = std::uint32_t{0x25A1};
// constexpr auto replacement_char = std::uint32_t{0xFFFD};

struct kerning_pair {
  /// Code point of the preceeding glyph
  std::uint32_t preceeding : 21 = 0;
  std::uint32_t pad : 3 = 0;
  std::uint32_t distance : 8 = 0;
};

struct font {
  std::uint8_t baseline;
  std::uint8_t widest;
  std::uint8_t height;  // in bytes rather than pixels.
  std::uint8_t spacing;
  using glyph = std::tuple<std::span<kerning_pair const>, std::span<std::byte const>>;
  std::unordered_map<std::uint32_t, glyph> glyphs;
};

#endif  // FONT_HPP
