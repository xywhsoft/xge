#!/usr/bin/env python3
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


SCRIPT_RE = re.compile(r"(?:^|[\s`])(\.\/)?([A-Za-z0-9_./\\-]+(?:\.\*|\.bat|\.sh))(?:$|[\s`])")


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
	docs = [Path("README.md")]
	docs.extend(sorted(Path("docs").rglob("*.md")))
	for doc in docs:
		relpath = doc.as_posix()
		path = ROOT / relpath
		if not path.exists():
			errors.append(f"missing doc: {relpath}")
			continue
		text = path.read_text(encoding="utf-8")
		for match in SCRIPT_RE.finditer(text):
			name = match.group(2).replace("\\", "/")
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
