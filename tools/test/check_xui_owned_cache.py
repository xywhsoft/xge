#!/usr/bin/env python3
"""CPU-only checked cache tests over one matched native object graph.

No engine/window/device initialization, downloads, network or security probes.
The actual native private helpers link directly from production objects; this
runner adds no exports, wrapper implementations or ABI substitutions.
"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import time


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("source", "engine", "runtime", "out"):
        parser.add_argument("--" + name, type=Path, required=True)
    parser.add_argument("--object-overlay", type=Path)
    parser.add_argument("--fixture-root", type=Path)
    parser.add_argument("--case", action="append", choices=("layout", "layout_draft", "chunk_edit", "code_document", "renderer", "renderer_busy", "shape", "simple_layout", "requested_callbacks", "textedit_cache", "textedit_regression", "message_cache", "message_regression", "message_display", "codeedit_cache", "codeedit_regression", "webview_callbacks", "web_provider", "image_resource", "image_regression", "html_interaction", "hold_outcome", "hold_renderer", "doc_ui_callbacks", "doc_editor_regression", "doc_model_worker", "document_model", "document_model_fault", "doc_model_ui"))
    args = parser.parse_args()
    source, engine, runtime, out = [value.resolve() for value in (args.source, args.engine, args.runtime, args.out)]
    out.mkdir(parents=True, exist_ok=False)
    metadata = engine / "build-report.json"
    build = json.loads(metadata.read_text(encoding="utf-8"))
    windows = os.name == "nt"
    objects = [engine / "obj" / (name.replace("/", "_") + ".o") for name in build["sources"]]
    if args.object_overlay:
        objects = [args.object_overlay.resolve() / path.name if (args.object_overlay.resolve() / path.name).is_file() else path for path in objects]
    objects += sorted((source / "build/shared/obj").glob("*.o"))
    if windows:
        objects += sorted((engine / "obj").glob("*_res.o"))
    cases = args.case or ["layout", "layout_draft", "chunk_edit", "code_document"]
    fixture_root = args.fixture_root.resolve() if args.fixture_root else source / "tests/xui"
    tests = [fixture_root / ("xui_owned_" + case + "_test.c") for case in cases]
    pin = [metadata, runtime, *objects, *tests]
    pin += sorted((source / "include").rglob("*.h"))
    pin += sorted((source / "src/xui").rglob("*.h"))
    pin += sorted((source / "src/xge/text").glob("*.h"))
    pin += sorted((source / "tests/xui").glob("xui_owned_*_test.c"))
    pin += sorted(fixture_root.glob("*.c"))
    pin += sorted(fixture_root.glob("*.h"))
    pin += sorted((source / "tests/xui").glob("*.h"))
    pin += [source / "res/xui/assets/msgbox_info.png", source / "extlibs/xrt/xrt.h"]
    fixture_bundle = fixture_root.parent.parent
    pin += [path for path in (fixture_bundle / "src").rglob("*") if path.is_file() and path.suffix in (".c", ".h", ".inl")]
    pin += [Path(__file__).resolve(), fixture_bundle / "manifest.json"]
    pin += [source / "tests/xui/xui_test_proxy.c", source / "tests/xui/xui_test_proxy.h"]
    pin += sorted((engine / "host-xrt").glob("*"))
    inputs = {str(path): digest(path) for path in pin if path.is_file()}
    report = {"complete": False, "passed": False, "scope": "Offline CPU-only actual native checked cache fixtures, matched complete production object graph; no full UI/GPU qualification.", "inputs": inputs, "rows": []}

    def save():
        (out / "results.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    def run(name, command, env=None):
        started = time.monotonic()
        try:
            result = subprocess.run(list(map(str, command)), cwd=source, env=env, capture_output=True, timeout=90)
            code, output = result.returncode, result.stdout + result.stderr
        except subprocess.TimeoutExpired as error:
            code, output = 124, (error.stdout or b"") + (error.stderr or b"")
        log = out / (name + ".log")
        log.write_bytes(output)
        report["rows"].append({"name": name, "command": list(map(str, command)), "exitCode": code, "seconds": round(time.monotonic() - started, 3), "logSha256": digest(log)})
        save()
        print(name, code, flush=True)
        return code == 0

    flags = ["-std=c11", "-Wall", "-Wextra", "-Werror", "-Wno-unused-function", "-Wno-unused-parameter", "-Wno-cast-function-type", "-DXGE_XRT_HOST=1", "-include", engine / "xge_build_config.h"]
    flags += ["-I" + str(path) for path in (source, source / "include", source / "tests/xui", source / "tests", source / "src/xui/core", source / "src/xui/accessibility", source / "src/xui/code", source / "src/xui/document", source / "src/xge/text", source / "src/xge/core", source / "extlibs/xrt", engine / "host-xrt")]
    libraries = ["-lm"] + (["-lws2_32", "-liphlpapi", "-lgdi32", "-luser32", "-lshell32", "-lole32", "-loleaut32", "-luuid", "-limm32", "-lwinmm", "-lavrt"] if windows else ["-lX11", "-lXi", "-lXcursor", "-lGL", "-ldl", "-lpthread", "-Wl,-rpath," + str(runtime.parent)])
    env = dict(os.environ)
    if windows:
        shutil.copy2(runtime, out / runtime.name)
    else:
        env["LD_LIBRARY_PATH"] = str(runtime.parent) + ":" + env.get("LD_LIBRARY_PATH", "")
    save()
    for case, test in zip(cases, tests):
        # Ask the actual preprocessor which production C TUs this fixture
        # directly includes. Several old cache fixtures intentionally call
        # private consumers; linking their normal object a second time is a
        # harness error. Verify the included bytes against the matched source,
        # exclude exactly those objects, and retain an explicit record.
        if not run(case+"-dependencies",[build["compiler"][0],*flags,"-MM","-MT","owned_fixture",test]):
            continue
        dependency_text=(out/(case+"-dependencies.log")).read_text(encoding="utf-8").replace("\\\n"," ")
        dependency_text=re.split(r":\s",dependency_text,maxsplit=1)[1]
        dependencies=[re.sub(r"\\(.)",r"\1",value) for value in re.findall(r"(?:\\.|[^\s])+",dependency_text)]
        included=set();included_rows=[]
        for value in dependencies:
            dependency=Path(value);normalized=dependency.as_posix().replace("\\","/")
            for name in build["sources"]:
                if normalized==name or normalized.endswith("/"+name):
                    actual=dependency if dependency.is_absolute() else source/dependency
                    assert actual.is_file() and digest(actual)==digest(source/name),(case,"unmatched included production TU",actual)
                    included.add(name.replace("/","_")+".o")
                    included_rows.append({"source":name,"includedPath":str(actual),"sha256":digest(actual)})
        report.setdefault("directlyIncludedProductionTUs",{})[case]=included_rows
        case_objects=[path for path in objects if path.name not in included]
        if case == "doc_editor_regression":
            # The pre-existing CPU fixture saves local files in this directory.
            (source / "build/document").mkdir(parents=True, exist_ok=True)
        for opt in ("O0", "O2"):
            executable = out / (case + "-" + opt + (".exe" if windows else ""))
            command = [build["compiler"][0], *flags, "-" + opt, test, source / "tests/xui/xui_test_proxy.c", *case_objects, runtime, *libraries, "-o", executable]
            if case=="document_model_fault":
                command.insert(1,"-Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=free")
            if run(case + "-" + opt + "-build", command):
                run(case + "-" + opt + "-run", [executable], env)
    report["inputsUnchanged"] = all(Path(name).is_file() and digest(Path(name)) == value for name, value in inputs.items())
    report["complete"] = True
    report["passed"] = report["inputsUnchanged"] and all(row["exitCode"] == 0 for row in report["rows"])
    save()
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
