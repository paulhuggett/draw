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
#include "draw/all_fonts.hpp"
#include "draw/bitmap.hpp"
#include "draw/glyph_cache.hpp"
#include "draw/text.hpp"

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
  virtual ~bitmap_output() noexcept = default;

  void show_frame(bitmap const& bmp, rect update) {
    // Clamp the update rect to the bounds of the bitmap.
    update = {
        .top = std::max(update.top, coordinate{0}),
        .left = std::max(update.left, coordinate{0}),
        .bottom = std::clamp(update.bottom, coordinate{0}, bmp.bounds().bottom),
        .right = std::clamp(update.right, coordinate{0}, bmp.bounds().right),
    };

    this->start_frame();
    auto const store_it = std::begin(bmp.store());
    for (auto yb = static_cast<unsigned>(update.top); yb <= static_cast<unsigned>(update.bottom); ++yb) {
      auto const yoffset = yb * bmp.stride();
      for (auto xb = static_cast<unsigned>(update.left); xb <= static_cast<unsigned>(update.right); ++xb) {
        auto const offset = yoffset + xb / 8U;
        assert(offset < bmp.store().size());
        auto const d = *(store_it + offset);
        auto const bit = xb % 8U;
        auto const c = ((d & (0x80_b >> bit)) != 0_b ? 'X' : '.');
        this->print({.x = static_cast<coordinate>(xb), .y = static_cast<coordinate>(yb)}, c);
      }
    }
    this->end_frame();
  }

protected:
  virtual void start_frame() = 0;
  virtual void print(point pos, char c) = 0;
  virtual void end_frame() = 0;
};

class curses_bitmap_output final : public bitmap_output {
public:
  curses_bitmap_output()  {
    initscr();  // Refreshes stdscr
    cbreak();
    noecho();
    halfdelay(1);
  }
  curses_bitmap_output(curses_bitmap_output const&) = delete;
  curses_bitmap_output(curses_bitmap_output&&) noexcept = delete;
  ~curses_bitmap_output() noexcept override { endwin(); }

  curses_bitmap_output & operator=(curses_bitmap_output const & ) = delete;
  curses_bitmap_output & operator=(curses_bitmap_output && ) noexcept = delete;

private:
  void start_frame() override { /* do nothing */ }
  void print(point pos, char c) override { mvwprintw(stdscr, pos.y, pos.x, "%c", c); }
  void end_frame() override { refresh(); }
};

class json_bitmap_output final : public bitmap_output {
public:
  json_bitmap_output() { std::puts("["); }
  json_bitmap_output(json_bitmap_output const&) = delete;
  json_bitmap_output(json_bitmap_output&&) noexcept = delete;
  ~json_bitmap_output() noexcept override { std::puts("\n]"); }

  json_bitmap_output& operator=(json_bitmap_output const&) = delete;
  json_bitmap_output& operator=(json_bitmap_output&&) noexcept = delete;

private:
  void start_frame() override {
    std::printf("%s[\n", frame_separator_);
    row_separator_ = "";
    frame_separator_ = ", ";
  }
  void print(point pos, char c) override {
    if (pos.x != prev_.x + 1 || pos.y != prev_.y) {
      this->flush();
      start_ = pos;
    }
    row_ += c;
    prev_ = pos;
  }
  void end_frame() override {
    this->flush();
    std::printf("\n  ]");
  }

  void flush() {
    if (row_.length() > 0) {
      std::printf(R"(%s  { "location": [%d, %d], "pixels": "%s" })", row_separator_, start_.x, start_.y, row_.c_str());
      row_separator_ = ",\n";
      row_.clear();
    }
  }
  char const* frame_separator_ = "";
  char const* row_separator_ = "";
  std::string row_;
  point prev_{.x = -1, .y = -1};
  point start_{.x = -1, .y = -1};
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
      case 'w': box.x = std::max(static_cast<coordinate>(intarg(optarg)), coordinate{1}); break;
      case 'h': box.y = std::max(static_cast<coordinate>(intarg(optarg)), coordinate{1}); break;
      case 'x': vector.x = std::max(static_cast<coordinate>(intarg(optarg)), coordinate{1}); break;
      case 'y': vector.y = std::max(static_cast<coordinate>(intarg(optarg)), coordinate{1}); break;
      default: usage(argv[0]); std::exit(EXIT_FAILURE);
      }
    }
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

class drawable {
public:
  virtual ~drawable() noexcept = default;
  virtual void draw(bitmap& dest, point origin) = 0;
  [[nodiscard]] virtual point size() const = 0;
};

class text_drawable final : public drawable {
public:
  void draw(bitmap& dest, point origin) override { dest.draw_string(gc_, font, str_, origin); }
  [[nodiscard]] point size() const override {
    return {.x = draw::string_width(font, str_), .y = static_cast<coordinate>(font.height * 8U)};
  }

private:
  draw::font const& font = draw::sans16;
  std::u8string_view str_ = u8"Hello"sv;
  std::vector<std::byte> glyph_cache_store_{draw::glyph_cache::get_size(draw::all_fonts)};
  draw::glyph_cache gc_{draw::all_fonts, glyph_cache_store_};
};

class rect_drawable final : public drawable {
public:
  constexpr explicit rect_drawable(point size) : size_{size} {}
  void draw(bitmap& dest, point origin) override {
    rect const r{.top = 0, .left = 0, .bottom = size_.y, .right = size_.x};
    dest.paint_rect(r.offset(origin), draw::black);
  }
  [[nodiscard]] point size() const override { return size_; }

private:
  point size_;
};

int main(int argc, char* argv[]) {
  constexpr auto frame = point{.x = 128, .y = 32};
  static constexpr auto& background = draw::white;
  static std::array<std::byte, bitmap::required_store_size(frame.x, frame.y)> frame_store{};

  options opts{argc, argv};

  auto output = make_output(opts.json);

  auto frame_buffer = bitmap{frame_store, frame.x, frame.y};
  frame_buffer.paint_rect({.top = 0, .left = 0, .bottom = frame.y - 1, .right = frame.x - 1}, background);

  text_drawable draw;
  draw::point const size = draw.size();
  rect r{.top = 0, .left = 0, .bottom = size.y, .right = size.x};
  draw.draw(frame_buffer, r.top_left());
  r = r.offset(opts.vector);
  auto until = std::chrono::system_clock::now() + opts.delay;
  for (;;) {
    std::tie(r, opts.vector) = nudge<frame.x, frame.y>(r, opts.vector);
    std::optional<rect> update;
    if (auto const& d = frame_buffer.dirty()) {
      assert(d->right <= frame.x);
      assert(d->bottom <= frame.y);
      update = *d;
      frame_buffer.paint_rect(*d, background);
      frame_buffer.clean();
    }

    draw.draw(frame_buffer, r.top_left());

    output->show_frame(frame_buffer, update ? update->union_rect(r) : r);
    if (opts.frames > 0) {
      --opts.frames;
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
