#!/usr/bin/env python3
import argparse
import re
import sys
from pathlib import Path


REQUIRED_TEXT = (
	"Platform Backend:",
	"Sokol Target:",
	"Graphics Backend:",
	"RenderTarget:",
	"OpenGL Version:",
	"Platform Runtime",
)


def _read_text(path):
	if path == "-":
		return sys.stdin.read()
	return Path(path).read_text(encoding="utf-8", errors="replace")


def _runtime_matches(text):
	return re.findall(
		r"Platform Runtime(?: Final)?:\s+running=(\d+)\s+window=(\d+)x(\d+)\s+framebuffer=(\d+)x(\d+)\s+dpi=([0-9.]+)",
		text,
	)


def main():
	parser = argparse.ArgumentParser(description="Check XGE platform smoke output for required caps/runtime lines.")
	parser.add_argument("log", nargs="?", default="-", help="Log file path, or '-' for stdin")
	parser.add_argument("--require-final", action="store_true", help="Require Platform Runtime Final line")
	args = parser.parse_args()

	text = _read_text(args.log)
	missing = [item for item in REQUIRED_TEXT if item not in text]
	runtimes = _runtime_matches(text)
	if not runtimes:
		missing.append("valid Platform Runtime dimensions")
	if args.require_final and "Platform Runtime Final:" not in text:
		missing.append("Platform Runtime Final:")

	if missing:
		print("[XGE] Platform smoke log check failed")
		for item in missing:
			print(f"  missing: {item}")
		return 1

	last = runtimes[-1]
	window_w = int(last[1])
	window_h = int(last[2])
	framebuffer_w = int(last[3])
	framebuffer_h = int(last[4])
	dpi = float(last[5])
	if window_w <= 0 or window_h <= 0 or framebuffer_w <= 0 or framebuffer_h <= 0 or dpi <= 0.0:
		print("[XGE] Platform smoke log check failed")
		print("  invalid runtime dimensions or dpi")
		return 1

	print("[XGE] Platform smoke log check passed")
	print(f"  window={window_w}x{window_h} framebuffer={framebuffer_w}x{framebuffer_h} dpi={dpi:.2f}")
	return 0


if __name__ == "__main__":
	sys.exit(main())
