# Direct-XRT static build

`tools/build/static.py` builds the same configured native profile as a static
archive. It does not replace the normal DLL build or install into release.
Use a frozen source root and a new output directory:

```powershell
python -B tools/build/static.py --root F:/frozen-xge --out F:/build/xge-static --profile default
```

On Linux use `python3` with GCC/binutils and the development libraries for
X11, Xi, Xcursor and OpenGL. The default profile retains XUI, text, audio,
shapes and its existing HarfBuzz dependency. `--profile`, exact `--define
NAME=0|1`, and `--debug-api` use the normal preprocessor feature guards.
A selected WebView2 backend requires the complete `WEBVIEW2_SDK_DIR`.
Reduced profiles are valid C SDKs; the current common xlang policy source
requires the full default declaration surface and its stager rejects a
reduced surface.

The flags include `-O2 -ffunction-sections -fdata-sections`,
`XGE_XRT_HOST=1`, and no XGE/XUI DLL import/export switches. The archive
contains no XRT implementation, generated XRT import slots, hosted binder,
or host-resolver reference. The final program provides its one XRT.
`xgeStaticXrtContractV1` is immutable cold ABI metadata, not a call bridge.

`single/xge.c` remains one aggregate member: its included engine sources
share private state. Other configured XUI/3D sources keep their actual TU
boundaries. Functions are not split by source rewriting. Final size depends
on the real linker's archive extraction and section reachability. Merely
placing the archive in a package does not prove dead-section collection.

The Linux build explicitly records its actual libgcc archive and glibc header
version; binaries using C23/fortified glibc exports do not thereby support
older libc versions.

On Windows the builder also preserves all native code members of the actual
GCC `libgcc.a`, `libmingwex.a` and `libmsvcrt.a` in separate support archives.
GNU `.idata$*` members are excluded from these code archives. Their actual
DLL/name metadata becomes a separate `.def`; no printf/scanf or CRT wrappers
are generated. The current supported toolchain is MSVCRT with MinGW ANSI
stdio, not UCRT. Keep CRT FILE objects and each CRT allocation/free pair in
the same CRT. The report records source archive/member hashes and the
MinGW runtime notices. Additional exact license texts can be supplied with
repeated `--runtime-notice PATH`; these are copied by the package stager.
The GNU runtime license is GPL-3.0-or-later with GCC-exception-3.1; preserve
its matching source/toolchain provenance when redistributing support code.

The Windows resource object is only the optional XGE DLL-shell icon. The
static package excludes it by default; applications retain their own icon.
Built-in font, emoji and atlas C data remain part of the selected features.

`--reuse-build OLD_OUT` verifies compiler, flags, dependency hashes and
object hash before copying a same-profile object. `build-report.json`
records actual compiled/reused TUs and every dependency. No cached object
is accepted on a source hash alone.

```powershell
python -B tools/test/check_static.py --root F:/frozen-xge --build F:/build/xge-static --xlang-root D:/GIT/x-lang --out F:/checks/xge-static
```

This CPU check links every selected native member without GC, then links
with GC, and runs build-info/color/owned-data-object operations with one
program XRT. It opens no window, GPU, browser or OS clipboard. It checks
C/C++ declarations and absence of dynamic XGE imports. GNU linking does not
qualify TinyCC's COFF/import/CRT support; the actual compiler integration
must test those separately. The default Markdown TU has real static TLS
sections and SECREL relocations; selected reachable TLS code needs correct
OS-loader PE TLS support. JIT TLS is not qualified by the GCC test and must
fail closed if unsupported. Historical failed attempts remain separate.
