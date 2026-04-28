#!/usr/bin/env python3

import argparse
import json
import pathlib
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

def run_rects(exe:pathlib.Path) -> subprocess.CompletedProcess:
    return subprocess.run(
        [exe, '-j', '-f100', '-d0'],
        capture_output=True,
        shell=False,
        cwd=None,
        timeout=10,
        check=True,
        encoding='utf-8'
    )

EXIT_SUCCESS = 0
EXIT_FAILURE = 1

def main() -> int:
    exit_code = EXIT_SUCCESS
    parser = argparse.ArgumentParser(prog='curses_rect_compare',
                                     description='Compare output from draw-curses-rect to the expected')
    parser.add_argument('exe', help='The draw-curses-rect binary', type=pathlib.Path)
    parser.add_argument('reference', help='JSON golden reference', type=pathlib.Path)
    args = parser.parse_args()

    completed = subprocess.run([args.exe, '-j', '-f100', '-d0'],
                   capture_output=True,
                   shell=False,
                   cwd=None,
                   timeout=10,
                   check=True,
                   encoding='utf-8')
    with open(args.reference, 'r', encoding='utf-8') as reference:
        for d in compare_json(json.loads(completed.stdout), json.load(reference)):
            exit_code = EXIT_FAILURE
            print(d)
    return exit_code

if __name__ == '__main__':
    sys.exit(main())
