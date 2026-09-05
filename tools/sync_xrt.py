#!/usr/bin/env python3
"""Regenerate the vendored XRT in an isolated snapshot, never in the source repo."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    args = parser.parse_args()
    source = args.source.resolve()
    target = Path(__file__).resolve().parents[1] / "lib" / "xrt"
    if not (source / "tools" / "amalgamate.py").is_file():
        parser.error("source must be an XRT source repository")
    with tempfile.TemporaryDirectory(prefix="xge-xrt-sync-") as directory:
        snapshot = Path(directory)
        for name in ("include", "src", "tools", "config"):
            shutil.copytree(source / name, snapshot / name,
                            ignore=shutil.ignore_patterns("__pycache__", "*.pyc"))
        shutil.copy2(source / "LICENSE", snapshot / "LICENSE")
        digest = hashlib.sha256()
        for path in sorted(p for p in snapshot.rglob("*") if p.is_file()):
            relative = path.relative_to(snapshot)
            content = path.read_bytes()
            if content != (source / relative).read_bytes():
                raise RuntimeError(f"XRT changed during snapshot: {relative}; retry")
            digest.update(relative.as_posix().encode() + b"\0" + hashlib.sha256(content).digest())
        subprocess.run([sys.executable, "tools/amalgamate.py"], cwd=snapshot, check=True)
        subprocess.run([sys.executable, "tools/amalgamate.py", "--check"], cwd=snapshot, check=True)
        content = (snapshot / "single" / "xrt.h").read_bytes()
        commit = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=source, text=True).strip()
        dirty = subprocess.check_output(["git", "status", "--porcelain", "--", "include", "src", "tools", "config", "LICENSE"], cwd=source, text=True)
        record = {"source_commit": commit, "source_worktree_dirty": bool(dirty),
                  "source_snapshot_sha256": digest.hexdigest(),
                  "header_sha256": hashlib.sha256(content).hexdigest(),
                  "generation": "tools/amalgamate.py in isolated source snapshot"}
        target.mkdir(parents=True, exist_ok=True)
        (target / "xrt.h").write_bytes(content)
        (target / "upstream.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
        print(f"Synced {len(content)} bytes; SHA256 {record['header_sha256']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
