"""Build/run additive foundation checks against an explicit, isolated SDK."""
import argparse
import json
import subprocess
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("profile", type=Path)
    parser.add_argument("tests", nargs="*", default=["foundation", "cpu", "spatial", "import", "animation", "terrain", "async"])
    args = parser.parse_args()
    profile = args.profile.resolve()
    results = []
    for name in args.tests:
        exe = profile / f"test_3d_{name}.exe"
        frozen = profile.parent / "source"
        headers = ["-I" + str(frozen / "include"), "-I" + str(frozen)] if frozen.is_dir() else []
        command = ["gcc", *headers, "@config/includes.rsp", "-O2", "-Wall", "-Wextra", "-Werror",
                   "-Wno-missing-field-initializers", "-DXGE_DLL", "-DXUI_DLL", "-include",
                   profile / "xge_build_config.h", f"tests/xge/test_3d_{name}.c",
                   profile / "xge.lib", "-lm", "-o", exe]
        started = time.perf_counter()
        compiled = subprocess.run([str(x) for x in command], cwd=ROOT, capture_output=True, text=True)
        result = subprocess.run([str(exe)], cwd=ROOT, capture_output=True, text=True, timeout=180) if compiled.returncode == 0 else compiled
        item = {"test": name, "compile_exit": compiled.returncode, "exit": result.returncode,
                "seconds": time.perf_counter() - started, "stdout": result.stdout, "stderr": result.stderr,
                "compile_stderr": compiled.stderr}
        results.append(item)
        (profile / "foundation-tests.json").write_text(json.dumps(results, indent=2), encoding="utf-8")
        print(name, result.returncode, result.stdout.strip(), result.stderr.strip(), flush=True)
        if result.returncode:
            raise SystemExit(1)


if __name__ == "__main__":
    main()
