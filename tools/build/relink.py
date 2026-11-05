"""Strict reuse of one explicitly qualified build; never compiles or edits it."""
import hashlib
import json
from pathlib import Path
import re


def image_names(api_major, host_xrt, debug, windows):
    stem = "xge3_host" if api_major == 3 and host_xrt else "xge"
    if debug:
        stem += "dbg"
    prefix, suffix = ("", ".dll") if windows else ("lib", ".so")
    return {"runtime_image": prefix + stem + suffix,
            "unstripped_image": prefix + stem + ".unstripped" + suffix,
            "import_library": stem + ".lib" if windows else None,
            "runtime_soname": None if windows else prefix + stem + suffix}


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def check_locations(root, out, base, source):
    targets = [Path(p).resolve() for p in (root, out)]
    qualified = [Path(p).resolve() for p in (base, source)]
    if any(target == old or old in target.parents for target in targets for old in qualified):
        raise RuntimeError("Relink source/output must be outside the qualified source and output trees")


class QualifiedRelink:
    def __init__(self, root, out, base, source, identity, flags, profile, defines,
                 api_major, features, debug, digest, stamp):
        self.root, self.out, self.base, self.source = [Path(p).resolve() for p in (root, out, base, source)]
        self.identity, self.digest, self.stamp = identity, digest, stamp
        check_locations(root, out, base, source)
        report = self.base / ("build-report-debug.json" if debug else "build-report.json")
        self.report_path, self.report_sha = report, sha(report)
        previous = json.loads(report.read_text())
        self.report_data = previous
        expected = {"api_major": api_major, "public_abi_schema": 2, "profile": profile,
                    "defines": defines, "compiler": identity, "xrt_mode": "host",
                    "features": sorted(features)}
        for name, value in expected.items():
            if previous.get(name) != value:
                raise RuntimeError(f"Qualified relink configuration differs: {name}")
        if not previous.get("linked"):
            raise RuntimeError("Qualified relink requires a successful original link")
        self.old_host, self.new_host = self.base / "host-xrt", self.out / "host-xrt"
        if self.old_flags(flags) != previous["flags"]:
            raise RuntimeError("Qualified relink compile flags differ")
        if (self.base / "xge_build_config.h").read_bytes() != (self.out / "xge_build_config.h").read_bytes():
            raise RuntimeError("Qualified relink ABI/configuration differs")
        self.expected = set(previous["compiled"]) | set(previous["reused"])
        self.seen, self.objects, self.inputs = set(), [], {}
        self.debug = debug

    def old_flags(self, flags):
        return [value.replace(self.new_host.as_posix(), self.old_host.as_posix()) for value in flags]

    def dependency(self, value, recorded_stamp=None):
        path = Path(value)
        original = path if path.is_absolute() else self.source / path
        if recorded_stamp is not None and self.stamp(original) != recorded_stamp:
            raise RuntimeError(f"Qualified dependency changed since its build: {original}")
        current = self.root / path if not path.is_absolute() else path
        if path.is_absolute():
            for old, new in ((self.base, self.out), (self.source, self.root)):
                try:
                    current = new / path.relative_to(old)
                    break
                except ValueError:
                    pass
        key = str(original) + "\0" + str(current)
        if key not in self.inputs:
            before, after = sha(original), sha(current)
            if before != after:
                raise RuntimeError(f"Qualified relink dependency content differs: {value}")
            self.inputs[key] = {"qualified": str(original), "current": str(current), "sha256": before}

    def object(self, source, compiler, options, shared):
        if source not in self.expected or source in self.seen:
            raise RuntimeError(f"Unexpected/duplicate qualified object: {source}")
        directory = self.source / "build/shared/obj" if shared else self.base / ("obj-debug" if self.debug else "obj")
        obj = directory / (source.replace("/", "_").replace("\\", "_") + ".o")
        dep, record = obj.with_suffix(".d"), obj.with_suffix(".json")
        cache = json.loads(record.read_text())
        text = re.split(r":\s", dep.read_text().replace("\\\n", " "), maxsplit=1)[1]
        declared = {re.sub(r"\\(.)", r"\1", name) for name in re.findall(r"(?:\\.|[^\s])+", text)}
        if declared != set(cache["deps"]):
            raise RuntimeError(f"Qualified cache/dependency inventory differs: {source}")
        command = [compiler, *self.old_flags(options), "-MD", "-MF", str(dep), "-MT", str(obj), "-c", source, "-o", str(obj)]
        key = self.digest([self.identity, compiler, self.stamp(compiler), command])
        if cache["key"] != key:
            raise RuntimeError(f"Qualified object compiler/command differs: {source}")
        if source not in cache["deps"]:
            raise RuntimeError(f"Qualified object has no source dependency: {source}")
        for name, stamp in cache["deps"].items():
            self.dependency(name, stamp)
        self.seen.add(source)
        self.objects.append({"source": source, "object": str(obj), "sha256": sha(obj),
                             "dependency_file": str(dep), "dependency_sha256": sha(dep),
                             "cache_record": str(record), "cache_sha256": sha(record)})
        return obj

    def resource(self, windres):
        obj = self.base / ("obj-debug" if self.debug else "obj") / ("xgedbg_res.o" if self.debug else "xge_res.o")
        key = self.digest([windres, self.stamp(windres), self.stamp(self.source / "platform/windows/xge.rc"),
                           self.stamp(self.source / "res/xge/icons/xx_dll.ico")])
        if obj.with_suffix(".json").read_text() != key:
            raise RuntimeError("Qualified Windows resource compiler/inputs differ")
        self.dependency("platform/windows/xge.rc")
        self.dependency("res/xge/icons/xx_dll.ico")
        self.objects.append({"source": "Windows resource", "object": str(obj), "sha256": sha(obj),
                             "cache_record": str(obj.with_suffix(".json")), "cache_sha256": sha(obj.with_suffix(".json"))})
        return obj

    def finish(self):
        if self.seen != self.expected:
            raise RuntimeError("Qualified relink source inventory differs")
        if sha(self.report_path) != self.report_sha:
            raise RuntimeError("Qualified build report changed during relink")
        for row in self.inputs.values():
            if sha(row["qualified"]) != row["sha256"] or sha(row["current"]) != row["sha256"]:
                raise RuntimeError("Qualified relink dependency changed during link")
        for row in self.objects:
            if sha(row["object"]) != row["sha256"]:
                raise RuntimeError("Qualified object changed during link")
            for name in ("dependency", "cache"):
                if name + "_sha256" in row and sha(row[name + ("_file" if name == "dependency" else "_record")]) != row[name + "_sha256"]:
                    raise RuntimeError("Qualified dependency/cache record changed during link")
        return {"report": str(self.report_path), "report_sha256": self.report_sha,
                "source_root": str(self.source), "objects": self.objects,
                "dependencies": list(self.inputs.values()), "inputs_unchanged": True,
                "scope": "Exact qualified objects and dependencies; no fallback compilation or modification of the original build."}
