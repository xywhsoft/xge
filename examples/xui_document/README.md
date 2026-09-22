# Unified Document example

From the repository root, with GCC and windres on PATH:

```bat
call examples\xui_document\build.bat
build\xui_document.exe
```

The left editor owns a rich document. The middle editor and right preview share
one Markdown document. Source/Visual/Live MD switches the middle editor's projection;
all three modes use the same revision and undo history. Live MD shows the active
top-level container as source and renders the other containers. Ctrl+Z/Y, Ctrl+B/I,
Ctrl+A/C/X/V, selection, mouse wheel and Ctrl+wheel zoom are supported.

For an automated real-XGE rendering check:

```bat
build\xui_document.exe --verify
```

This runs the GPU game-loop backend, hides the native window, renders three
frames in Source and three in Live MD, checks that each content panel contains
text pixels and the middle panel changes between modes, saves
`artifacts/xui-document-rebuild/native-smoke.png` and `native-live-smoke.png`, and exits with a failing status
on resource, rendering, pixel-validation or image-save failure. It does not
exercise a physical Windows IME, assistive technology or interactive dialogs.

See [the API and delivery notes](../../docs/XUI_DOCUMENT.md) for ownership,
transactions, file saving, test commands and current limitations. In particular,
the advanced-object provider callbacks are extension points; math and Mermaid
are not yet typeset by a default provider. Live MD conservatively falls back to
the whole source when the current parser cannot establish reliable container
boundaries; this fallback is reported by the renderer statistics.
