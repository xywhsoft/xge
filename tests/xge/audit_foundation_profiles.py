"""Check SDK exports, optional modules and legacy public structure layouts."""
import argparse
import json
import re
import subprocess
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def call(command, **kwargs):
    return subprocess.run([str(x) for x in command], cwd=ROOT, check=True, **kwargs)


def exports(dll):
    text = call(["objdump", "-p", dll], capture_output=True, text=True).stdout
    return set(re.findall(r"^\s*\[\s*\d+\].*\s+((?:xge|xui)\w+)\s*$", text, re.M))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("root", type=Path)
    p.add_argument("baseline", type=Path)
    args = p.parse_args()
    base, baseline = args.root.resolve(), args.baseline.resolve()
    frozen = base / "source"
    results = []
    for name in ["3d", "full-dev", "bare", "no-animation", "no-async", "core"]:
        sdk = base / name
        flags = ["gcc", "-I" + str(frozen / "include"), "-I" + str(frozen), "@config/includes.rsp",
                 "-DXGE_DLL", "-include", sdk / "xge_build_config.h"]
        text = call([*flags, "-E", "-P", "-x", "c", "-"], input='#include "xge.h"\n', capture_output=True, text=True).stdout
        declarations = set(re.findall(r"\b(xge3d\w+)\s*\(", text))
        exported = exports(sdk / "xge.dll")
        three_d = {name for name in exported if name.startswith("xge3d")}
        assert declarations == three_d, (name, declarations - three_d, three_d - declarations)
        if name == "full-dev":
            old = exports(baseline / "xge.dll")
            assert old <= exported, sorted(old - exported)
            results.append({"legacy_exports_preserved": len(old), "added_exports": len(exported - old)})
        results.append({"profile": name, "matching_3d_exports": len(three_d)})
    old_header = base / "baseline-xge.h"
    with zipfile.ZipFile(base / "before.zip") as archive:
        old_header.write_bytes(archive.read("include/xge.h"))
    types = ["xge3d_node_t", "xge3d_transform_t", "xge3d_camera_t", "xge3d_vertex_t", "xge3d_mesh_desc_t",
             "xge3d_material_desc_t", "xge3d_render_desc_t", "xge3d_render_stats_t", "xge3d_model_info_t",
             "xge3d_animation_layer_t", "xge3d_light_desc_t", "xge3d_shadow_settings_t", "xge3d_terrain_desc_t",
             "xge3d_upload_budget_t", "xge3d_upload_stats_t", "xge3d_request_info_t", "xge3d_hit_t"]
    layouts = []
    for name, header in [("before", old_header.as_posix()), ("after", "xge.h")]:
        source = base / f"layout-{name}.c"
        source.write_text(f'#include "{header}"\n#include <stdio.h>\nint main(void){{\n' +
                          "".join(f'printf("%zu\\n",sizeof({t}));\n' for t in types) + "return 0;}\n")
        exe = base / "full-dev" / f"layout-{name}.exe"
        call(["gcc", "-I" + str(frozen / "include"), "-I" + str(frozen), "@config/includes.rsp", "-DXGE_DLL",
              "-include", base / "full-dev/xge_build_config.h", source, "-o", exe])
        layouts.append(call([exe], capture_output=True, text=True).stdout.splitlines())
    assert layouts[0] == layouts[1], dict(zip(types, zip(*layouts)))
    results.append({"unchanged_legacy_structures": dict(zip(types, map(int, layouts[1])))})
    (base / "abi-and-exports.json").write_text(json.dumps(results, indent=2), encoding="utf-8")
    print(json.dumps(results, indent=2))


if __name__ == "__main__":
    main()
