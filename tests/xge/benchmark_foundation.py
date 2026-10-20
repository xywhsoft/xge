"""Compare the same legacy-API CPU workload against two SDKs. No FPS claim."""
import argparse
import hashlib
import json
import shutil
import statistics
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("before", type=Path)
    parser.add_argument("after", type=Path)
    args = parser.parse_args()
    result = {}
    destination = args.after.resolve().parent / "benchmarks"
    destination.mkdir(exist_ok=True)
    for name, sdk in [("before", args.before.resolve()), ("after", args.after.resolve())]:
        out = destination / name
        out.mkdir(exist_ok=True)
        shutil.copy2(sdk / "xge.dll", out / "xge.dll")
        exe = out / "bench.exe"
        frozen = sdk.parent / "source"
        headers = ["-I" + str(frozen / "include"), "-I" + str(frozen)] if frozen.is_dir() else []
        command = ["gcc", *headers, "@config/includes.rsp", "-O2", "-Wall", "-Wextra", "-Werror", "-Wno-missing-field-initializers",
                   "-DXGE_DLL", "-include", sdk / "xge_build_config.h", "tests/xge/test_3d_foundation_bench.c",
                   sdk / "xge.lib", "-lm", "-o", exe]
        subprocess.run([str(x) for x in command], cwd=ROOT, check=True)
        samples = []
        for _ in range(5):
            run = subprocess.run([str(exe)], cwd=ROOT, capture_output=True, text=True, check=True, timeout=120)
            samples.append(json.loads(run.stdout.strip().splitlines()[-1]))
        result[name] = {"engine_sha256": hashlib.sha256((out / "xge.dll").read_bytes()).hexdigest(), "samples": samples,
                        "median": {key: statistics.median(row[key] for row in samples)
                                   for key in ["update_ms", "query_ms", "destroy_ms"]}}
    assert {row["query_hits"] for part in result.values() for row in part["samples"]} == {6000}
    result["scope"] = "6000 synthetic static meshes; CPU update/query/destruction only; first index construction excluded; not full game/frame performance"
    (destination / "comparison.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(json.dumps(result, indent=2), flush=True)


if __name__ == "__main__":
    main()
