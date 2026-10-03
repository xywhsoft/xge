"""Verify actual profile DLLs, public declarations and incremental builds."""
import argparse
import ctypes
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from build_profile import PROFILES


def run(args, **kwargs):
    return subprocess.run(args, cwd=ROOT, check=True, text=True, **kwargs)


def check(profile):
    out = ROOT / "build" if profile == "default" else ROOT / "build" / profile
    run([sys.executable, "tools/build_profile.py", profile])
    report = json.loads((out / "build-report.json").read_text())
    features = set(report["features"])
    library = ctypes.CDLL(str(out / "xge.dll"))
    tests = {
        "xgeDraw": "XGE_ENABLE_2D", "xgeFontLoad": "XGE_ENABLE_TEXT",
        "xgeAudioInit": "XGE_ENABLE_AUDIO", "xgeShapeExCreate": "XGE_ENABLE_SHAPE_EX",
        "xgeSvgCacheClear": "XGE_ENABLE_SVG", "xgeEmojiPackLoadBuiltin": "XGE_ENABLE_EMOJI",
        "xgeParticleWorldCreate": "XGE_ENABLE_PARTICLES", "xuiButtonCreate": "XUI_ENABLE_BUTTON",
        "xuiCodeEditCreate": "XUI_ENABLE_CODE_EDIT", "xuiDocumentViewCreate": "XUI_ENABLE_DOCUMENT_VIEW",
        "xuiWebViewCreate": "XUI_ENABLE_WEBVIEW",
        "xge3dSceneCreate": "XGE_ENABLE_3D", "xge3dRender": "XGE_ENABLE_3D",
        "xge3dEnvironmentCreate": "XGE_ENABLE_3D", "xge3dSceneRaycast": "XGE_ENABLE_3D",
        "xge3dNodeSetLods": "XGE_ENABLE_3D", "xge3dNodeGetBounds": "XGE_ENABLE_3D",
        "xge3dSceneSetOrigin": "XGE_ENABLE_3D", "xge3dSceneQueryAabb": "XGE_ENABLE_3D",
        "xge3dTerrainCreate": "XGE3D_ENABLE_TERRAIN", "xge3dTerrainSample": "XGE3D_ENABLE_TERRAIN",
        "xge3dLoaderCreate": "XGE3D_ENABLE_ASYNC", "xge3dLoaderPump": "XGE3D_ENABLE_ASYNC",
        "xge3dModelLoad": "XGE3D_ENABLE_MODEL", "xge3dNodeSetLight": "XGE3D_ENABLE_LIGHTING",
        "xge3dShadowDefault": "XGE3D_ENABLE_SHADOW", "xge3dEnvironmentSetIBL": "XGE3D_ENABLE_IBL",
        "xge3dClipLoad": "XGE3D_ENABLE_ANIMATION", "xge3dAnimatorCreate": "XGE3D_ENABLE_ANIMATION",
        "xge3dAnimatorSetRootMotion": "XGE3D_ENABLE_ANIMATION", "xge3dNodeSkinMatrices": "XGE3D_ENABLE_ANIMATION",
    }
    for symbol, macro in tests.items():
        present = bool(getattr(library, symbol, None))
        assert present == (macro in features), (profile, symbol, present)
    dll_text = run(["objdump", "-p", str(out / "xge.dll")], capture_output=True).stdout
    imported = re.findall(r"DLL Name: (.+)", dll_text)
    assert not any("harfbuzz" in name.lower() or "stdc++" in name.lower() for name in imported), imported
    if "XGE_ENABLE_XUI" not in features:
        assert not re.search(r"^\s*\[\s*\d+\].*\s+xui\w+\s*$", dll_text, re.M), profile
        manifest = json.loads((ROOT / "tools/features.json").read_text())
        assert not any(s in manifest["xui_sources"] for s in report["sources"]), report["sources"]
        assert not any("harfbuzz" in s for s in report["compiled"] + report["reused"])
    args = ["gcc", "-O2", "-Wall", "-Wextra", "-Werror", "-DXGE_DLL", "-DXGE_DEBUGMODE=0", "-I.",
            "-include", str(out / "xge_build_config.h")]
    exe = out / "test_feature_profile.exe"
    run([*args, "test/test_feature_profile.c", str(out / "xge.lib"), "-o", str(exe)])
    run([str(exe)])
    # A declaration must actually disappear, including a supplied macro value of zero.
    forbidden = [symbol for symbol, macro in tests.items() if macro not in features]
    for symbol in forbidden:
        probe = '#include "xge.h"\n#include "xui.h"\nvoid *disabled_entry = (void *)&' + symbol + ';\n'
        result = subprocess.run([*args, "-fsyntax-only", "-x", "c", "-"], input=probe, cwd=ROOT, text=True, capture_output=True)
        assert result.returncode != 0, (profile, "declaration still present", symbol)
    run([sys.executable, "tools/build_profile.py", profile])
    hot = json.loads((out / "build-report.json").read_text())
    assert not hot["compiled"] and not hot["linked"], hot
    summary = {"profile": profile, "runtime_bytes": report["runtime_bytes"],
               "unstripped_bytes": report["unstripped_bytes"], "hot_seconds": hot["seconds"],
               "imports": imported, "checks": tests, "sources": report["sources"]}
    (ROOT / f"artifacts/xge-3d/p0-{profile}-verification.json").write_text(json.dumps(summary, indent=2))
    print(f"{profile}: exports, removed declarations, lifecycle/resources, hot cache passed")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("profiles", nargs="*", default=["core", "2d", "3d", "ui-min", "full-dev"])
    args = parser.parse_args()
    for profile in args.profiles:
        check(profile)
