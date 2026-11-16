"""Dependency closure of the real XRT snapshot consumer (no generation mocks)."""
from pathlib import Path
import importlib.util
import unittest

spec = importlib.util.spec_from_file_location("sync_xrt", Path(__file__).parents[1] / "package/sync_xrt.py")
sync = importlib.util.module_from_spec(spec)
spec.loader.exec_module(sync)


class ExtensionClosure(unittest.TestCase):
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
