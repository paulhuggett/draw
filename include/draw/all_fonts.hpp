//
// Created by Paul Bowen-Huggett on 24/04/2026.
//

#ifndef DRAW_ALL_FONTS_HPP
#define DRAW_ALL_FONTS_HPP

#include <array>
#include <functional>

#include "draw/sans16.hpp"
#include "draw/sans32.hpp"

namespace draw {

constexpr std::array all_fonts{std::cref(sans16), std::cref(sans32)};

}  // end namespace draw

#endif  // DRAW_ALL_FONTS_HPP
