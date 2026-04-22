#!/usr/bin/env python3
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DOCS = [
	"docs/EXAMPLES.md",
	"docs/guide/first-xge-program.md",
	"docs/case/minimal-window.md",
	"dev/docs/构建脚本说明.md",
	"dev/docs/Sokol跨平台冒烟测试计划.md",
	"dev/docs/人工冒烟测试流程.md",
	"dev/docs/板卡Linux冒烟测试计划.md",
	"dev/docs/平台后端验证结果.md",
]


SCRIPT_RE = re.compile(r"(?:^|[\s`])(\.\/)?([A-Za-z0-9_]+(?:\.\*|\.bat|\.sh))(?:$|[\s`])")


def _expand_script(name):
	if "xxx" in name:
		return []
	if name.endswith(".*"):
		base = name[:-2]
		return [base + ".bat", base + ".sh"]
	return [name]


def main():
	errors = []
	seen = set()
	for relpath in DOCS:
		path = ROOT / relpath
		if not path.exists():
			errors.append(f"missing doc: {relpath}")
			continue
		text = path.read_text(encoding="utf-8")
		for match in SCRIPT_RE.finditer(text):
			name = match.group(2)
			for script in _expand_script(name):
				key = (relpath, script)
				if key in seen:
					continue
				seen.add(key)
				if not (ROOT / script).exists():
					errors.append(f"{relpath}: missing referenced script `{script}`")

	if errors:
		print("[XGE] Script documentation check failed")
		for error in errors:
			print(f"  [FAIL] {error}")
		return 1
	print("[XGE] Script documentation check passed")
	return 0


if __name__ == "__main__":
	sys.exit(main())
