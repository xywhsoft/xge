# XGE

XGE is a C graphics and application-UI library. It provides a window and render loop, 2D drawing and text, resources and input, plus the retained-mode XUI widget library.

The current public version is **2.0.0**. [`xge.h`](xge.h) and [`xui.h`](xui.h) are the authoritative public API. Documentation and examples explain use of the declared API; private implementation details are not part of the contract.

## Start here

- [Build and verification](docs/BUILD.md)
- [Using XGE](docs/XGE.md)
- [Using XUI](docs/XUI.md)
- [Examples](docs/EXAMPLES.md)
- [Platform notes](docs/PLATFORM.md)
- [Emoji support](docs/EMOJI.md)

The complete release documentation is maintained in Chinese. The public headers and runnable examples are language-neutral and remain the reference for exact API names and signatures.

## Repository layout

| Path | Purpose |
| --- | --- |
| `xge.h` | Public XGE C API. |
| `xui.h` | Public XUI C API. |
| `xge.c`, `src/` | Implementation; applications must not depend on private types or functions. |
| `examples/` | Runnable XGE and XUI examples. |
| `test/`, `test_xui/` | Automated tests. |
| `platform/` | Android, iOS, and Web integration entry points. |
| `res/` | Built-in runtime resources. |

`build/` and `artifacts/` are local generated directories and are not release source.
