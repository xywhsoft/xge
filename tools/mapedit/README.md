# XGE MapEdit V2

MapEdit is an XGE/XUI V2 desktop editor. Source resources live in `assets`,
`option`, and `res`; generated executables and their staged resources live in
the ignored `build` directory. The historical `release` directory is retained
as a compatibility snapshot and is no longer modified by `build.bat`.

## Build and test

```bat
tools\mapedit\build.bat
tools\mapedit\tests\test.bat
tools\mapedit\tests\smoke.bat
```

The launcher is produced at `tools\mapedit\build\xge_mapedit.exe`.

## Project and user data

Pass a project root explicitly when editing data outside the packaged sample:

```bat
tools\mapedit\build\xge_mapedit.exe --project D:\games\my-project
```

`XGE_MAPEDIT_PROJECT_DIR` provides the same setting. Relative project paths are
resolved from the caller's working directory. Project assets and `setup.xson`
are read from that root. Layouts, startup diagnostics, and crash-recovery data
are stored under `%LOCALAPPDATA%\XGE\MapEdit` rather than beside the executable.
The `assets\*.xson` files are project metadata that map material filenames to
display names; MapEdit creates a missing metadata file when it first scans a
material category.

Map files use the versioned `xge.map` schema implemented by `map_sdk`. Legacy
unversioned maps remain readable and are migrated to the current schema when
saved. See `map_sdk\README.md` for the format contract.

The application uses XUI's XGE input bridge, hot-key commands, and modal
file/message dialogs. Rendering is on demand; animation, recovery, layout
debouncing and file-change polling request their next deadlines explicitly.

## IDE-inspired workbench update

- The IDE's `Consolas-with-Yahei_Nerd_Font.ttf` is now the preferred font at
  18px. Menu, tab, row and button metrics have been enlarged accordingly; XUI
  uses Chinese translations. Font provenance is documented in `res/fonts`.
- The workbench uses XUI Dock/Row/Column layouts. Tool selection and document
  actions have separate toolbar rows. Dock layout is saved after a 750ms
  debounce and again on normal shutdown.
- Save, Save As, Undo and Redo route to the active map/tileset workspace.
  Resource-property edits remain in memory until explicitly saved. Switching
  between the two workspaces does not discard either document's changes.
- Native close, menu exit and document replacement use Save / Don't Save /
  Cancel. A failed or canceled save aborts the pending operation. Native close
  uses the public `xgeSetQuitRequestCallback` API; rebuild the root XGE DLL if
  updating from a build without that API.
- Undo includes tile gestures, map/tileset properties, passage, overlay and
  custom data. A hash index makes repeated cell updates amortized O(1).
  History retains at most 128 commands and a 128MiB committed-history budget
  per document. Save points use revision IDs, including after branching.
  Metadata snapshots do not copy map tile planes; resizing does, and remaps
  passage/custom-data coordinates. Nested XRT snapshots use DeepClone, not
  shallow container Clone.
- Property values are updated in place while the inspected schema is stable,
  preserving selection and expanded categories. Invalid integer/size input
  leaves the map unchanged.
- All eight grid views share raster coordinate/zoom rules. Ctrl+wheel zooms
  at the cursor (25%-800%); the middle button pans; View offers 100% (Ctrl+0)
  and Fit. The minimum zoom applies to Fit too. Surfaces remain viewport-sized.
- Recovery files are keyed by canonical project/document paths and separated
  by document type. Both dirty documents are backed up. External file changes
  prompt for Reload / Keep; keeping memory blocks ordinary Save until reload
  or explicit Save As, protecting the external version.

### Module boundaries

`src/mapedit_history.c` owns command storage/indexing/savepoints;
`mapedit_document.c` owns CPU snapshots, no-op detection and resize/remapping;
`mapedit_files.c` owns canonical identity/file stamps;
`mapedit_view.c` owns raster-coordinate rules; `mapedit_theme.c` owns font and
theme initialization. Each is compiled separately; the application does not
textually include implementation `.c` files.

### Verification and remaining stages

The core tests cover one million cells with duplicate updates, savepoint and
branch behavior, metadata/tileset undo, nested-value independence, resizing,
coordinate edges and cancelable quit. GUI smoke tests send a real native
`WM_CLOSE`, cancel its confirmation, test failed-save continuation, tileset
Save As routing, property selection, recovery, zoom/pan and a 1024x1024 map.
`--frames` runs isolate layouts/recovery under `build/test-user`. `--capture`
writes a PNG of the actual XUI surface, not a mockup. Test documents are created
only under the build directory and removed by the test.

This stage keeps one map and one tileset document. Multi-document map tabs,
a searchable resource tree with native drag-and-drop, and background resource
indexing/loading are still subsequent stages, not implemented features.
