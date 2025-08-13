#include <bit>
#include <cassert>
#include <chrono>
#include <cmath>
#include <format>
#include <locale>
#include <print>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

// Curses
#include <ncurses.h>
#include <panel.h>

// Draw
#include "bitmap.hpp"
#include "glyph_cache.hpp"
#include "sans16.hpp"
#include "sans32.hpp"
#include "types.hpp"

using namespace std::literals;
using namespace draw::literals;
using draw::bitmap;
using draw::gray;
using draw::ordinate;
using draw::point;
using draw::rect;

int main() {
  using namespace std::chrono_literals;

  constexpr auto frame_width = ordinate{128};
  constexpr auto frame_height = ordinate{32};
  std::array<std::byte, bitmap::required_store_size(frame_width, frame_height)> frame_store{};
  bitmap frame_buffer{frame_store, frame_width, frame_height};
  draw::glyph_cache gc{sans32};

  std::array<char8_t, 32> time_str;
  auto const first = time_str.begin();
  auto const last = std::format_to_n(first, time_str.size(), "{:%H:%M:%S}", std::chrono::round<std::chrono::seconds>(std::chrono::system_clock::now())).out;
  frame_buffer.draw_string(gc, std::u8string_view{first, last}, point{.x = 0, .y = 0});
  frame_buffer.dump();
}
