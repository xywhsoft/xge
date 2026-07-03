# XUI Refactor Workspace

This directory prepares an isolated XUI refactor workspace. It currently contains a copied XGE core runtime, platform scaffolding, third-party dependencies, and build scripts, but does not include the legacy XUI implementation.

## Contents

| Path | Description |
| --- | --- |
| `xge.h` | Copied XGE public header with legacy XUI declarations removed. |
| `xui.h` | XUI public API entry; future UI APIs belong here, not in `xge.h`. |
| `xge.c` | Aggregated XGE compilation entry. |
| `src/` | Copied XGE core implementation without `xge_xui*` files or legacy XUI aggregation. |
| `lib/` | xrt, sokol, stb, miniaudio, and related dependencies. |
| `platform/` | Android, iOS, Web, and mini-program scaffolding. |
| `res/` | Basic icon resources required by the DLL build. |
| `test/` | Current XGE smoke test without legacy XUI dependencies. |
| `test_xui/` | Dedicated XUI test code. |
| `docs/` | XUI architecture design documents. |

## Build Checks

Windows / MinGW-w64:

```bat
build_dll.bat
build_dbg_dll.bat
build_test.bat
build_dbg_test.bat
test_xui\build_proxy_xge_test.bat
```

The `build/` output directory is ignored by the root `.gitignore`.

