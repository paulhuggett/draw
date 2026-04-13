//===- demo/curses_rect.cpp -----------------------------------------------===//
//*                                                _    *
//*   ___ _   _ _ __ ___  ___  ___   _ __ ___  ___| |_  *
//*  / __| | | | '__/ __|/ _ \/ __| | '__/ _ \/ __| __| *
//* | (__| |_| | |  \__ \  __/\__ \ | | |  __/ (__| |_  *
//*  \___|\__,_|_|  |___/\___||___/ |_|  \___|\___|\__| *
//*                                                     *
//===----------------------------------------------------------------------===//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
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
//===----------------------------------------------------------------------===//

// Standard library
#include <bit>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string_view>
#include <thread>
#include <vector>

// POSIX
#include <unistd.h>

// Curses
#include <ncurses.h>
#include <panel.h>

// Draw
#include "draw/bitmap.hpp"

using namespace std::string_view_literals;
using namespace std::chrono_literals;
using namespace draw::literals;
using draw::bitmap;
using draw::coordinate;
using draw::gray;
using draw::point;
using draw::rect;

namespace {

class bitmap_output {
public:
  constexpr bitmap_output() = default;
  constexpr bitmap_output(bitmap_output const&) = default;
  constexpr bitmap_output(bitmap_output&&) noexcept = default;
  constexpr virtual ~bitmap_output() noexcept = default;

  bitmap_output & operator=(bitmap_output const & ) = default;
  bitmap_output & operator=(bitmap_output && ) noexcept = default;

  void show_frame(bitmap const& bmp) {
    this->start_frame();
    auto xb = 0U;  // The x ordinate (in bytes)
    auto yb = 0U;
    for (auto const d : bmp.store()) {
      if (xb == 0U) {
        this->start_row();
      }
      for (auto bit = 0U; bit < 8U; ++bit) {
        auto const c = ((d & (0x80_b >> bit)) != 0_b ? 'X' : '.');
        this->print(c, yb, (xb << 3) | bit);
      }
      ++xb;
      if (xb >= bmp.stride()) {
        // The end of a scan-line.
        xb = 0U;
        ++yb;
        this->end_row();
      }
    }
    this->end_frame();
  }

protected:
  virtual void start_frame() {}
  virtual void start_row() {}
  virtual void print(char c, unsigned yb, unsigned xb) = 0;
  virtual void end_row() {}
  virtual void end_frame() {}
};

class curses_bitmap_output : public bitmap_output {
public:
  curses_bitmap_output()  {
    initscr();  // Refreshes stdscr
    cbreak();
    noecho();
    halfdelay(1);
  }
  ~curses_bitmap_output() noexcept override { endwin(); }

private:
  void print(char c, unsigned yb, unsigned xb) override {
    mvwprintw(stdscr, static_cast<int>(yb), static_cast<int>(xb), "%c", c);
  }
  void end_frame() override { refresh(); }
};

class json_bitmap_output : public bitmap_output {
public:
  json_bitmap_output() { std::puts("["); }
  ~json_bitmap_output() noexcept override { std::puts("]"); }

private:
  void start_frame() override {
    std::printf("%s[\n", frame_separator_);
    row_separator_ = "\"";
    frame_separator_ = ", ";
  }
  void start_row() override {
    std::printf("%s", row_separator_);
    row_separator_ = ",\n\"";
  }
  void print(char c, unsigned /*yb*/, unsigned /*xb*/) override { std::putchar(c); }
  void end_row() override { std::putchar('"'); }
  void end_frame() override { std::printf("\n]"); }

  char const* frame_separator_ = "";
  char const* row_separator_ = nullptr;
};

std::unique_ptr<bitmap_output> make_output(bool const use_json) {
  if (use_json) {
    return std::make_unique<json_bitmap_output>();
  }
  return std::make_unique<curses_bitmap_output>();
}

template <coordinate FrameWidth, coordinate FrameHeight>
std::tuple<draw::rect, draw::point> nudge(draw::rect const& r, draw::point vector) {
  if (r.left < std::abs(vector.x) || r.right >= FrameWidth - 1) {
    vector.x = static_cast<coordinate>(-vector.x);
  }
  if (r.top < std::abs(vector.y) || r.bottom >= FrameHeight - 1) {
    vector.y = static_cast<coordinate>(-vector.y);
  }
  return {r.offset(vector), vector};
}

struct options {
  int frames = 0;
  bool json = false;
  std::chrono::milliseconds delay = 50ms;
  point box = {.x = 13, .y = 5};
  point vector = {.x = 3, .y = 1};

  options(int argc, char* argv[]) {
    int ch = 0;
    while ((ch = getopt(argc, argv, "d:jw:h:f:x:y:")) != -1) {
      switch (ch) {
      case 'd': delay = std::chrono::milliseconds{intarg(optarg)}; break;
      case 'f': frames = intarg(optarg); break;
      case 'j': json = true; break;
      case 'w': box.x = static_cast<coordinate>(intarg(optarg)); break;
      case 'h': box.y = static_cast<coordinate>(intarg(optarg)); break;
      case 'x': vector.x = static_cast<coordinate>(intarg(optarg)); break;
      case 'y': vector.y = static_cast<coordinate>(intarg(optarg)); break;
      default: usage(argv[0]); std::exit(EXIT_FAILURE);
      }
    }
    argc -= optind;
    argv += optind;
  }

  void usage(char const* proc) {
    std::printf("Usage: %s [options]\n", proc);
    std::printf(R"(
  -d<n>  Inter-frame delay in milliseconds (default=50ms)
  -f<n>  Limit number of frames to <n>
  -j     Produce JSON (rather than using curses)
  -w<n>  Box width n pixels  (default=13)
  -h<n>  Box height n pixels (default=5)
  -x<n>  X distance moved per frame (default=3)
  -y<n>  Y distance moved per frame (default=1)
  )");
  }

  int intarg(char const* arg) { return static_cast<int>(std::strtol(arg, nullptr, 10)); }
};

}  // end anonymous namespace

int main(int argc, char* argv[]) {
  constexpr auto frame = point{.x = 128, .y = 32};
  static constexpr auto& background = draw::white;
  static std::array<std::byte, bitmap::required_store_size(frame.x, frame.y)> frame_store{};

  options opts{argc, argv};

  auto output = make_output(opts.json);

  auto frame_buffer = bitmap{frame_store, frame.x, frame.y};
  frame_buffer.paint_rect({.top = 0, .left = 0, .bottom = frame.y - 1, .right = frame.x - 1}, background);
  draw::rect r{.top = 0, .left = 0, .bottom = opts.box.y, .right = opts.box.x};
  draw::point vector{.x = opts.vector.x, .y = opts.vector.y};
  r = r.offset(vector);
  auto until = std::chrono::system_clock::now() + opts.delay;
  for (;;) {
    std::tie(r, vector) = nudge<frame.x, frame.y>(r, vector);
    if (auto const& d = frame_buffer.dirty()) {
      frame_buffer.paint_rect(*d, background);
      frame_buffer.clean();
    }
    frame_buffer.paint_rect(r, draw::black);

    output->show_frame(frame_buffer);
    if (opts.frames > 0) {
      --(opts.frames);
      if (opts.frames == 0) {
        break;
      }
    }

    if (!opts.json) {
      std::this_thread::sleep_until(until);
      until += opts.delay;
    }
  }
}
