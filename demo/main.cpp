#include <bit>
#include <cassert>
#include <chrono>
#include <cmath>
#include <print>
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

using namespace std::string_view_literals;
using namespace draw::literals;
using draw::bitmap;
using draw::gray;
using draw::ordinate;
using draw::point;
using draw::rect;

namespace {

void themometer(bitmap& bmp, rect const& r, float pcnt) {
  auto fill = r.inset(1, 1);
  fill.right = r.left + static_cast<ordinate>(std::round(r.width() * pcnt));

  bmp.frame_rect(r);
  bmp.paint_rect(fill, draw::gray);
  bmp.line(point{.x = fill.right, .y = fill.top}, point{.x = fill.right, .y = fill.bottom});

  fill.left = fill.right + 1;
  fill.right = r.right - 1;
  bmp.paint_rect(fill, draw::white);
}

void show(bitmap const& bmp) {
  auto xb = 0U;  // The x ordinate (in bytes)
  auto yb = 0U;
  for (auto const d : bmp.store()) {
    for (auto bit = 0U; bit < 8U; ++bit) {
      mvprintw(static_cast<int>(yb), static_cast<int>((xb * 8U) + bit), "%c", (d & (0x80_b >> bit)) != 0_b ? 'X' : ' ');
    }
    ++xb;
    if (xb >= bmp.stride()) {
      // The end of a scan-line.
      xb = 0U;
      ++yb;
    }
  }
}

}  // end anonymous namespace

int main() {
  using namespace std::chrono_literals;

  initscr();  // Refreshes stdscr
  cbreak();
  noecho();
  halfdelay(5);

  std::array<std::byte, 128 / 8 * 32> frame_store{};
  bitmap frame_buffer{frame_store, 128U, 32U};
  draw::glyph_cache gc16{sans16};
  draw::glyph_cache gc32{sans32};

  auto count = 0;
  auto swidth = 0;
  do {
    frame_buffer.paint_rect(rect{.top = 0, .left = static_cast<ordinate>(128 - swidth), .right = 127, .bottom = 31},
                            draw::white);

    themometer(frame_buffer, rect{.top = 26, .left = 0, .bottom = 31, .right = 127}, (count % 100) / 100.0F);
    std::array<char8_t, 32> str_buffer;
    auto const first = std::begin(str_buffer);
    auto const last = std::format_to_n(first, str_buffer.size(), "{}", count).out;
    std::u8string_view const str_view{first, last};
    swidth = std::min(draw::string_width(gc32, str_view), ordinate{128});
    frame_buffer.draw_string(gc32, str_view, point{static_cast<ordinate>(128 - swidth), -1});

    ++count;

    show(frame_buffer);
    refresh();
  } while (getch() != 'q');
  endwin();
}
