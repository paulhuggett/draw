#ifndef FONT_HPP
#define FONT_HPP

#include <cstddef>
#include <cstdint>
#include <span>
#include <unordered_map>

constexpr auto white_square = std::uint32_t{0x25A1};
// constexpr auto replacement_char = std::uint32_t{0xFFFD};

struct font {
  std::uint8_t baseline;
  std::uint8_t widest;
  std::uint8_t height;  // in bytes rather than pixels.
  std::uint8_t spacing;
  std::unordered_map<std::uint32_t, std::span<std::byte const>> glyphs;
};

#endif  // FONT_HPP
