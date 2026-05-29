#!/usr/bin/env python3
import argparse
import datetime
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
RESULTS = ROOT / "dev" / "docs" / "平台后端验证结果.md"
VALID_STATUS = ("未测", "通过", "失败", "跳过")


def _read_text(path):
	return path.read_text(encoding="utf-8")


def _write_text(path, text):
	path.write_text(text, encoding="utf-8", newline="\n")


def _section_match(text, platform):
	pattern = r"(^##\s+" + re.escape(platform) + r"\s*$)([\s\S]*?)(?=^##\s+|\Z)"
	return re.search(pattern, text, re.MULTILINE)


def _available_platforms(text):
	return re.findall(r"^##\s+(.+?)\s*$", text, re.MULTILINE)


def _make_entry(args):
	now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
	lines = [
		"",
		"最近记录：",
		"",
		"```text",
		f"时间：{now}",
		f"状态：{args.status}",
	]
	if args.env:
		lines.append(f"环境：{args.env}")
	if args.scripts:
		lines.append(f"脚本：{args.scripts}")
	if args.summary:
		lines.append(f"结果：{args.summary}")
	if args.log:
		lines.append(f"日志：{args.log}")
	lines.append("```")
	lines.append("")
	return "\n".join(lines)


def _update_section(section, args):
	if re.search(r"状态：\s*\S+", section) is None:
		section = "\n状态：未测\n" + section
	section = re.sub(r"状态：\s*\S+", f"状态：{args.status}", section, count=1)
	return section.rstrip() + "\n" + _make_entry(args)


def main():
	parser = argparse.ArgumentParser(description="Record XGE platform backend smoke test result.")
	parser.add_argument("platform", nargs="?", help="Platform section name in dev/docs/平台后端验证结果.md")
	parser.add_argument("status", nargs="?", choices=VALID_STATUS, help="Result status")
	parser.add_argument("--env", default="", help="Environment summary")
	parser.add_argument("--scripts", default="", help="Executed scripts")
	parser.add_argument("--summary", default="", help="Observed result summary")
	parser.add_argument("--log", default="", help="Key log lines")
	parser.add_argument("--dry-run", action="store_true", help="Print updated section without writing")
	parser.add_argument("--list", action="store_true", help="List available platform section names")
	args = parser.parse_args()

	if not RESULTS.exists():
		print(f"[XGE] Missing result file: {RESULTS}")
		return 2
	text = _read_text(RESULTS)
	if args.list:
		for platform in _available_platforms(text):
			print(platform)
		return 0
	if not args.platform or not args.status:
		parser.error("platform and status are required unless --list is used")

	match = _section_match(text, args.platform)
	if match is None:
		print(f"[XGE] Platform section not found: {args.platform}")
		print("[XGE] Available platforms:")
		for platform in _available_platforms(text):
			print(f"  {platform}")
		return 2

	heading = match.group(1)
	section = match.group(2)
	updated_section = _update_section(section, args)
	updated_text = text[:match.start()] + heading + updated_section + text[match.end():]
	if args.dry_run:
		print(heading + updated_section)
		return 0
	_write_text(RESULTS, updated_text)
	print(f"[XGE] Recorded {args.platform}: {args.status}")
	return 0


if __name__ == "__main__":
	sys.exit(main())
