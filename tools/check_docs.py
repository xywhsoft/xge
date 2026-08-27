#!/usr/bin/env python3
"""Validate the small, user-facing release documentation set."""

import re
import sys
from pathlib import Path
from urllib.parse import unquote


ROOT = Path(__file__).resolve().parents[1]
DOCS = [ROOT / "README.md", ROOT / "README.en.md"]
DOCS.extend(sorted((ROOT / "docs").glob("*.md")))
DOCS.extend(
	ROOT / "platform" / name / "README.md"
	for name in ("android", "ios", "web")
)
LINK_RE = re.compile(r"\[[^\]]+\]\(([^)]+)\)")
FORBIDDEN = ("dev/", "dev\\", "dev/xui", "dev\\xui")


def resolve_link(base, reference):
	reference = reference.strip()
	if not reference or reference.startswith("#") or "://" in reference or reference.startswith("mailto:"):
		return None
	path_text, _, _ = unquote(reference).partition("#")
	if not path_text:
		return None
	return (base.parent / path_text).resolve()


def main():
	errors = []
	for doc in DOCS:
		if not doc.exists():
			errors.append(f"missing release document: {doc.relative_to(ROOT)}")
			continue
		text = doc.read_text(encoding="utf-8")
		for forbidden in FORBIDDEN:
			if forbidden in text:
				errors.append(f"{doc.relative_to(ROOT)}: release document refers to legacy path `{forbidden}`")
		for match in LINK_RE.finditer(text):
			target = resolve_link(doc, match.group(1))
			if target is not None and not target.exists():
				errors.append(f"{doc.relative_to(ROOT)}: missing link `{match.group(1)}`")
	if errors:
		print("[XGE] Documentation check failed")
		for error in errors:
			print(f"  [FAIL] {error}")
		return 1
	print("[XGE] Documentation check passed")
	return 0


if __name__ == "__main__":
	sys.exit(main())
