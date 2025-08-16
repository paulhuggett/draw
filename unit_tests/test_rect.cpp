#include <gtest/gtest.h>

#include "draw/bitmap.hpp"
#include "draw/types.hpp"

namespace draw {

void PrintTo(rect const& r, std::ostream* os);
void PrintTo(rect const& r, std::ostream* os) {
  *os << "{.top=" << r.top << ",.left=" << r.left << ",.bottom=" << r.bottom << ",.right=" << r.right << '}';
}

}  // namespace draw
namespace {

TEST(Rect, Null) {
  draw::rect r;
  EXPECT_EQ(r.width(), 0);
  EXPECT_EQ(r.height(), 0);
  EXPECT_TRUE(r.empty());
}

TEST(Rect, InsetEmpty) {
  draw::rect const r = draw::rect{}.inset(1, 1);
  EXPECT_EQ(r.width(), 0);
  EXPECT_EQ(r.height(), 0);
  EXPECT_TRUE(r.empty());
}

TEST(Rect, InsetSmaller) {
  draw::rect const r = draw::rect{.top = 10, .left = 10, .bottom = 20, .right = 20}.inset(1, 1);
  EXPECT_EQ(r, (draw::rect{.top = 11, .left = 11, .bottom = 19, .right = 19}));
  EXPECT_EQ(r.width(), 8);
  EXPECT_EQ(r.height(), 8);
  EXPECT_FALSE(r.empty());
}

TEST(Rect, InsetLarger1) {
  draw::rect const r = draw::rect{.top = 10, .left = 10, .bottom = 20, .right = 20}.inset(-1, -1);
  EXPECT_EQ(r, (draw::rect{.top = 9, .left = 9, .bottom = 21, .right = 21}));
  EXPECT_EQ(r.width(), 12);
  EXPECT_EQ(r.height(), 12);
  EXPECT_FALSE(r.empty());
}

TEST(Rect, InsetLarger2) {
  draw::rect const r = draw::rect{.top = 10, .left = 10, .bottom = 20, .right = 20}.inset(-5, -5);
  EXPECT_EQ(r, (draw::rect{.top = 5, .left = 5, .bottom = 25, .right = 25}));
  EXPECT_EQ(r.width(), 20);
  EXPECT_EQ(r.height(), 20);
  EXPECT_FALSE(r.empty());
}

TEST(Rect, InsetToEmpty) {
  draw::rect const r = draw::rect{.top = 10, .left = 10, .bottom = 20, .right = 20}.inset(1, 1);
  EXPECT_EQ(r, (draw::rect{.top = 11, .left = 11, .bottom = 19, .right = 19}));
  EXPECT_EQ(r.width(), 8);
  EXPECT_EQ(r.height(), 8);
  EXPECT_FALSE(r.empty());
}

}  // end anonymous namespace
