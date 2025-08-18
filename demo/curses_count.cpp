//===- demo/curses_count.cpp ----------------------------------------------===//
//*                                                        _    *
//*   ___ _   _ _ __ ___  ___  ___    ___ ___  _   _ _ __ | |_  *
//*  / __| | | | '__/ __|/ _ \/ __|  / __/ _ \| | | | '_ \| __| *
//* | (__| |_| | |  \__ \  __/\__ \ | (_| (_) | |_| | | | | |_  *
//*  \___|\__,_|_|  |___/\___||___/  \___\___/ \__,_|_| |_|\__| *
//*                                                             *
//===----------------------------------------------------------------------===//
// Copyright © 2025 Paul Bowen-Huggett
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// “Software”), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// SPDX-License-Identifier: MIT
//===----------------------------------------------------------------------===//
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
#include "draw/bitmap.hpp"
#include "draw/glyph_cache.hpp"
#include "draw/sans16.hpp"
#include "draw/sans32.hpp"
#include "draw/types.hpp"

using namespace std::string_view_literals;
using namespace draw::literals;
using draw::bitmap;
using draw::gray;
using draw::ordinate;
using draw::point;
using draw::rect;

namespace {

void themometer(bitmap& bmp, rect const& r, float pcnt) {
  assert(pcnt <= 1.0F);

  // Draw the thermometer border.
  bmp.frame_rect(r);

  // A gray fill for the body of the thermometer.
  auto fill = r.inset(1, 1);
  fill.right = r.left + static_cast<ordinate>(std::round(r.width() * pcnt));
  bmp.paint_rect(fill, draw::gray);
  // A solid line to denote the end of the filled region.
  bmp.line(point{.x = fill.right, .y = fill.top}, point{.x = fill.right, .y = fill.bottom});

  // Fill the remaining port with white.
  fill.left = fill.right + 1;
  fill.right = r.right - 1;
  bmp.paint_rect(fill, draw::white);
}

/// Draws the contents of a bitmap on a specific curses window.
/// \param win  The curses window on which the bitmap should be drawn
/// \param bmp  The bitmap to be drawn
void show(WINDOW* const win, bitmap const& bmp) {
  auto xb = 0U;  // The x ordinate (in bytes)
  auto yb = 0U;
  for (auto const d : bmp.store()) {
    for (auto bit = 0U; bit < 8U; ++bit) {
      mvwprintw(win, static_cast<int>(yb), static_cast<int>((xb * 8U) + bit), "%c",
                (d & (0x80_b >> bit)) != 0_b ? 'X' : ' ');
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

  constexpr auto frame_width = ordinate{128};
  constexpr auto frame_height = ordinate{32};
  std::array<std::byte, bitmap::required_store_size(frame_width, frame_height)> frame_store{};
  bitmap frame_buffer{frame_store, frame_width, frame_height};
  draw::glyph_cache gc;

  auto count = 0;
  auto swidth = 0;
  do {
    // Erase the previous glyphs.
    frame_buffer.paint_rect(rect{.top = 0,
                                 .left = static_cast<ordinate>(frame_width - swidth),
                                 .bottom = frame_height - 1U,
                                 .right = frame_width - 1U},
                            draw::white);

    themometer(frame_buffer,
               rect{.top = frame_height - 6, .left = 0, .bottom = frame_height - 1, .right = frame_width - 1},
               (count % 100) / 100.0F);
    std::array<char8_t, 32> str_buffer;
    auto const first = std::begin(str_buffer);
    auto const last = std::format_to_n(first, str_buffer.size(), "{}", count).out;
    std::u8string_view const str_view{first, last};
    swidth = std::min(draw::string_width(sans32, str_view), ordinate{frame_width});
    frame_buffer.draw_string(gc, sans32, str_view, point{static_cast<ordinate>(frame_width - swidth), -1});

    ++count;

    show(stdscr, frame_buffer);
    refresh();
  } while (getch() != 'q');
  endwin();
}
