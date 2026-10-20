"""Run existing workshop regression binaries with an isolated replacement SDK DLL."""
import argparse
import hashlib
import json
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("sdk", type=Path)
    parser.add_argument("workshop", type=Path)
    parser.add_argument("--reuse-binaries", action="store_true", help="retain already captured baseline applications")
    args = parser.parse_args()
    sdk, workshop = args.sdk.resolve(), args.workshop.resolve()
    out = sdk.parent / "workshop-compatibility"
    out.mkdir(exist_ok=True)
    shutil.copy2(sdk / "xge.dll", out / "xge.dll")
    tests = ["test_shared_renderer", "test_spatial_runtime", "test_scene_setup_runtime", "test_model_ownership_host"]
    results = []
    for name in tests:
        original = workshop / "build/cmake-spatial-editor-ninja" / f"{name}.exe"
        if not original.is_file():
            raise FileNotFoundError(original)
        target = out / original.name
        if not args.reuse_binaries:
            shutil.copy2(original, target)
        elif not target.is_file():
            raise FileNotFoundError(target)
        result = subprocess.run([str(target)], cwd=workshop, capture_output=True, timeout=120)
        item = {"test": name, "binary_sha256": hashlib.sha256(target.read_bytes()).hexdigest(),
                "engine_sha256": hashlib.sha256((out / "xge.dll").read_bytes()).hexdigest(),
                "exit": result.returncode, "stdout": result.stdout.decode("utf-8", errors="replace"),
                "stderr": result.stderr.decode("utf-8", errors="replace"), "rebuilt_application": False}
        results.append(item)
        (out / "verification.json").write_text(json.dumps(results, indent=2, ensure_ascii=False), encoding="utf-8")
        print(name, item["exit"], item["stdout"].strip(), item["stderr"].strip(), flush=True)
        if result.returncode:
            raise SystemExit(1)


if __name__ == "__main__":
    main()
