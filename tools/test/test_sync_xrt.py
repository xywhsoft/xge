"""Dependency closure of the real XRT snapshot consumer (no generation mocks)."""
from pathlib import Path
import importlib.util
import unittest
import tempfile
import subprocess
import io
import zipfile

spec = importlib.util.spec_from_file_location("sync_xrt", Path(__file__).parents[1] / "package/sync_xrt.py")
sync = importlib.util.module_from_spec(spec)
spec.loader.exec_module(sync)


class ExtensionClosure(unittest.TestCase):
    def test_commit_archive_ignores_user_checkout_eol(self):
        with tempfile.TemporaryDirectory() as name:
            root = Path(name)
            def git(*args):
                return subprocess.check_output(["git", *args], cwd=root, stderr=subprocess.STDOUT)
            git("init", "--quiet")
            git("config", "core.autocrlf", "false")
            (root / "source.c").write_bytes(b"first\nsecond\n")
            git("add", "source.c")
            git("-c", "user.name=Snapshot test", "-c", "user.email=snapshot@example.invalid",
                "-c", "commit.gpgsign=false", "commit", "--quiet", "-m", "fixture")
            for setting in ("true", "false", "input"):
                git("config", "core.autocrlf", setting)
                with zipfile.ZipFile(io.BytesIO(sync.archive_snapshot(root, "HEAD", ["source.c"]))) as archive:
                    self.assertEqual(archive.read("source.c"), b"first\nsecond\n")

    def test_source_fingerprint_order_is_platform_independent(self):
        with tempfile.TemporaryDirectory() as name:
            root = Path(name)
            for relative in ("a.c", "B.c", "dir/Z.c", "dir/b.c"):
                path = root / relative
                path.parent.mkdir(exist_ok=True)
                path.write_bytes(b"source\n")
            self.assertEqual([p.relative_to(root).as_posix() for p in sync.snapshot_files(root)],
                             ["B.c", "a.c", "dir/Z.c", "dir/b.c"])

    def closure(self, records, requested):
        calls = []
        def read(path):
            calls.append(path)
            return records[path.split("/")[1]]
        result = sync.extension_closure(read, requested)
        return result, calls

    def test_actual_family_and_new_transitive_http_dependency(self):
        records = {"xllm": {}, "xllm-session": {"dependency_manifests": ["extlibs/xllm/config/modules.json"]},
            "xhttp-client": {}, "xwork": {"dependency_manifests": [
                "extlibs/xllm-session/config/modules.json", "extlibs/xhttp-client/config/modules.json"]}}
        order, calls = self.closure(records, sync.EXTENSIONS)
        self.assertEqual(order, ("xllm", "xllm-session", "xhttp-client", "xwork"))
        self.assertEqual(len(calls), 4)

    def test_diamond_and_duplicate_roots_read_each_manifest_once(self):
        records = {"a": {"dependency_manifests": ["extlibs/b/config/modules.json", "extlibs/c/config/modules.json"]},
            "b": {"dependency_manifests": ["extlibs/d/config/modules.json"]},
            "c": {"dependency_manifests": ["extlibs/d/config/modules.json"]}, "d": {}}
        order, calls = self.closure(records, ("a", "d", "a"))
        self.assertEqual(order, ("d", "b", "c", "a"))
        self.assertEqual(len(calls), 4)

    def test_cycle_rejected(self):
        with self.assertRaisesRegex(ValueError, "cycle"):
            self.closure({"a": {"dependency_manifests": ["extlibs/b/config/modules.json"]},
                "b": {"dependency_manifests": ["extlibs/a/config/modules.json"]}}, ("a",))

    def test_missing_dependency_is_not_silently_omitted(self):
        with self.assertRaises(KeyError):
            self.closure({"a": {"dependency_manifests": ["extlibs/missing/config/modules.json"]}}, ("a",))

    def test_invalid_dependency_shapes(self):
        for value in (None, {}, "extlibs/a/config/modules.json", [None], ["config/modules.json"], ["../a/config/modules.json"]):
            with self.subTest(value=value), self.assertRaises(ValueError):
                self.closure({"a": {"dependency_manifests": value}}, ("a",))


if __name__ == "__main__":
    unittest.main()
