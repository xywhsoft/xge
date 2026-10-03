#!/usr/bin/env python3
"""Small profile builder: compiler dependency files, cached objects, shared config."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
PROFILES = {"default": 0, "core": 1, "2d": 2, "3d": 3, "ui-min": 4, "full-dev": 5, "ide": 0}


def run(args, **kwargs):
    return subprocess.run([str(a) for a in args], check=True, cwd=ROOT, **kwargs)


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True).encode()).hexdigest()


def stamp(path):
    s = Path(path).stat()
    return [s.st_mtime_ns, s.st_size]


def dependencies(path):
    text = path.read_text().replace("\\\n", " ")
    text = re.split(r":\s", text, maxsplit=1)[1]
    return [re.sub(r"\\(.)", r"\1", p) for p in re.findall(r"(?:\\.|[^\s])+", text)]


def enabled(cc, flags, names):
    source = '#include "xui_config.h"\n'
    for name in names:
        source += f"#if {name}\nXGE_SELECTED_{name}\n#endif\n"
    output = run([cc, *flags, "-E", "-P", "-x", "c", "-"], input=source, text=True, capture_output=True)
    return set(re.findall(r"^XGE_SELECTED_(\w+)$", output.stdout, re.M))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("profile", choices=PROFILES, nargs="?", default="default")
    parser.add_argument("output", nargs="?")
    parser.add_argument("--define", action="append", default=[], help="shared NAME=0/1 override")
    parser.add_argument("--sources-only", action="store_true", help="print selected XUI sources without building")
    args = parser.parse_args()
    os.chdir(ROOT)
    started = time.perf_counter()
    windows = os.name == "nt"
    cc = shutil.which(os.environ.get("CC", "gcc"))
    if not cc:
        raise RuntimeError("C compiler not found")
    version = run([cc, "--version"], text=True, capture_output=True).stdout
    identity = [cc, stamp(cc), version]
    out = Path(args.output or ("build/ide" if args.profile == "ide" else "build" if args.profile == "default" else f"build/{args.profile}"))
    out.mkdir(parents=True, exist_ok=True)
    objects = out / "obj"
    objects.mkdir(exist_ok=True)
    flags = ["-O2", "-Wall", "-Wextra", "-Wno-unused-parameter", "-Wno-unused-function",
             "-Wno-cast-function-type", "-DXGE_DLL", "-DXGE_BUILD_DLL", "-DXUI_DLL",
             "-DXUI_BUILD_DLL", "-DBUILD_DLL", "-DXGE_DEBUGMODE=0", "-I.",
             f"-DXGE_BUILD_PROFILE={PROFILES[args.profile]}"]
    flags += ["-D" + d for d in args.define]
    if args.profile == "ide":
        flags += ["-DXGE_XRT_PROFILE_IDE"]
    if not windows:
        flags += ["-fPIC"]
    manifest = json.loads((ROOT / "tools/features.json").read_text())
    names = set(re.findall(r"(?:XGE|XUI|XGE3D)_ENABLE_\w+", (ROOT / "xge_config.h").read_text() + (ROOT / "xui_config.h").read_text()))
    features = enabled(cc, flags, names)
    if "XGE_ENABLE_TEXT" in features and not any(d.split("=")[0] == "XGE_ENABLE_HARFBUZZ" for d in args.define):
        flags += ["-DXGE_ENABLE_HARFBUZZ=1"]
        features = enabled(cc, flags, names)
    sources = ["xge.c"]
    # Evaluate compound source guards with the compiler, not a second macro interpreter.
    selections = list(manifest["xui_sources"].items())
    source_probe = '#include "xui_config.h"\n' + "".join(f"#if XGE_ENABLE_XUI && ({guard})\nXGE_SOURCE_{i}\n#endif\n" for i, (_, guard) in enumerate(selections))
    selected = run([cc, *flags, "-E", "-P", "-x", "c", "-"], input=source_probe, text=True, capture_output=True).stdout
    sources += [selections[int(i)][0] for i in re.findall(r"^XGE_SOURCE_(\d+)$", selected, re.M)]
    if args.sources_only:
        print(" ".join(sources[1:]))
        return
    selections = list(manifest.get("3d_sources", {}).items())
    source_probe = '#include "xge_config.h"\n' + "".join(f"#if {guard}\nXGE_SOURCE_{i}\n#endif\n" for i, (_, guard) in enumerate(selections))
    selected = run([cc, *flags, "-E", "-P", "-x", "c", "-"], input=source_probe, text=True, capture_output=True).stdout
    sources += [selections[int(i)][0] for i in re.findall(r"^XGE_SOURCE_(\d+)$", selected, re.M)]
    config = "#ifndef XGE_BUILD_CONFIG_H\n#define XGE_BUILD_CONFIG_H\n"
    config += f"#define XGE_BUILD_PROFILE {PROFILES[args.profile]}\n"
    config += "".join(f"#define {name} {int(name in features)}\n" for name in sorted(names))
    if args.profile == "ide":
        config += "#define XGE_XRT_PROFILE_IDE\n"
    config += "#endif\n"
    config_path = out / "xge_build_config.h"
    if not config_path.exists() or config_path.read_text() != config:
        config_path.write_text(config)
    libs = ["-lm"]
    if windows:
        libs += ["-lws2_32", "-liphlpapi", "-lgdi32", "-luser32", "-lshell32", "-lole32", "-loleaut32", "-luuid", "-limm32", "-lwinmm", "-lavrt"]
    else:
        libs += ["-lX11", "-lXi", "-lXcursor", "-lGL", "-ldl", "-lpthread"]
    if windows and "XUI_ENABLE_WEBVIEW" in features and os.environ.get("XUI_ENABLE_WEBVIEW2") == "1":
        sdk = Path(os.environ.get("WEBVIEW2_SDK_DIR", ""))
        include = sdk / "build/native/include"
        loader = sdk / "build/native/x64/WebView2Loader.dll.lib"
        if not (include / "WebView2.h").is_file() or not loader.is_file():
            raise RuntimeError("Set WEBVIEW2_SDK_DIR to an extracted WebView2 SDK")
        flags += ["-DXUI_ENABLE_WEBVIEW2", "-isystem", str(include)]
        if os.environ.get("XUI_WEBVIEW_TEST_EXPORTS") == "1":
            flags += ["-DXUI_WEBVIEW_TEST_EXPORTS"]
        libs += [str(loader)]
    compiled = []
    reused = []
    link_objects = []

    def compile_object(source, compiler, options, shared=False):
        object_dir = ROOT / "build/shared/obj" if shared else objects
        object_dir.mkdir(parents=True, exist_ok=True)
        obj = object_dir / (source.replace("/", "_").replace("\\", "_") + ".o")
        dep = obj.with_suffix(".d")
        record = obj.with_suffix(".json")
        command = [compiler, *options, "-MD", "-MF", str(dep), "-MT", str(obj), "-c", source, "-o", str(obj)]
        key = digest([identity, compiler, stamp(compiler), command])
        try:
            cache = json.loads(record.read_text())
            valid = obj.is_file() and cache["key"] == key and all(stamp(p) == s for p, s in cache["deps"].items())
        except (OSError, KeyError, ValueError):
            valid = False
        if valid:
            reused.append(source)
        else:
            print(f"[compile] {source}", flush=True)
            # A failed compile cannot leave an object that is accepted on the next run.
            if record.exists():
                record.unlink()
            run(command)
            record.write_text(json.dumps({"key": key, "deps": {p: stamp(p) for p in dependencies(dep)}}))
            compiled.append(source)
        link_objects.append(obj)

    for source in sources:
        compile_object(source, cc, flags)
    if "XGE_ENABLE_HARFBUZZ" in features:
        cxx = shutil.which(os.environ.get("CXX", "g++"))
        if not cxx:
            raise RuntimeError("The existing optional HarfBuzz dependency requires g++")
        hb_flags = ["-std=c++17", "-O2", "-fno-exceptions", "-fno-rtti", "-ffunction-sections", "-fdata-sections", "-DNDEBUG", "-DHB_NO_BUFFER_SERIALIZE", "-DHB_NO_BUFFER_VERIFY", "-DHB_NO_DRAW", "-DHB_NO_OT_FETCH", "-DHB_NO_MMAP", "-DHB_NO_COLOR"]
        compile_object("lib/harfbuzz/src/harfbuzz.cc", cxx, hb_flags + ([] if windows else ["-fPIC"]), shared=True)
    if windows:
        resource = objects / "xge_res.o"
        resource_record = resource.with_suffix(".json")
        windres = shutil.which("windres")
        if not windres:
            raise RuntimeError("windres not found")
        resource_key = digest([windres, stamp(windres), stamp("xge.rc"), stamp("res/xx_dll.ico")])
        if not resource.exists() or not resource_record.exists() or resource_record.read_text() != resource_key:
            run([windres, "-O", "coff", "-i", "xge.rc", "-o", resource])
            resource_record.write_text(resource_key)
        link_objects.append(resource)
    library = out / ("xge.dll" if windows else "libxge.so")
    unstripped = out / ("xge.unstripped.dll" if windows else "libxge.unstripped.so")
    import_library = out / "xge.lib"
    link_args = [cc, "-shared", "-o", str(library), *map(str, link_objects), *libs]
    if windows:
        link_args += [f"-Wl,--out-implib,{import_library}"]
    else:
        link_args += ["-Wl,--no-undefined"]
    link_key = digest([identity, link_args, {str(p): stamp(p) for p in link_objects}, {p: stamp(p) for p in libs if Path(p).is_file()}])
    link_record = out / "link.key"
    linked = not (library.is_file() and unstripped.is_file() and (not windows or import_library.is_file()) and link_record.is_file() and link_record.read_text() == link_key)
    if linked:
        print(f"[link] {library}", flush=True)
        if link_record.exists():
            link_record.unlink()
        run(link_args)
        shutil.copy2(library, unstripped)
        strip = shutil.which("strip")
        if not strip:
            raise RuntimeError("strip not found")
        run([strip, "--strip-unneeded", library])
        link_record.write_text(link_key)
    report = {"profile": args.profile, "defines": args.define, "compiler": identity, "flags": flags,
              "features": sorted(features), "sources": sources, "compiled": compiled, "reused": reused,
              "linked": linked, "seconds": time.perf_counter() - started,
              "runtime_bytes": library.stat().st_size, "unstripped_bytes": unstripped.stat().st_size}
    (out / "build-report.json").write_text(json.dumps(report, indent=2) + "\n")
    print(f"[XGE {args.profile}] {len(compiled)} compiled, {len(reused)} reused, {report['seconds']:.2f}s, {report['runtime_bytes']} runtime bytes", flush=True)


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as exc:
        if exc.stderr:
            print(exc.stderr, file=sys.stderr)
        sys.exit(exc.returncode or 1)
    except (OSError, RuntimeError, ValueError) as exc:
        print(f"[ERROR] {exc}", file=sys.stderr)
        sys.exit(1)
