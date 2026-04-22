#!/usr/bin/env python3
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
RESULTS = ROOT / "dev" / "docs" / "平台后端验证结果.md"

REQUIRED = [
	("Linux X11", "实现 Linux Sokol X11 路径"),
	("macOS", "实现 macOS Sokol 路径"),
	("Android", "实现 Android Sokol 路径"),
	("iOS", "实现 iOS Sokol 路径"),
	("Web/Emscripten", "实现 Web/Emscripten Sokol 路径"),
	("板卡 Linux EGL pbuffer", "实现板卡 Linux EGL 路径"),
	("EGL surfaceless", "在平台支持时实现 EGL surfaceless 上下文"),
]


def _read_text(path):
	return path.read_text(encoding="utf-8")


def _section_status(text, name):
	pattern = r"^##\s+" + re.escape(name) + r"\s*$([\s\S]*?)(?=^##\s+|\Z)"
	match = re.search(pattern, text, re.MULTILINE)
	if match is None:
		return "缺失"
	section = match.group(1)
	status = re.search(r"状态：\s*(\S+)", section)
	if status is None:
		return "缺失"
	return status.group(1).strip()


def main():
	if not RESULTS.exists():
		print(f"[XGE] Missing result file: {RESULTS}")
		return 2
	text = _read_text(RESULTS)
	closeable = []
	blocked = []
	for platform, spec in REQUIRED:
		status = _section_status(text, platform)
		if status == "通过":
			closeable.append((platform, spec))
		else:
			blocked.append((platform, spec, status))

	print("[XGE] Platform backend verification status")
	if closeable:
		print("\nCloseable:")
		for platform, spec in closeable:
			print(f"  [OK] {platform}: {spec}")
	else:
		print("\nCloseable: none")

	print("\nBlocked:")
	if blocked:
		for platform, spec, status in blocked:
			print(f"  [--] {platform}: {spec} (status={status})")
	else:
		print("  none")

	return 1 if blocked else 0


if __name__ == "__main__":
	sys.exit(main())
