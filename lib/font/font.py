#!/usr/bin/env python3
# ===- lib/font/font.py ---------------------------------------------------===//
# *   __             _    *
# *  / _| ___  _ __ | |_  *
# * | |_ / _ \| '_ \| __| *
# * |  _| (_) | | | | |_  *
# * |_|  \___/|_| |_|\__| *
# *                       *
# ===----------------------------------------------------------------------===//
#  Copyright © 2025 Paul Bowen-Huggett
#
#  Permission is hereby granted, free of charge, to any person obtaining
#  a copy of this software and associated documentation files (the
#  “Software”), to deal in the Software without restriction, including
#  without limitation the rights to use, copy, modify, merge, publish,
#  distribute, sublicense, and/or sell copies of the Software, and to
#  permit persons to whom the Software is furnished to do so, subject to
#  the following conditions:
#
#  The above copyright notice and this permission notice shall be
#  included in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
#  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
#  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
#  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
#  SPDX-License-Identifier: MIT
# ===----------------------------------------------------------------------===//
from typing import Any, Dict, List, Tuple, Union
import argparse
import json
import os
import pathlib
import sys
import typing
import unicodedata
# 3rd party
import png

REPLACEMENT_CHAR = 0xFFFD

FontDict = Dict[int, Union[Tuple[int, ...], int]]
SplitList = List[Tuple[int, int]]

def is_splitable(no_split:SplitList, x:int) -> Tuple[SplitList, bool]:
    """
    Check if the column at x is splitable based on the no_split list.
    The no_split list contains pairs of (start, end) indices that should not be split.
    """
    splitable = True
    no_split_point = no_split[0]
    if x >= no_split_point[0] and x <= no_split_point[1]:
        splitable = False
        if x == no_split_point[1]:
            no_split = no_split[1:]  # Remove this region from the no-split list
    return (no_split, splitable)

Input = Dict[str, Any]
InputList = List[Input]
PixelData = List[List[int]]

def read_png_file(parent:pathlib.Path, f:Input) -> Tuple[int, int, PixelData]:
    file = parent / pathlib.Path(f['file'])
    if not file.exists():
        raise RuntimeError(f'File {file} does not exist!')
    reader = png.Reader(filename=file)
    width, height, pixels, metadata = reader.asRGBA()
    pixels = list(pixels)
    if metadata['bitdepth'] != 8:
        raise RuntimeError("File {file} does not have a bitdepth of 8")
    if metadata['planes'] != 4:
        raise RuntimeError("File {file} does not have 4 planes: expected RGBA format")
    if height % 8 != 0:
        raise RuntimeError('Height must be a multiple of 8!')
    return width, height, pixels


class BuildFontState:
    def __init__(self)  -> None:
        self.__all_code_points:FontDict = {}
        self.__all_patterns:Dict[int, int] = {} # Map a hash of the bitmap to the corresponding code point.
    def append_columns(self, code_point:int, columns:List[int]) -> None:
        all_patterns_key = hash(tuple(columns))
        prev_code_point = self.__all_patterns.get(all_patterns_key)
        if prev_code_point is None:
            self.__all_code_points[code_point] = tuple(columns)
            self.__all_patterns[all_patterns_key] = code_point
        else:
            self.__all_code_points[code_point] = prev_code_point
    def all_code_points(self) -> FontDict:
        return self.__all_code_points


def scan_y(x:int, height:int, pixels:PixelData) -> List[int]:
    column = [0] * (height // 8)
    x_offset = x * 4
    for y in range(0, height):
        rgba = pixels[y][x_offset : x_offset + 4]
        # Is this a black pixel?
        if rgba == bytearray.fromhex('00 00 00 FF'):
            column[y // 8] |= (1 << (y % 8))
    return column


def build_font(inputs:InputList, parent:pathlib.Path) -> Tuple[FontDict, int] | None:
    if len(inputs) == 0:
        raise RuntimeError('No inputs specified!')

    code_point = ord('!')
    state = BuildFontState()
    height = None

    for f in inputs:
        width, height2, pixels = read_png_file(parent, f)
        if height is None:
            height = height2
        elif height2 != height:
            raise RuntimeError(f'Height of {parent / pathlib.Path(f['file'])} does not match previous height of {height}!')

        # 'starts' is a list of two-tuples (x, code_point) where x is the x-coordinate of the
        # column and code_point is the Unicode code point for that column.
        starts = dict(f["starts"])

        # 'nosplit' is a list of empty column pairs that should not cause a split for a new glyph.
        no_split:SplitList = sorted(f['no-split'], key=lambda x: x[0])
        # Guarantee that no_split[] will not be empty
        no_split += [(width, width)]

        empty_column = [0] * (height // 8)
        columns:list[int] = [] # The columns that make up an individual character

        for x in range(0, width):
            code_point = starts.get(x, code_point)
            column = scan_y(x, height, pixels)

            # We've scanned an entire vertical column. Now decide whether to keep it.
            no_split, splitable = is_splitable(no_split, x)
            if not splitable or column != empty_column:
                columns.extend(column)  # We're keeping this column.
            elif column == empty_column and len(columns) > 0:
                # A split point. Record this code-point and its columns.
                state.append_columns(code_point, columns)
                # Start a new code point.
                columns = []
                code_point += 1
            else:
                assert column == empty_column
    if height is None:
        return None
    return state.all_code_points(), height // 8


SIGNATURE = '''// This file was generated by font.py
// Do not edit!
'''

def dump_char(glyph:Union[Tuple[int, ...], int], height:int) -> None:
    if isinstance(glyph, int):
        print(f'duplicate of code-point {glyph}')
        return
    for column in reversed(range(0, len(glyph) // height)):
        for row in range(0, height):
            pos = column * height + row
            b = glyph[pos]
            c = 0
            for bit in range(0, 8):
                if b & (1 << bit) != 0:
                    c |= 1 << (7 - bit)
            print(f'{c:08b}', end='')
        print()


KernDictValue = List[Tuple[int, int]]
KernDict = Dict[int, KernDictValue]

def write_kerning_pairs(source:typing.TextIO, k:int, kdv:KernDictValue) -> None:
    source.write(f'constexpr std::array kern_{k:04x} = {{')
    separator = ''
    for prev_cp, distance in kdv:
        source.write(f'{separator}kerning_pair{{.preceding={prev_cp},.pad=0,.distance={distance}}}')
        separator = ','
    source.write('};\n')


def write_bitmap_data(source:typing.TextIO, k:int, data:Tuple[int, ...]) -> None:
    source.write(f'constexpr std::array bitmap_{k:04x} = {{')
    separator = ''
    for value in data:
        source.write(f'{separator}std::byte{{{value:#04x}}}')
        separator = ','
    source.write('};\n')


def write_source_file(font:FontDict,
                      kd:KernDict,
                      height:int,
                      output_dir:pathlib.Path,
                      definition:Dict[str, Any]) -> None:
    fid = int(definition['id'])
    name:str = definition['name']
    spacing = int(definition['spacing'])
    if name.find(os.sep) != -1:
        raise RuntimeError(f'font name ("{name}") must not contain a path separator')
    with open(os.path.join(output_dir, name + '.hpp'), 'w', encoding='utf-8') as source:
        guard = 'DRAW_FONT_' + name.upper() + '_HPP'
        data_ns = name + '_data'
        source.write(SIGNATURE)
        source.write(f'''
#ifndef {guard}
#define {guard}
#include <array>
#include <cassert>
#include "draw/font.hpp"
#include "draw/types.hpp"
namespace draw {{
namespace {data_ns} {{
''')
        widest = 0
        baseline = 32 - 8
        for k, v in font.items():
            if k in kd:
                write_kerning_pairs(source, k, kd[k])

            if not isinstance(v, int):
                widest = max(widest, len(v) // height)
                write_bitmap_data(source, k, v)

        source.write(f'''
}} // end namespace {data_ns}
constexpr font const {name} {{
  .id={fid},
  .baseline={baseline},
  .widest={widest},
  .height={height},
  .spacing={spacing},
  .glyphs={{
''')
        for k, v in font.items():
            kname = unicodedata.name(chr(k), '')
            if len(kname) > 0:
                kname = " // " + kname

            # If the value is an integer, this is a reference to a previous glyph.
            bm = v if isinstance(v, int) else k
            kp_name = f'{data_ns}::kern_{k:04x}' if k in kd else 'draw::empty_kern'
            source.write(f'    {{ {k:#04x}, glyph{{.kerns = decltype(draw::glyph::kerns)::from_array({kp_name}), .bm = decltype(draw::glyph::bm)::from_array({data_ns}::bitmap_{bm:04x})}} }},{kname}\n')
        source.write('  }\n};\n')
        source.write('} // end namespace draw\n')
        source.write(f'#endif // {guard}\n')


def str_to_cp(s) -> int:
    if not isinstance(s, str):
        return s
    if len(s) != 1:
        raise RuntimeError("string must be one character")
    return ord(s[0])


def uniqued(iterable, key=None):
    if key is None:
        key = lambda v: v
    seen = set()
    for v in iterable:
        k = key(v)
        if k not in seen:
            seen.add(k)
            yield v


JsonKernList = Dict[str, List[Tuple[str, int]]]

def kern_pairs(kl:JsonKernList) -> KernDict:
    """The kern list is a series of tuples which represent the previous code point, the current code
    point, and the distance by which the spacing between the glyphs should be reduced. The two code
    points can be specified as either integers or strings of length 1.

    Returns a dictionary whose keys are the current code points and whose values are a list of
    tuples for the previous code point and distance."""

    kernd:KernDict = {}
    for k,v in kl.items():
        for k2, dist in v:
            prev_cp = k
            curr_cp = k2
            distance = dist
            kernd.setdefault(str_to_cp(curr_cp), []).append((str_to_cp(prev_cp), distance))
    key = lambda x:x[0]
    return { k: sorted(uniqued(v, key=key), key=key) for k,v in kernd.items() }


def samples(font: FontDict, height:int) -> None:
    for c in sorted(font.keys()):
        name = unicodedata.name(chr(c), 'UNKNOWN')
        print(f'code point U+{c:04X} {name}:')
        dump_char(font[c], height)
        print()


def main() -> int:
    parser = argparse.ArgumentParser(description='Font Generator')
    parser.add_argument('file', help='JSON metadata describing the input files', type=pathlib.Path)
    parser.add_argument('-o', '--output-dir', help='Output directory', default=os.getcwd())
    parser.add_argument('--samples', help='Output samples', action='store_true')
    args = parser.parse_args()

    with open(args.file, 'r', encoding='utf-8') as fp:
        definition = json.load(fp)
    bfr = build_font(definition['glyphs'], args.file.parent)
    if bfr is None:
        return 1
    font, height = bfr

    if args.samples:
        samples(font, height)
    write_source_file(font,
                      kern_pairs(definition.get('kern', {})),
                      height,
                      args.output_dir,
                      definition)
    return 0

if __name__ == '__main__':
    sys.exit(main())
