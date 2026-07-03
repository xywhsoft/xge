#!/usr/bin/env python3
import re
import sys
from pathlib import Path
from urllib.parse import unquote


ROOT = Path(__file__).resolve().parents[1]
DOC_ROOTS = [
	ROOT / "README.md",
	ROOT / "docs",
]
SPEC = ROOT / "dev" / "XGE_DOCUMENTATION_SPEC.md"

LINK_RE = re.compile(r"\[[^\]]+\]\(([^)]+)\)")
SPEC_DOC_RE = re.compile(r"`((?:README(?:\.en)?|docs/|dev/docs/)[^`]+?\.md)`")
API_RE = re.compile(r"^XGE_API\s+.+?\s+(xge[A-Za-z0-9_]+)\s*\(", re.MULTILINE)


def _read_text(path):
	return path.read_text(encoding="utf-8")


def _iter_docs():
	for root in DOC_ROOTS:
		if root.is_file():
			yield root
		elif root.is_dir():
			for path in root.rglob("*.md"):
				yield path


def _normalize_ref(base, ref):
	ref = ref.strip()
	if not ref or ref.startswith("#"):
		return None, None
	if "://" in ref or ref.startswith("mailto:"):
		return None, None
	path_part, _, anchor = ref.partition("#")
	path_part = unquote(path_part)
	if not path_part:
		return None, anchor
	path = (base.parent / path_part).resolve()
	return path, anchor


def _planned_docs():
	planned = set()
	if not SPEC.exists():
		return planned
	text = _read_text(SPEC)
	for match in SPEC_DOC_RE.finditer(text):
		rel = match.group(1).replace("\\", "/")
		planned.add((ROOT / rel).resolve())
	return planned


def _anchor_exists(path, anchor):
	if not anchor:
		return True
	if not path.exists():
		return True
	text = _read_text(path)
	if f'id="{anchor}"' in text or f"id='{anchor}'" in text:
		return True
	needle = anchor.lower()
	for line in text.splitlines():
		if not line.startswith("#"):
			continue
		title = line.lstrip("#").strip().lower()
		slug = re.sub(r"[^\w\u4e00-\u9fff -]", "", title)
		slug = re.sub(r"\s+", "-", slug)
		if slug == needle:
			return True
	return False


def _check_api_coverage(errors):
	header = ROOT / "xge.h"
	api_dir = ROOT / "docs" / "api"
	if not header.exists():
		errors.append("missing xge.h for API coverage check")
		return
	if not api_dir.exists():
		errors.append("missing docs/api for API coverage check")
		return

	header_text = _read_text(header)
	api_names = sorted(set(API_RE.findall(header_text)))
	api_text = "\n".join(_read_text(path) for path in api_dir.glob("*.md"))
	missing = [name for name in api_names if name not in api_text]
	if missing:
		for name in missing:
			errors.append(f"xge.h: public API `{name}` is not covered by docs/api/*.md")


def _english_path_for(path):
	if path == ROOT / "README.md":
		return ROOT / "README.en.md"
	return path.with_name(path.stem + ".en.md")


def _check_english_coverage(errors):
	for doc in _iter_docs():
		if doc.name.endswith(".en.md"):
			continue
		target = _english_path_for(doc)
		if not target.exists():
			errors.append(f"{doc.relative_to(ROOT).as_posix()}: missing English document `{target.relative_to(ROOT).as_posix()}`")


def main():
	errors = []
	warnings = []
	planned = _planned_docs()

	for doc in _iter_docs():
		text = _read_text(doc)
		rel_doc = doc.relative_to(ROOT).as_posix()
		for match in LINK_RE.finditer(text):
			ref = match.group(1)
			target, anchor = _normalize_ref(doc, ref)
			if target is None:
				continue
			if not target.exists():
				if target in planned:
					warnings.append(f"{rel_doc}: planned doc not generated yet `{ref}`")
				else:
					errors.append(f"{rel_doc}: missing linked doc `{ref}`")
				continue
			if target.suffix.lower() == ".md" and not _anchor_exists(target, anchor):
				errors.append(f"{rel_doc}: missing anchor `{ref}`")

	_check_api_coverage(errors)
	_check_english_coverage(errors)

	if warnings:
		print("[XGE] Documentation check warnings")
		for warning in warnings:
			print(f"  [WARN] {warning}")

	if errors:
		print("[XGE] Documentation check failed")
		for error in errors:
			print(f"  [FAIL] {error}")
		return 1

	print("[XGE] Documentation check passed")
	return 0


if __name__ == "__main__":
	sys.exit(main())
