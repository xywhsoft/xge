"""Cross-checkout static reuse must validate this checkout's source bytes."""
from pathlib import Path
import tempfile
import unittest
from static import sha,reuse_dependencies

class StaticReuseTest(unittest.TestCase):
    def test_cross_checkout(self):
        with tempfile.TemporaryDirectory()as folder:
            base=Path(folder).resolve();old=base/'old';new=base/'new';old.mkdir();new.mkdir()
            source=old/'unit.c';source.write_bytes(b'old implementation')
            target=new/'unit.c';target.write_bytes(source.read_bytes())
            system=base/'system.h';system.write_bytes(b'system ABI')
            dependencies={str(source):sha(source),str(system):sha(system)}
            self.assertEqual(reuse_dependencies(dependencies,old,new),{str(target):sha(target),str(system):sha(system)})
            target.write_bytes(b'new implementation')
            self.assertIsNone(reuse_dependencies(dependencies,old,new))
            target.unlink()
            self.assertIsNone(reuse_dependencies(dependencies,old,new))
            self.assertEqual(reuse_dependencies(dependencies,old,old),dependencies)
            system.write_bytes(b'changed system ABI')
            self.assertIsNone(reuse_dependencies(dependencies,old,old))

if __name__=='__main__':unittest.main()
