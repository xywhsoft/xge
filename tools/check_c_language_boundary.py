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
}

CPP_SOURCE_SUFFIXES = {
	".c++",
	".cc",
	".cpp",
	".cxx",
	".h++",
	".hh",
	".hpp",
	".hxx",
	".ixx",
	".mpp",
	".mm",
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

SOURCE_SUFFIXES = {
	".c",
	".h",
}

CPP_COMPILER_RE = re.compile(
	r"(^|[^A-Za-z0-9_])("
	r"cl\+\+|g\+\+|c\+\+|clang\+\+|"
	r"/TP|std:c\+\+|std=gnu\+\+|std=c\+\+|"
	r"-lstdc\+\+|libstdc\+\+|"
	r"enable_language\s*\([^)]*\bCXX\b|"
	r"project\s*\([^)]*\bCXX\b|"
	r"LANGUAGES\s+[^)\r\n]*\bCXX\b|"
	r"ClCompile|CppCompile|"
	r"\.cpp\b|\.cxx\b|\.cc\b"
	r")([^A-Za-z0-9_]|$)",
	re.IGNORECASE,
)

THORVG_INCLUDE_RE = re.compile(
	r"^\s*#\s*include\s*[<\"][^>\"]*(?:thorvg|tvg)[^>\"]*[>\"]",
	re.IGNORECASE,
)

THORVG_REFERENCE_RE = re.compile(
	r"("
	r"dev[\\/]+thorvg_reference|"
	r"thorvg_reference[\\/]|"
	r"thorvg\.h|"
	r"thorvg_capi\.h"
	r")",
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


def is_svg_compare_reference_tool(path):
	rel_path = rel(path)
	return rel_path.startswith("tools/svg_compare/") and path.suffix.lower() == ".ps1"


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
			if THORVG_REFERENCE_RE.search(line) and not is_svg_compare_reference_tool(path):
				errors.append(f"{rel(path)}:{line_no}: ThorVG reference path/header is not allowed outside dev/")


def check_thorvg_source_references(errors):
	self_path = Path(__file__).resolve()
	for path in iter_files():
		if path.resolve() == self_path:
			continue
		if path.suffix.lower() not in SOURCE_SUFFIXES:
			continue
		try:
			text = path.read_text(encoding="utf-8", errors="replace")
		except OSError as exc:
			errors.append(f"{rel(path)}: failed to read source: {exc}")
			continue
		for line_no, line in enumerate(text.splitlines(), 1):
			if THORVG_INCLUDE_RE.search(line):
				errors.append(f"{rel(path)}:{line_no}: ThorVG/tvg include is not allowed outside dev/")
			elif THORVG_REFERENCE_RE.search(line):
				errors.append(f"{rel(path)}:{line_no}: ThorVG reference path/header is not allowed outside dev/")


def run_self_test():
	bad_build = [
		"gcc src/xge.c dev\\thorvg_reference\\src\\tvgShape.cpp",
		"cl /Idev/thorvg_reference/inc src\\xge_svg.c",
		"gcc src/xge.c -I thorvg_reference/inc",
		"gcc src/xge.c thorvg_capi.h",
	]
	bad_source = [
		"#include <thorvg.h>",
		"#include \"thorvg_capi.h\"",
		"#include \"dev/thorvg_reference/inc/thorvg.h\"",
		"# include <tvgShape.h>",
	]
	good_lines = [
		"SVG ThorVG compatibility label only",
		"const char* name = \"svg_thorvg_subset\";",
		"/* ThorVG reference lives under dev/ and is not compiled. */",
	]
	allowed_reference_tool = ROOT / "tools" / "svg_compare" / "build_thorvg_svg2png.ps1"
	errors = []
	for line in bad_build:
		if (CPP_COMPILER_RE.search(line) is None) and (THORVG_REFERENCE_RE.search(line) is None):
			errors.append(f"self-test missed bad build line: {line}")
	for line in bad_source:
		if (THORVG_INCLUDE_RE.search(line) is None) and (THORVG_REFERENCE_RE.search(line) is None):
			errors.append(f"self-test missed bad source line: {line}")
	for line in good_lines:
		if THORVG_INCLUDE_RE.search(line) is not None or THORVG_REFERENCE_RE.search(line) is not None:
			errors.append(f"self-test false positive line: {line}")
	if not is_svg_compare_reference_tool(allowed_reference_tool):
		errors.append("self-test missed SVG compare reference tool allowlist")
	if errors:
		print("[XGE] C language boundary self-test failed")
		for error in errors:
			print(f"  [FAIL] {error}")
		return 1
	print("[XGE] C language boundary self-test passed")
	return 0


def main():
	if len(sys.argv) > 1 and sys.argv[1] == "--self-test":
		return run_self_test()
	errors = []
	check_cpp_sources(errors)
	check_cpp_build_entries(errors)
	check_thorvg_source_references(errors)
	if errors:
		print("[XGE] C language boundary check failed")
		for error in errors:
			print(f"  [FAIL] {error}")
		return 1
	print("[XGE] C language boundary check passed")
	return 0


if __name__ == "__main__":
	sys.exit(main())
