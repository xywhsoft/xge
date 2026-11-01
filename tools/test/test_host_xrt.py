"""Declaration import generator admission and closed-symbol tests."""
from pathlib import Path
import importlib.util
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('host_xrt',Path(__file__).resolve().parents[1]/'build/host_xrt.py')
host_xrt = importlib.util.module_from_spec(spec); spec.loader.exec_module(host_xrt)
BOUNDARY = '#if defined(XRT_IMPLEMENTATION) && !defined(XRT_IMPLEMENTATION_ONCE)\n'


class HostImports(unittest.TestCase):
    def generate(self,text):
        folder = tempfile.TemporaryDirectory(); self.addCleanup(folder.cleanup)
        root = Path(folder.name); header = root/'xrt.h'; header.write_bytes(text.encode())
        metadata = host_xrt.generate(header,root/'generated')
        return metadata,(root/'generated/xge_host_xrt_imports.h').read_text()

    def test_exact_pointer_and_callback_declarators(self):
        metadata,text = self.generate('''#define XRT_API
/* XRT_API void invented(void); */
XRT_API const void* xrtProbe(const int* input, void (*callback)(void*, int));
XRT_API int __xrtProbe(double deadline);
'''+BOUNDARY+'XRT_API void xrtImplementationOnly(void) {}\n#endif\n')
        self.assertEqual(metadata['names'],['__xrtProbe','xrtProbe'])
        self.assertIn('typedef const void* XgeHostXrtType_xrtProbe(const int* input, void (*callback)(void*, int));',text)
        self.assertIn('#define xrtProbe (*XgeHostXrtSlot_xrtProbe)',text)
        self.assertNotIn('xrtImplementationOnly',text)
        self.assertNotIn('XgeHostXrtType_invented',text)

    def test_line_endings_do_not_change_abi_fingerprint(self):
        source = 'XRT_API int xrtProbe(void);\n'+BOUNDARY
        a,_ = self.generate(source); b,_ = self.generate(source.replace('\n','\r\n'))
        self.assertEqual(a['headerSha256'],b['headerSha256'])
        self.assertNotEqual(a['sourceSha256'],b['sourceSha256'])

    def test_unknown_function_macros_and_data_fail_closed(self):
        for source in ['XRT_API int xrtProbe(void);\n#define xrtProbe() 1\n',
                'XRT_API int xrtGlobal;\n','XRT_API int (*xrtProbe(void))(int);\n']:
            with self.subTest(source=source),self.assertRaises(ValueError): self.generate(source+BOUNDARY)

    def test_debug_wrappers_remain_in_original_conditional_scope(self):
        _,text = self.generate('''XRT_API void* xrtMalloc(unsigned size);
XRT_API void* xrtMallocAt(unsigned size, const char* file, unsigned line);
#if DEBUG
#define xrtMalloc(size) xrtMallocAt((size), __FILE__, __LINE__)
#endif
'''+BOUNDARY)
        self.assertIn('#if DEBUG\n#undef xrtMalloc\n#define xrtMalloc(size)',text)

    def test_plain_unmapped_and_unknown_imports_fail_closed(self):
        metadata = dict(names=['xrtAlloc','xrtVersion'])
        self.assertEqual(host_xrt.required_symbols(' U XgeHostXrtSlot_xrtAlloc\n',metadata),['xrtAlloc','xrtVersion'])
        for source in [' U xrtAlloc\n',' U __imp_xrtAlloc\n',' U XgeHostXrtSlot_xrtUnknown\n']:
            with self.subTest(source=source),self.assertRaises(ValueError): host_xrt.required_symbols(source,metadata)


if __name__ == '__main__': unittest.main()
