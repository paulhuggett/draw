#include <cassert>
#include <cmath>
#include <print>
#include <string_view>
#include <vector>

#include "bitmap.hpp"
#include "glyph_cache.hpp"
#include "sans.hpp"

using namespace std::string_view_literals;
using draw::grey;
using draw::point;
using draw::rect;

int main() {
  draw::bitmap frame_buffer{128U, 32U};
  draw::glyph_cache gc{sans};
  // draw_string(screen, gc, u8"Excrétionnaïtreçât!"sv, point{2, 0});
  draw_string(frame_buffer, gc, u8"Hello World!"sv, point{1, 0});
  // draw_line(frame_buffer, 0, 30, 100, 3);

  constexpr auto bar_rect = rect{.top = 18, .left = 4, .bottom = 26, .right = 123};
  auto fill = bar_rect.inset(1, 1);
  fill.right = fill.left + std::round(fill.width() * 0.25);

  frame_buffer.frame_rect(bar_rect);
  frame_buffer.paint_rect(fill, grey);
  frame_buffer.line(point{.x = fill.right, .y = fill.top}, point{.x = fill.right, .y = fill.bottom});
  // draw_string(screen, gc, u8"100%"sv, point{2, 32});

  frame_buffer.line(point{0, 31}, point{127, 0});
  frame_buffer.dump();
}
