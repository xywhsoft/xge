#!/usr/bin/env python3
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

SKIP_DIRS = {
	".git",
	"artifacts",
	"build",
	"dev",
	"lib",
}

CPP_SOURCE_SUFFIXES = {
	".cc",
	".cpp",
	".cxx",
	".hh",
	".hpp",
	".hxx",
}

BUILD_CONFIG_SUFFIXES = {
	".bat",
	".cmd",
	".cmake",
	".gradle",
	".mk",
	".props",
	".ps1",
	".py",
	".sh",
	".sln",
	".targets",
	".txt",
	".vcxproj",
}

BUILD_CONFIG_NAMES = {
	"CMakeLists.txt",
	"Makefile",
}

CPP_COMPILER_RE = re.compile(
	r"(^|[^A-Za-z0-9_])("
	r"cl\+\+|g\+\+|c\+\+|clang\+\+|"
	r"/TP|std:c\+\+|std=gnu\+\+|std=c\+\+|"
	r"enable_language\s*\([^)]*\bCXX\b|"
	r"project\s*\([^)]*\bCXX\b|"
	r"LANGUAGES\s+[^)\r\n]*\bCXX\b|"
	r"ClCompile|CppCompile|"
	r"\.cpp\b|\.cxx\b|\.cc\b"
	r")([^A-Za-z0-9_]|$)",
	re.IGNORECASE,
)


def is_skipped(path):
	try:
		rel = path.relative_to(ROOT)
	except ValueError:
		return True
	parts = set(rel.parts)
	return any(part in SKIP_DIRS for part in parts)


def iter_files():
	for path in ROOT.rglob("*"):
		if not path.is_file() or is_skipped(path):
			continue
		yield path


def rel(path):
	return path.relative_to(ROOT).as_posix()


def check_cpp_sources(errors):
	for path in iter_files():
		if path.suffix.lower() in CPP_SOURCE_SUFFIXES:
			errors.append(f"{rel(path)}: C++ source/header is not allowed outside dev/")


def check_cpp_build_entries(errors):
	self_path = Path(__file__).resolve()
	for path in iter_files():
		if path.resolve() == self_path:
			continue
		if path.suffix.lower() not in BUILD_CONFIG_SUFFIXES and path.name not in BUILD_CONFIG_NAMES:
			continue
		try:
			text = path.read_text(encoding="utf-8", errors="replace")
		except OSError as exc:
			errors.append(f"{rel(path)}: failed to read script: {exc}")
			continue
		for line_no, line in enumerate(text.splitlines(), 1):
			if CPP_COMPILER_RE.search(line):
				errors.append(f"{rel(path)}:{line_no}: C++ compiler/source reference is not allowed")


def main():
	errors = []
	check_cpp_sources(errors)
	check_cpp_build_entries(errors)
	if errors:
		print("[XGE] C language boundary check failed")
		for error in errors:
			print(f"  [FAIL] {error}")
		return 1
	print("[XGE] C language boundary check passed")
	return 0


if __name__ == "__main__":
	sys.exit(main())
