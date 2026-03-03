//===- unit_tests/create_bitmap.cpp -----------------------*- mode: C++ -*-===//
//*                      _         _     _ _                          *
//*   ___ _ __ ___  __ _| |_ ___  | |__ (_) |_ _ __ ___   __ _ _ __   *
//*  / __| '__/ _ \/ _` | __/ _ \ | '_ \| | __| '_ ` _ \ / _` | '_ \  *
//* | (__| | |  __/ (_| | ||  __/ | |_) | | |_| | | | | | (_| | |_) | *
//*  \___|_|  \___|\__,_|\__\___| |_.__/|_|\__|_| |_| |_|\__,_| .__/  *
//*                                                           |_|     *
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

#include "create_bitmap.hpp"

std::tuple<std::vector<std::byte>, draw::bitmap> create_bitmap_and_store(std::uint16_t const width,
                                                                         std::uint16_t const height) {
  std::vector store{draw::bitmap::required_store_size(width, height), std::byte{0U}};
  return std::tuple(std::move(store), draw::bitmap{std::span{store}, width, height});
}
