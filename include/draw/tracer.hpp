//===- include/draw/tracer.hpp ----------------------------*- mode: C++ -*-===//
//*  _                            *
//* | |_ _ __ __ _  ___ ___ _ __  *
//* | __| '__/ _` |/ __/ _ \ '__| *
//* | |_| | | (_| | (_|  __/ |    *
//*  \__|_|  \__,_|\___\___|_|    *
//*                               *
//===----------------------------------------------------------------------===//
// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
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
//
//===----------------------------------------------------------------------===//
#ifndef DRAW_TRACER_HPP
#define DRAW_TRACER_HPP

// Standard library
#include <cassert>
#include <cstddef>
#include <tuple>
#include <version>

// Define DRAW_PRINT as 1 if this is a hosted implementation with std::print() support.
#if defined(DRAW_HOSTED) && DRAW_HOSTED && defined(__cpp_lib_print) && __cpp_lib_print >= 202207L
#include <print>
#define DRAW_PRINT (1)
#else
#define DRAW_PRINT (0)
#endif  // DRAW_HOSTED && __cpp_lib_print

// Local includes
#include "draw/types.hpp"

namespace draw {

template <bool Enabled>
class tracer {
public:
#if DRAW_PRINT
  template <typename... Args>
  [[maybe_unused]] constexpr void operator()(std::format_string<Args...> const& /*format*/, Args&&... /*args*/) const {
    // do nothing
  }
#else
  template <typename... Args>
  [[maybe_unused]] constexpr void operator()(char const* /*format*/, Args&&... /*args*/) const {
    // do nothing
  }
#endif  // DRAW_PRINT
  /// \param src_x The first and last pixel position to be dumped.
  /// \param src_row A row of pixel data.
  [[maybe_unused]] constexpr void operator()(std::tuple<unsigned, unsigned> const& src_x,
                                             std::byte const* const src_row) const {
    (void)src_x;
    (void)src_row;
    // do nothing
  }
};
#if DRAW_PRINT
template <>
class tracer<true> {
public:
  template <typename... Args>
  [[maybe_unused]] constexpr void operator()(std::format_string<Args...> const& format, Args&&... args) const {
    std::print(format, std::forward<Args>(args)...);
  }
  /// \param src_x The first and last pixel position to be dumped.
  /// \param src_row A row of pixel data.
  [[maybe_unused]] constexpr void operator()(std::tuple<unsigned, unsigned> const& src_x,
                                             std::byte const* const src_row) const {
    if (src_row == nullptr) {
      return;
    }
    using namespace draw::literals;
    for (auto x = get<0>(src_x); x < get<1>(src_x); ++x) {
      if (x % 8U == 0U) {
        std::print("'");
      }
      std::print("{:c}", (src_row[x / 8U] & (0x80_b >> (x % 8U))) != std::byte{0U} ? '1' : '0');
    }
    std::print("    ");
  }
};
#endif  // DRAW_PRINT

}  // namespace draw

#endif  // DRAW_TRACER_HPP
