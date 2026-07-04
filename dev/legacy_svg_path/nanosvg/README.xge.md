NanoSVG is vendored here for SVG image rasterization fallback.

- Source: https://github.com/memononen/nanosvg
- Commit: 48120e91e64b2f409ed600cdfd6d790a49ba11ab
- Files: `nanosvg.h`, `nanosvgrast.h`
- License: embedded in the header files.

XGE uses this dependency only to load SVG image/icon files into RGBA pixels,
then uploads those pixels through the existing `xge_texture` GPU path.
