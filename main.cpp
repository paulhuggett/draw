#include <cassert>
#include <cmath>
#include <print>
#include <string_view>
#include <vector>

#include "bitmap.hpp"
#include "glyph_cache.hpp"
#include "sans16.hpp"
#include "sans32.hpp"

using namespace std::string_view_literals;
using draw::bitmap;
using draw::gray;
using draw::point;
using draw::rect;

namespace {

void themometer(bitmap& bmp, rect const& r, float pcnt) {
  auto fill = r.inset(1, 1);
  fill.right = r.left + static_cast<draw::ordinate>(std::round(r.width() * pcnt));

  bmp.frame_rect(r);
  bmp.paint_rect(fill, draw::light_gray);
  bmp.line(point{.x = fill.right, .y = fill.top}, point{.x = fill.right, .y = fill.bottom});
}

}  // end anonymous namespace

int main() {
  std::array<std::byte, 128 / 8 * 32> frame_store{};
  bitmap frame_buffer{frame_store, 128U, 32U};
  draw::glyph_cache gc16{sans16};
  draw::glyph_cache gc32{sans32};

  for (auto ctr = 0; ctr < 1; ++ctr) {
    // draw_string(frame_buffer, gc16, u8"Hello World!"sv, point{2, 0});
    // themometer(frame_buffer, rect{.top = 18, .left = 4, .bottom = 26, .right = 123}, 0.25);
    auto right = draw_string(frame_buffer, gc32,
                             u8"attempt to disvalue, muzzle the afghan czech czar and exninja, bob bixby dvorak"sv,
                             point{-000 /*sans32.spacing*/, 0});
    // frame_buffer.frame_rect(draw::rect{.left=0, .top=0, .right=right.x, .bottom=31});
  }
  frame_buffer.dump();
}
