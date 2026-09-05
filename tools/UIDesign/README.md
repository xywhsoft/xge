# XUI UI Design

XUI UI Design is the visual editor for XUI documents. Its design surface uses
a fixed 900 x 600 coordinate system, while the editor view can zoom and pan
without changing saved widget geometry.

## Build and run

From the repository root:

```bat
tools\UIDesign\build.bat
tools\UIDesign\release\xui_uidesign.exe
```

Useful headless and smoke-test modes:

```bat
tools\UIDesign\release\xui_uidesign.exe --frames 2
tools\UIDesign\release\xui_uidesign.exe --preview design.json --frames 2
tools\UIDesign\release\xui_uidesign.exe --generate design.json generated_view.c
```

Run the full UIDesign test gate with:

```bat
tools\UIDesign\test.bat
```

The gate verifies document round trips, size limits, and malformed-input
rejection; compiles and instantiates the generated C view; runs CLI generation;
and exercises both editor and preview paths. It also checks per-document undo,
history byte budgets, native input ordering and text-editor shortcut ownership,
cancelled close/exit, external file conflicts, layout recovery, and SDK hashes.

## Readability and input

The default font is Microsoft YaHei at **18 px**, with taller menus, toolbox
items, tree rows and property rows. Font lookup uses the executable directory
and the Windows directory, not the shell's current working directory:

1. An explicit `--font FILE`.
2. `font/UIDesign.ttf` next to the executable, if supplied by the user.
3. Windows `Fonts/msyh.ttc`, falling back to `Fonts/simhei.ttf`.

`--font-size 14..24` overrides the size. No proprietary font file from the IDE
is redistributed. Property help is always available at the bottom of the
inspector, and uses the registry's descriptions and default values. Identity
also shows whether the selected control type supports C export.

Input is delivered through `xuiProxyXgePumpInput`, including native text/IME
events. The focused input control gets the first chance to handle shortcuts;
design undo/copy/delete operate only when the design canvas is focused.
Esc cancels the current canvas operation; **Ctrl+W** closes a document and
**Ctrl+Q** exits. The OS window close button uses the same unsaved-changes guard.

## Workbench and sessions

New/Open create tabs (up to 16). Opening a path already in the workbench focuses
its tab. Each session owns its model, dirty baseline, undo/redo, zoom, pan,
grid and snap settings. Inactive sessions have no live widget resources.
Only the active document is materialized in the designer.

Undo has both a 64-entry limit and a **64 MiB per-document** byte budget; all
sessions together are capped at **128 MiB** of retained undo/redo snapshots.
Old inactive history is discarded first under the aggregate budget. Working
models, clean baselines and temporary snapshots are separate allocations.

Dock layouts are debounced and written atomically to
`%LOCALAPPDATA%/XUI/UIDesign/workspace.xson`. Persisted panel references use stable
names, not runtime integer IDs. **View > Reset Layout** restores the default.
Invalid/unknown-version layouts are not overwritten automatically. A failed
layout save reports an error and retries, without affecting design documents.
`--workspace FILE` supplies an isolated layout path. Bounded smoke runs do not
use the user's normal layout file unless explicitly requested.

The editor renders on demand, with scheduled checks for disk changes and layout
saves; `--frames` and preview runs remain continuous for deterministic testing.
Sessions are currently in-memory only: restarting restores the dock layout,
not unsaved documents or the list of open tabs.

The `ui_design_session` module is independent of the workbench. Session IDs and
monotonic content revisions provide a result-validation boundary for future
background jobs. Export remains synchronous in this milestone; no worker
threads or IDE integration have been added.

## Canvas navigation

- Use **View > Zoom In**, **Zoom Out**, **Zoom Fit**, or **100%** for precise
  zoom commands.
- Hold **Ctrl** and use the mouse wheel to zoom around the pointer.
- Drag with the middle mouse button to pan. A plain mouse wheel also pans.
- Grid and snapping remain expressed in design-space units, so saved rectangles
  do not change when the viewport changes.

## C export

**File > Export** writes a `.c` file and a sibling `.h` file. The first export
milestone supports Widget, Panel, Label, Button, Input, CheckBox, Image, and
Tabs. Export fails for unsupported control types instead of silently dropping
them.

The generated header exposes:

- a view struct containing the generated widget handles;
- an `onAction` host callback used by Button `event.onClick` values;
- a `resolveSurface` host callback used by Image source paths;
- create and destroy functions with explicit ownership.

Button action values are plain application-defined names such as
`settings.apply`; generated code does not assume a command framework.

## Document safety

Close/Reload prompt when a document differs from its last saved content.
Exit checks every dirty tab; cancelling preserves all tabs, including ones
already approved for closing. Selecting controls or navigating the canvas
does not make a document dirty. Active property edits are committed and
validated before switching, saving or closing.

Document writes use atomic replacement and support UTF-8 paths. File size and
last-write stamps detect external modifications or deletion; affected tabs are
marked `[disk!]`. Saving back to a changed file is refused. Use **File > Reload
from Disk** to accept the disk version (with an unsaved-changes prompt), or
**Save As** to keep your version under a different path. This is a conflict
guard, not an operating-system file lock.

The loader caps input at 16 MiB and validates node IDs,
parent/container relationships, cycles, duplicate properties, fixed-size text
limits, and supported document version before replacing the active model.

## SDK pairing and regression artifacts

Builds check required exports and generate `release/sdk-manifest.json` containing
SHA-256 hashes of XGE/XUI/XRT headers, the import library, DLL and executable.
Do not mix DLLs or headers from the IDE's SDK with this release. To verify:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/UIDesign/tools/sdk_manifest.ps1 -Verify
```

For a readable screenshot, use `--screenshot FILE.png --frames 2`.
The `--workbench-exercise --workspace FILE --frames 3` regression mode creates
two test documents beside FILE; use a new temporary directory. The normal test
gate provides and cleans its own isolated directory.
