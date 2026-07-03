# MapEdit XUI2 Refactor Spec

This spec tracks the migration of `tools/mapedit` from the legacy `xge_xui_*`
UI stack to the native XUI2 controls under `dev/xui2`.

## Goals

- Keep the existing `tools/mapedit` executable buildable while the XUI2 version
  is developed in parallel.
- Use XUI2-native controls for the editor shell: toolbar, dock panel, canvas,
  status bar, tree/list style resource views, and property editing panels.
- Preserve the current map asset contract under `tools/mapedit/release/assets`.
- Add smoke arguments so each migration slice can be built and run headlessly
  with a deterministic final summary.

## Non-Goals

- Do not rewrite map data formats in this phase.
- Do not remove the legacy editor before the XUI2 replacement covers the
  existing map and tileset workflows.
- Do not introduce a separate UI framework outside XUI2.

## Architecture

- `tools/mapedit` remains the legacy editor and asset root.
- `tools/mapedit_xui2` is the parallel XUI2 editor prototype.
- Shared map loading and save logic should move into small non-UI modules only
  after the XUI2 shell is stable.
- XUI2 editor surfaces should use `xuiCanvas` for map rendering and XUI2 dock
  windows for panels.

## Migration Checklist

- [x] Create a parallel XUI2 map editor project.
- [x] Build an XUI2 editor shell with toolbar, dock panel, map canvas, side
  panels, output panel, and status bar.
- [x] Add smoke execution with `--frames N` and a final summary.
- [x] Load asset metadata from the current map editor release asset tree.
- [x] Mirror the legacy map and tileset workspace tab topology in XUI2 dock
  windows.
- [x] Render real tilesheet and autotile previews in the XUI2 asset panel.
- [ ] Port map workspace tabs: maps, tile select, edit, passage, tags.
- [ ] Port tileset workspace tabs: materials, sets, arrange, passage,
  actor overlay, tags.
- [~] Move map editing commands to XUI2 events and command handlers.
  Smoke now routes the Paint toolbar command into `mapeditXui2MapDocSetTile`.
- [~] Save edited map state through the existing map SDK contract.
  Smoke now writes an ignored `mapedit_xui2_smoke_saved_map.xson` output.
- [~] Extract full map XSON load/save out of the legacy map tab into a shared
  non-UI module.
- [x] Add an XUI2 map document module that loads map dimensions and tile arrays
  from existing map XSON files and writes a smoke-save output.
- [ ] Replace the legacy executable only after feature parity smoke tests pass.

## Smoke Gates

- `tools\mapedit_xui2\build.bat`
- `tools\mapedit_xui2\release\xge_mapedit_xui2.exe --frames 2`

The smoke run must print `mapedit_xui2 final-summary` with `create=1`,
`layout=1`, `canvas=1`, and nonzero panel/window counts.
