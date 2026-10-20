"""Consolidate measured foundation evidence and verify a relocated developer SDK."""
import csv
import hashlib
import json
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "build/foundation-20261018"
ART = ROOT / "artifacts/xge-foundation"


def read(path):
    return json.loads(path.read_text(encoding="utf-8"))


def main():
    ART.mkdir(parents=True, exist_ok=True)
    rows = list(csv.reader((BASE / "pressure.csv").open(encoding="utf-8")))
    memory = [[int(x) for x in row[1:]] for row in rows if row and row[0] == "memory"]
    steady = [row for row in memory if row[0] >= 300]
    pressure = {"cycles": 1200, "frames": sum(row[0] == "frame" for row in rows if row), "memory_samples": memory,
                "steady_crt_bytes_span": max(row[4] for row in steady) - min(row[4] for row in steady),
                "steady_crt_blocks_span": max(row[5] for row in steady) - min(row[5] for row in steady),
                "private_final_minus_first": memory[-1][1] - memory[0][1]}
    assert pressure["steady_crt_bytes_span"] <= 16384 and pressure["steady_crt_blocks_span"] <= 4
    assert pressure["private_final_minus_first"] <= 4 * 1024 * 1024
    assert "pressure: 1200 cycles" in (BASE / "pressure.log").read_text(encoding="utf-8", errors="replace")
    tests = {name: read(BASE / name / "foundation-tests.json")
             for name in ["3d", "full-dev", "bare", "no-animation", "no-async"]}
    assert all(row["exit"] == 0 and row["compile_exit"] == 0 for group in tests.values() for row in group)
    compatibility = read(BASE / "workshop-compatibility/verification.json")
    assert len(compatibility) == 4 and all(row["exit"] == 0 for row in compatibility)
    comparison = read(BASE / "benchmarks/comparison.json")
    sdk = BASE / "sdk"
    config = (sdk / "xge_build_config.h").read_text(encoding="utf-8")
    prefix = config.split("#define XGE_BUILD_ABI_FINGERPRINT", 1)[0]
    headers = ["include/xge.h", "include/xui.h", "include/xge_config.h", "include/xui_config.h", "extlibs/xrt/xrt.h"]
    compiler = subprocess.run([shutil.which("gcc"), "--version"], capture_output=True, text=True, check=True).stdout
    expected = hashlib.sha256((prefix + "".join((sdk / name).read_text(encoding="utf-8") for name in headers) + compiler + "0").encode()).hexdigest()[:16]
    assert f"UINT64_C(0x{expected})" in config
    relocated = BASE / "relocated-sdk/独立 SDK"
    shutil.copytree(BASE / "sdk", relocated, dirs_exist_ok=True)
    subprocess.run(["gcc", "-O2", "-Wall", "-Wextra", "-Werror", "-DXGE_DLL", "-DXUI_DLL", "-I.", "-Iinclude",
                    "-include", "xge_build_config.h", "sdk-check.c", "xge.lib", "-o", "sdk-check.exe"], cwd=relocated, check=True)
    result = subprocess.run([str(relocated / "sdk-check.exe")], cwd=relocated, capture_output=True, text=True, check=True)
    report = {
        "date": "2026-10-18", "status": "passed", "scope": "frozen generic C/XRT foundation SDK; no game-specific rules; not a claim about subsequent live-workspace changes",
        "sdk": str(BASE / "sdk"), "frozen_source_manifest": str(BASE / "source-manifest.json"),
        "engine_regression_groups": sum(map(len, tests.values())), "tests": tests,
        "abi": read(BASE / "abi-and-exports.json"), "workshop_binary_compatibility": compatibility,
        "pressure": pressure, "cpu_benchmark": comparison,
        "sdk_relocation": {"path": str(relocated), "check": json.loads(result.stdout), "header_fingerprint_recomputed": expected},
        "limitations": ["Initial/structurally invalidated BVH build is synchronous", "Render collection remains linear",
                        "Allocation, driver calls and final CPU destruction cannot be preempted",
                        "Eight selected direct light slots; no clustered-lighting implementation",
                        "No full-city/crowd/MMO or GLES/WebGL performance claim"],
        "concurrent_changes": "Model retirement APIs appeared in the shared workspace during this task; preserved, frozen and included in GPU tests",
    }
    names = ["include/xge.h", "src/xge/3d/xge3d_scene.c", "src/xge/3d/xge3d_spatial.c",
             "src/xge/3d/xge3d_spatial_index.inc", "src/xge/3d/xge3d_instance.inc", "src/xge/3d/xge3d_profile.inc",
             "src/xge/3d/xge3d_animation.c", "src/xge/3d/xge3d_skin.c"]
    frozen_manifest = read(BASE / "source-manifest.json")
    report["source_payload_hashes"] = {name: frozen_manifest[name] for name in names}
    report["live_workspace_hashes_at_report"] = {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in names}
    report["later_workspace_changes"] = [name for name in names if frozen_manifest[name] != report["live_workspace_hashes_at_report"][name]]
    assert hashlib.sha256((BASE / "sdk/include/xge.h").read_bytes()).hexdigest() == frozen_manifest["include/xge.h"]
    (ART / "verification.json").write_text(json.dumps(report, indent=2, ensure_ascii=False), encoding="utf-8")
    for name in ["abi-and-exports.json", "source-manifest.json", "pressure.csv", "pressure.log", "trimming-runtime.json"]:
        shutil.copy2(BASE / name, ART / name)
    shutil.copy2(BASE / "benchmarks/comparison.json", ART / "cpu-comparison.json")
    shutil.copy2(ART / "verification.json", BASE / "sdk/docs/verification.json")
    sdk = BASE / "sdk"
    manifest = {str(path.relative_to(sdk)).replace("\\", "/"): hashlib.sha256(path.read_bytes()).hexdigest()
                for path in sdk.rglob("*") if path.is_file() and path.name != "manifest.json"}
    (sdk / "manifest.json").write_text(json.dumps({"files": manifest, "check": json.loads(result.stdout)}, indent=2), encoding="utf-8")
    print(json.dumps({"engine_groups": report["engine_regression_groups"], "workshop_binaries": len(compatibility),
                      "crt_steady_bytes_span": pressure["steady_crt_bytes_span"],
                      "crt_steady_blocks_span": pressure["steady_crt_blocks_span"],
                      "before_median": comparison["before"]["median"], "after_median": comparison["after"]["median"],
                      "sdk_relocation": json.loads(result.stdout)}, indent=2))


if __name__ == "__main__":
    main()
