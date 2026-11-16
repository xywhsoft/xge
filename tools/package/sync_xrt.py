#!/usr/bin/env python3
"""Regenerate the vendored XRT in an isolated snapshot, never in the source repo."""
from __future__ import annotations

import argparse
import hashlib
import io
import json
from pathlib import Path, PurePosixPath
import shutil
import subprocess
import sys
import tempfile
import zipfile

EXTENSIONS = ("xllm", "xllm-session", "xwork")
LF_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".inl", ".inc", ".py", ".sh", ".json", ".md", ".yml", ".yaml"}


def extension_closure(read_manifest, requested=EXTENSIONS):
    """Freeze each declared extension dependency before generating consumers.

    dependency_manifests are repository-relative paths in XRT's format. Use
    the selected commit's manifests when exporting a ref, never live worktree
    dependencies from a potentially different revision.
    """
    ordered, states = [], {}

    def visit(name):
        if not isinstance(name, str) or not name or name in (".", "..") or any(
            not (c.isascii() and (c.isalnum() or c in "_.-")) for c in name
        ):
            raise ValueError("Invalid extension directory: " + str(name))
        if states.get(name) == 2:
            return
        if states.get(name) == 1:
            raise ValueError("Extension dependency cycle at: " + name)
        states[name] = 1
        manifest = read_manifest(f"extlibs/{name}/config/modules.json")
        dependencies = manifest.get("dependency_manifests", [])
        if not isinstance(dependencies, list):
            raise ValueError("dependency_manifests must be an array: " + name)
        for dependency in dependencies:
            if not isinstance(dependency, str):
                raise ValueError("Invalid dependency manifest: " + name)
            path = PurePosixPath(dependency)
            if (path.is_absolute() or "\\" in dependency or ":" in dependency or
                len(path.parts) != 4 or path.parts[0] != "extlibs" or
                path.parts[2:] != ("config", "modules.json")):
                raise ValueError("Expected an extension dependency manifest: " + dependency)
            visit(path.parts[1])
        states[name] = 2
        ordered.append(name)

    for name in requested:
        visit(name)
    return tuple(ordered)


def canonical_bytes(path: Path) -> bytes:
    data = path.read_bytes()
    return data.replace(b"\r\n", b"\n") if path.suffix.lower() in LF_SUFFIXES else data


def snapshot_files(root: Path):
    # WindowsPath orders case-insensitively; PosixPath does not. Provenance
    # must describe the same exported source bytes identically on both hosts.
    return sorted((p for p in root.rglob("*") if p.is_file()),
                  key=lambda p: p.relative_to(root).as_posix())


def archive_snapshot(source: Path, commit: str, roots):
    # Git archive applies checkout EOL conversion. Never let the user's
    # core.autocrlf/platform default change an immutable commit's bytes.
    return subprocess.check_output(["git", "-c", "core.autocrlf=false", "-c", "core.eol=lf",
                                    "archive", "--format=zip", commit, "--", *roots], cwd=source)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("--base-ref", help="Generate from an immutable source commit rather than the current worktree")
    parser.add_argument("--overlay", action="append", default=[], help="Explicit worktree file to apply on top of --base-ref")
    parser.add_argument("--check", action="store_true", help="Compare generated public headers without replacing the vendored snapshot")
    args = parser.parse_args()
    source = args.source.resolve()
    target = Path(__file__).resolve().parents[2] / "extlibs" / "xrt"
    if not (source / "tools" / "amalgamate.py").is_file():
        parser.error("source must be an XRT source repository")
    if args.overlay and not args.base_ref:
        parser.error("--overlay requires --base-ref")
    with tempfile.TemporaryDirectory(prefix="xge-xrt-sync-") as directory:
        snapshot = Path(directory)
        commit = subprocess.check_output(["git", "rev-parse", f"{args.base_ref or 'HEAD'}^{{commit}}"], cwd=source, text=True).strip()
        manifest_overlays = {}
        def read_manifest(name):
            if args.base_ref and name not in args.overlay:
                data = subprocess.check_output(["git", "show", f"{commit}:{name}"], cwd=source)
            else:
                path = (source / name).resolve()
                if source not in path.parents or not path.is_file():
                    raise ValueError("Invalid manifest source: " + name)
                data = path.read_bytes()
                if args.base_ref:
                    manifest_overlays[name] = data
            result = json.loads(data.decode("utf-8-sig"))
            if not isinstance(result, dict):
                raise ValueError("Extension manifest must be an object: " + name)
            return result
        extensions = extension_closure(read_manifest)
        overlays = {}
        if args.base_ref:
            roots = ["include", "src", "tools", "config", "LICENSE", *[f"extlibs/{p}" for p in extensions]]
            archive = archive_snapshot(source, commit, roots)
            with zipfile.ZipFile(io.BytesIO(archive)) as files:
                for item in files.infolist():
                    name = PurePosixPath(item.filename)
                    if name.is_absolute() or ".." in name.parts or ":" in item.filename or "\\" in item.filename or ((item.external_attr >> 16) & 0o170000) == 0o120000:
                        raise RuntimeError(f"Unsafe archived path: {item.filename}")
                files.extractall(snapshot)
            for name in args.overlay:
                relative = PurePosixPath(name)
                path = (source / name).resolve()
                if relative.is_absolute() or ".." in relative.parts or ":" in name or "\\" in name or source not in path.parents or not path.is_file():
                    raise RuntimeError(f"Unsafe overlay path: {name}")
                if name not in roots and not any(name.startswith(root + "/") for root in roots):
                    raise RuntimeError(f"Overlay outside source roots: {name}")
                data = path.read_bytes()
                if name in manifest_overlays and data != manifest_overlays[name]:
                    raise RuntimeError(f"Manifest overlay changed during dependency resolution: {name}")
                dest = snapshot / name
                dest.parent.mkdir(parents=True, exist_ok=True)
                dest.write_bytes(data)
                if data != path.read_bytes():
                    raise RuntimeError(f"Overlay changed during snapshot: {name}")
                overlays[name] = hashlib.sha256(data).hexdigest()
        else:
            for name in ("include", "src", "tools", "config"):
                shutil.copytree(source / name, snapshot / name,
                                ignore=shutil.ignore_patterns("__pycache__", "*.pyc"))
            shutil.copy2(source / "LICENSE", snapshot / "LICENSE")
            for product in extensions:
                shutil.copytree(source / "extlibs" / product, snapshot / "extlibs" / product,
                                ignore=shutil.ignore_patterns("__pycache__", "*.pyc", "build"))
        digest = hashlib.sha256()
        for path in snapshot_files(snapshot):
            relative = path.relative_to(snapshot)
            content = path.read_bytes()
            if not args.base_ref and content != (source / relative).read_bytes():
                raise RuntimeError(f"XRT changed during snapshot: {relative}; retry")
            digest.update(relative.as_posix().encode() + b"\0" + hashlib.sha256(content).digest())
        subprocess.run([sys.executable, "tools/amalgamate.py"], cwd=snapshot, check=True)
        subprocess.run([sys.executable, "tools/amalgamate.py", "--check"], cwd=snapshot, check=True)
        for product in extensions:
            manifest = f"extlibs/{product}/config/modules.json"
            subprocess.run([sys.executable, "tools/amalgamate.py", "--manifest", manifest], cwd=snapshot, check=True)
            subprocess.run([sys.executable, "tools/amalgamate.py", "--manifest", manifest, "--check"], cwd=snapshot, check=True)
        # Match this repository's text eol=lf attributes before hashing. A
        # checkout must not invalidate the dependency's recorded identity.
        content = canonical_bytes(snapshot / "single" / "xrt.h")
        declarations = canonical_bytes(snapshot / "single" / "xrt_decl.h")
        copied = {}
        trees = [(snapshot / "include", target / "include")]
        trees += [(snapshot / "extlibs" / product / name, target / "extlibs" / product / name)
                  for product in extensions for name in ("include", "src", "config")]
        for origin, destination in trees:
            for path in sorted(p for p in origin.rglob("*") if p.is_file()):
                copied[destination / path.relative_to(origin)] = canonical_bytes(path)
        dirty = subprocess.check_output(["git", "status", "--porcelain", "--", "include", "src", "tools", "config", "LICENSE",
                                         *[f"extlibs/{p}" for p in extensions]], cwd=source, text=True)
        branch = subprocess.check_output(["git", "branch", "--show-current"], cwd=source, text=True).strip()
        record = {"source_repository": str(source), "source_branch": branch,
                  "source_commit": commit, "source_worktree_dirty": bool(dirty),
                  "source_snapshot_sha256": digest.hexdigest(),
                  "header_sha256": hashlib.sha256(content).hexdigest(),
                  "declaration_header_sha256": hashlib.sha256(declarations).hexdigest(),
                  "generation": "tools/amalgamate.py in isolated source snapshot"}
        record["text_eol"] = "lf"
        if args.base_ref:
            record["snapshot_mode"] = "immutable_base_with_explicit_overlays"
            record["overlay_files"] = overlays
        record["requested_extensions"] = list(EXTENSIONS)
        record["extensions"] = list(extensions)
        if args.check:
            if content != (target / "xrt.h").read_bytes() or declarations != (target / "xrt_decl.h").read_bytes():
                import difflib
                diff = difflib.unified_diff((target / "xrt.h").read_text(encoding="utf-8").splitlines(), content.decode("utf-8").splitlines(), n=2)
                lines = list(diff)
                print(f"Generated header SHA256 {record['header_sha256']}; declarations {record['declaration_header_sha256']}; diff lines {len(lines)}")
                print("\n".join(lines[:60]))
                raise RuntimeError("Generated headers differ from the vendored snapshot")
            for path, expected in copied.items():
                if not path.is_file() or path.read_bytes() != expected:
                    raise RuntimeError(f"Vendored source differs from the snapshot: {path}")
            installed = json.loads((target / "upstream.json").read_text(encoding="utf-8"))
            for key in ("source_commit", "source_snapshot_sha256", "header_sha256", "declaration_header_sha256", "extensions", "text_eol"):
                if installed.get(key) != record[key]:
                    raise RuntimeError(f"Vendored provenance differs from the snapshot: {key}")
            print(f"Verified vendored baseline {commit}; no files replaced")
            return 0
        target.mkdir(parents=True, exist_ok=True)
        for path, data in copied.items():
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
        (target / "xrt.h").write_bytes(content)
        (target / "xrt_decl.h").write_bytes(declarations)
        (target / "upstream.json").write_bytes((json.dumps(record, indent=2) + "\n").encode("utf-8"))
        print(f"Synced {len(content)} bytes; SHA256 {record['header_sha256']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
