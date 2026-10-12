#!/usr/bin/env python3
"""Discover and run repository test entry points from one stable command."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[2]
SUITES = {
    "xge": ROOT / "tests" / "xge",
    "xui": ROOT / "tests" / "xui",
    "xlayout": ROOT / "modules" / "xlayout" / "test",
}


def discover(suite: str) -> list[Path]:
    roots = SUITES.items() if suite == "all" else [(suite, SUITES[suite])]
    extension = ".bat" if os.name == "nt" else ".sh"
    scripts: list[Path] = []
    for _, root in roots:
        scripts.extend(root.rglob(f"build_*{extension}"))
        scripts.extend(root.rglob(f"check_*{extension}"))
    return sorted(set(scripts))


def display(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def select(scripts: list[Path], names: list[str]) -> list[Path]:
    if not names:
        return []
    selected: list[Path] = []
    for name in names:
        normalized = name.replace("\\", "/")
        matches = [
            path for path in scripts
            if display(path) == normalized
            or path.stem == normalized
            or display(path).removesuffix(path.suffix) == normalized
        ]
        if not matches:
            raise ValueError(f"unknown test entry: {name}")
        if len(matches) > 1:
            choices = ", ".join(display(path) for path in matches)
            raise ValueError(f"ambiguous test entry {name}: {choices}")
        if matches[0] not in selected:
            selected.append(matches[0])
    return selected


def run(path: Path, extra: list[str]) -> None:
    relative = display(path)
    print(f"[test] {relative}", flush=True)
    if path.suffix.lower() == ".bat":
        command = ["cmd", "/d", "/c", str(path), *extra]
    else:
        command = ["sh", str(path), *extra]
    subprocess.run(command, cwd=ROOT, check=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("targets", nargs="*", help="script stem or repository-relative path")
    parser.add_argument("--suite", choices=["all", *SUITES], default="all")
    parser.add_argument("--list", action="store_true", help="list available test entry points")
    parser.add_argument("--extra", nargs=argparse.REMAINDER, default=[], help="arguments passed to each script")
    args = parser.parse_args()

    scripts = discover(args.suite)
    if args.list:
        for script in scripts:
            print(display(script))
        return 0
    if not args.targets:
        parser.error("provide one or more test targets, or use --list")
    try:
        selected = select(scripts, args.targets)
    except ValueError as exc:
        parser.error(str(exc))
    for script in selected:
        run(script, args.extra)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as exc:
        raise SystemExit(exc.returncode or 1)
