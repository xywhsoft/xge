"""Check every XUI switch using cached components and dependency-closed links."""
import argparse
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def run(args, **kwargs):
    return subprocess.run([str(a) for a in args], cwd=ROOT, check=True, text=True, **kwargs)


def truth(expression, disabled):
    names = re.findall(r"XUI_ENABLE_\w+", expression)
    return any(n not in disabled for n in names) if "||" in expression else all(n not in disabled for n in names)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--verify-existing", action="store_true", help="recheck already linked matrix artifacts")
    args = parser.parse_args()
    manifest = json.loads((ROOT / "tools/features.json").read_text())
    report = json.loads((ROOT / "build/full-dev/build-report.json").read_text())
    switches = sorted(f for f in report["features"] if f.startswith("XUI_ENABLE_"))
    out = ROOT / "artifacts/xge-3d/control-matrix"
    out.mkdir(parents=True, exist_ok=True)
    owners = {}
    for source, guard in manifest["xui_sources"].items():
        if guard.startswith("XUI_ENABLE_"):
            for name in re.findall(r"XUI_API\s+[^;\n]*?\b(xui\w+)\s*\(", (ROOT / source).read_text()):
                owners.setdefault(guard, []).append(name)
    radio_group = [n for n in owners["XUI_ENABLE_RADIO"] if n.startswith("xuiRadioGroup")]
    owners["XUI_ENABLE_RADIO_GROUP"] = radio_group
    owners["XUI_ENABLE_RADIO"] = [n for n in owners["XUI_ENABLE_RADIO"] if n not in radio_group]
    optional_proxy = {"XUI_ENABLE_POPUP", "XUI_ENABLE_LIST_VIEW", "XUI_ENABLE_INPUT", "XUI_ENABLE_TEXT_EDIT", "XUI_ENABLE_CODE_EDIT"}
    libs = ["-lm", "-lws2_32", "-liphlpapi", "-lgdi32", "-luser32", "-lshell32", "-lole32", "-loleaut32", "-luuid", "-limm32", "-lwinmm", "-lavrt"]
    summaries = []
    for switch in switches:
        disabled = {switch}
        while True:
            previous = set(disabled)
            for dependent, required in manifest["xui_requires"].items():
                if any(not truth(r, disabled) for r in required):
                    disabled.add(dependent)
            if previous == disabled:
                break
        flags = report["flags"] + ["-D" + f + "=0" for f in sorted(disabled)]
        case = out / switch
        case.mkdir(exist_ok=True)
        objects = []
        recompile = set()
        if disabled & optional_proxy:
            recompile.add("src/xui_proxy_xge.c")
        if "XUI_ENABLE_RADIO_GROUP" in disabled and "XUI_ENABLE_RADIO" not in disabled:
            recompile.add("src/xui_radio.c")
        for source in report["sources"]:
            guard = manifest["xui_sources"].get(source)
            if guard and guard != "XGE_ENABLE_XUI" and not truth(guard, disabled):
                continue
            obj = ROOT / "build/full-dev/obj" / (source.replace("/", "_") + ".o")
            if source in recompile:
                obj = case / (Path(source).stem + ".o")
                if not args.verify_existing:
                    run(["gcc", *flags, "-c", source, "-o", obj], capture_output=True)
            objects.append(obj)
        objects += [ROOT / "build/shared/obj/lib_harfbuzz_src_harfbuzz.cc.o", ROOT / "build/full-dev/obj/xge_res.o"]
        dll = case / "xge.dll"
        if args.verify_existing:
            assert dll.exists() and dll.stat().st_mtime_ns >= max(obj.stat().st_mtime_ns for obj in objects), (switch, "stale matrix DLL")
        else:
            run(["gcc", "-shared", "-o", dll, *objects, *libs], capture_output=True)
        exports_text = run(["objdump", "-p", dll], capture_output=True).stdout
        exports = set(re.findall(r"^\s*\[\s*\d+\].*\s+(xui\w+)\s*$", exports_text, re.M))
        assert "xuiWidgetRegisterType" in exports, (switch, "export table not parsed")
        removed = []
        for flag in disabled:
            for symbol in owners.get(flag, []):
                assert symbol not in exports, (switch, flag, symbol)
                removed.append(symbol)
        probe = '#include "xui.h"\n' + "".join(f"void *disabled_{i} = (void *)&{symbol};\n" for i, symbol in enumerate(owners.get(switch, [])[:1]))
        run(["gcc", *flags, "-fsyntax-only", "-x", "c", "-"], input='#include "xui.h"\n', capture_output=True)
        if owners.get(switch):
            compiled = subprocess.run(["gcc", *flags, "-fsyntax-only", "-x", "c", "-"], cwd=ROOT, text=True, input=probe, capture_output=True)
            assert compiled.returncode != 0, (switch, "disabled declaration still present")
        summaries.append({"switch": switch, "dependency_closure": sorted(disabled), "removed_exports": len(removed), "recompiled": sorted(recompile)})
        print(switch, "passed", len(disabled), "closed switches", len(removed), "removed exports", flush=True)
    (out / "summary.json").write_text(json.dumps(summaries, indent=2) + "\n")
    print(len(summaries), "control/service switches verified")


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as exc:
        print(exc.stderr or "Compiler failed", flush=True)
        raise SystemExit(exc.returncode)
