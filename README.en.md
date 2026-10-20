# XGE

XGE is a C graphics and application-UI library. It provides a window and render loop, 2D drawing and text, resources and input, plus the retained-mode XUI widget library.

The current public version is **3.0.0**. [`include/xge.h`](include/xge.h) and [`include/xui.h`](include/xui.h) are the authoritative public API. Documentation and examples explain use of the declared API; private implementation details are not part of the contract.

Image coordinates and migration: [XGE 3 coordinate contract](docs/guide/TEXTURE_COORDINATES.md).
## Start here

- [Build and verification](docs/guide/BUILD.md)
- [Using XGE](docs/guide/XGE.md)
- [2D particles](docs/guide/PARTICLES.md) and [seven runnable scenes](examples/xge/xge_particles/README.md)
- [Using XUI](docs/guide/XUI.md)
- [Examples](docs/guide/EXAMPLES.md)
- [Platform notes](docs/guide/PLATFORM.md)
- [Emoji support](docs/guide/EMOJI.md)

The complete release documentation is maintained in Chinese. The public headers and runnable examples are language-neutral and remain the reference for exact API names and signatures.

## Repository layout

| Path | Purpose |
| --- | --- |
| `include/` | Public XGE, XUI, and Document C APIs. |
| `single/xge.c` | Single-file XGE implementation entry point. |
| `src/xge/`, `src/xui/` | Private implementation grouped by subsystem. |
| `modules/` | Project-owned standalone modules such as xLayout. |
| `extlibs/` | Versioned third-party dependencies. |
| `examples/xge/`, `examples/xui/` | Runnable XGE and XUI examples. |
| `tests/xge/`, `tests/xui/` | Automated tests. |
| `apps/` | Complete tools and applications built on XGE/XUI. |
| `platform/` | Android, iOS, and Web integration entry points. |
| `res/xge/`, `res/xui/` | Built-in runtime resources. |
| `tools/` | Build, test, check, generation, and packaging tools. |

`build/` and `artifacts/` are local generated directories and are not release source.
