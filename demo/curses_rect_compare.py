#!/usr/bin/env python3
# ===- demo/curses-rect_compare.py ----------------------------------------===//
#  Copyright © 2026 Paul Bowen-Huggett
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

import argparse
import json
from pathlib import Path
import subprocess
import sys
import typing

def compare_json(obj1:typing.Any, obj2:typing.Any, path:str="") -> list[str]:
    """
    Deep comparison of two objects (as returned by json.load()).

    Args:
        obj1: First JSON object
        obj2: Second JSON object
        path: Current key path (used internally for recursion)
    Returns:
        A list of strings describing all differences found.
    """
    diff = []

    if type(obj1) != type(obj2):
        diff.append(
            f"[{path}] Type mismatch: {type(obj1).__name__} vs {type(obj2).__name__} "
            f"({repr(obj1)} vs {repr(obj2)})"
        )
        return diff

    if isinstance(obj1, dict):
        keys1 = set(obj1.keys())
        keys2 = set(obj2.keys())
        for key in keys1 - keys2:
            diff.append(f"[{path}.{key}] Key only in first object: {repr(obj1[key])}")
        for key in keys2 - keys1:
            diff.append(f"[{path}.{key}] Key only in second object: {repr(obj2[key])}")
        for key in keys1 & keys2:
            diff.extend(compare_json(obj1[key], obj2[key], f"{path}.{key}"))
    elif isinstance(obj1, list):
        if len(obj1) != len(obj2):
            diff.append(f"[{path}] List length mismatch: {len(obj1)} vs {len(obj2)}")
        for i, (item1, item2) in enumerate(zip(obj1, obj2)):
            diff.extend(compare_json(item1, item2, f"{path}[{i}]"))
    elif obj1 != obj2:
        diff.append(f"[{path}] Value mismatch: {repr(obj1)} vs {repr(obj2)}")
    return diff


def is_inside_project_tree(inp:Path|str) -> bool:
    inp = Path(inp).resolve()
    ok = False
    project_tree = Path(__file__).parents[1]
    for root, _, files in project_tree.walk(on_error=print):
        r = Path(root).resolve()
        ok = ok or any(r / f == inp for f in files)
    return ok


def run_rects(exe:Path) -> subprocess.CompletedProcess:
    if not is_inside_project_tree(exe):
        raise RuntimeError(f'path {exe} is not inside the project tree')

    return subprocess.run(
        [exe, '-j', '-f100', '-d0'],
        capture_output=True,
        shell=False,
        cwd=None,
        timeout=10,
        check=True,
        encoding='utf-8'
    )

def compare(rects_out:str, reference:Path) -> str|None:
    if not is_inside_project_tree(reference):
        raise RuntimeError(f'path {reference} is not inside the project tree')

    with open(reference, 'r', encoding='utf-8') as r:
        for d in compare_json(json.loads(rects_out), json.load(r)):
            return d
    return None

EXIT_SUCCESS = 0
EXIT_FAILURE = 1

def main() -> int:
    exit_code = EXIT_SUCCESS
    parser = argparse.ArgumentParser(prog='curses_rect_compare',
                                     description='Compare output from draw-curses-rect to the expected')
    parser.add_argument('exe', help='The draw-curses-rect binary', type=Path)
    parser.add_argument('reference', help='JSON golden reference', type=Path)
    args = parser.parse_args()

    completed = run_rects(args.exe)

    compare_result = compare(completed.stdout, args.reference)
    if compare_result is None:
        exit_code = EXIT_SUCCESS
    else:
        exit_code = EXIT_FAILURE
        print(compare_result)
    return exit_code

if __name__ == '__main__':
    sys.exit(main())
