//===- unit_tests/rect.hpp --------------------------------*- mode: C++ -*-===//
//*                _    *
//*  _ __ ___  ___| |_  *
//* | '__/ _ \/ __| __| *
//* | | |  __/ (__| |_  *
//* |_|  \___|\___|\__| *
//*                     *
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
#ifndef RECT_HPP
#define RECT_HPP

// Standard library
#include <ostream>

// Google Test
#include <gtest/gtest.h>

// Local
#include "draw/types.hpp"

namespace draw {

inline void PrintTo(rect const& r, std::ostream* os) {
  *os << "{.top=" << r.top << ",.left=" << r.left << ",.bottom=" << r.bottom << ",.right=" << r.right << '}';
}

inline std::ostream& operator<<(std::ostream& os, rect const& r) {
  PrintTo(r, &os);
  return os;
}

}  // namespace draw

#endif  // RECT_HPP
