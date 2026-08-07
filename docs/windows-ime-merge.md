# Windows IME v2 merge guide

## Scope and base

The work was developed on `codex/windows-ime-tsf-v2` from mainline commit
`cd86f70`. It is intentionally isolated from ShapeEx, SVG, and unrelated XRT
work. The implementation is Windows-only behind `_WIN32` / `_WIN64` guards.

The normative behavior is defined in `docs/windows-ime-contract-v1.md`.

## Source groups

### XGE platform and ordered input

- `xge.h`
- `src/xge_core.c`
- `src/xge_impl.c`
- `src/xge_input.c`
- `src/xge_ime_win32_compat.h`
- `src/xge_ime_win32_tsf.c`
- `build_dll.bat`
- `build_dbg_dll.bat`

These files add the ordered event FIFO, IME presentation modes, Win32 TSF
backend, `ITextStoreACP`, UI-less candidate access, native-mode fallback, and
IME lifecycle hooks. Windows builds must link `ole32`, `oleaut32`, `uuid`, and
`imm32`.

### XUI input and editable controls

- `xui.h`
- `src/xui_internal.h`
- `src/xui_core.c`
- `src/xui_input.c`
- `src/xui_input_widget.c`
- `src/xui_text_edit.c`
- `src/xui_code_edit.c`
- `src/xui_terminal.c`
- `src/xui_proxy_xge.c`

These files add target-first key dispatch, consume results, marked-text
semantics, focused-control snapshots, caret geometry, candidate popup support,
and safe IME detach during context destruction. `xuiInternalContextSetImeDetach`
is exported only through the internal header so the separately built proxy can
link against `xge.lib`; it is not public application API.

`src/xui_proxy_xge.c` also contains the empty-geometry no-op correction
described in `docs/xui-empty-geometry-render-merge.md`.

## Public API additions

XGE adds:

- ordered input: `xgeInputEventGet/Post/PendingCount/DroppedCount`;
- key repeat compatibility: `xgeKeyRepeated`;
- IME mode, enable, caret rectangle, text client, candidate APIs, and explicit
  Full-mode candidate-presenter readiness;
- `xge_input_event_t`, `xge_ime_text_snapshot_t`,
  `xge_ime_text_client_t`, and candidate structures;
- `XGE_IME_MODE_NATIVE`, `COMPOSITION`, and `FULL`.

XUI adds:

- `xui_ime_composition_t` and `xuiInputImeCompositionEx`;
- `xuiInputKeyDownEx`, `KeyUpEx`, and `TextEx` consume results;
- marked-text query methods for Input, TextEdit, and CodeEdit;
- XGE proxy keyboard/IME pump and candidate presentation support.

The legacy polling getters and non-Ex XUI functions remain ABI-compatible
views. Integrations must choose one delivery model and must not drain both.

## Examples

The following examples share one UI and differ only by compile-time mode:

- `examples/xui_ime_native`
- `examples/xui_ime_composition`
- `examples/xui_ime_full`
- shared implementation: `examples/xui_ime_common`

Each supports `--frames N` for a noninteractive smoke run.

## Tests

Run before merging:

```bat
test\build_input_event_queue_test.bat
test\build_ime_mode_contract_test.bat
test\build_tsf_composition_store_test.bat
test\build_tsf_candidate_ui_test.bat
test_xui\build_input_test.bat
test_xui\build_input_widget_test.bat
test_xui\build_text_edit_test.bat
test_xui\build_code_edit_test.bat
test_xui\build_proxy_xge_test.bat
test_xui\build_layout_test.bat
test_xui\build_render_schedule_test.bat
test_xui\build_dock_panel_test.bat
test_xui\build_split_layout_test.bat
test_xui\build_context_test.bat
examples\xui_ime_native\build.bat
examples\xui_ime_composition\build.bat
examples\xui_ime_full\build.bat
build_dll.bat
```

The three examples should then smoke-run with `--frames 2` and pass the manual
matrix in the contract before release.

## Conflict guidance

1. Preserve newer mainline ShapeEx, SVG, renderer, and XRT changes.
2. Merge the XGE context fields and lifecycle calls rather than replacing the
   whole context or platform callback.
3. Keep TSF declarations in `xge_ime_win32_compat.h`; this avoids depending on
   a particular MinGW header revision.
4. Preserve the input order and TSF ownership boundary. Do not reintroduce key
   whitelists or a composition-state suppression table.
5. Preserve the XUI route: focused target, global hotkey, ancestor bubble,
   built-in default.
6. In render-proxy conflicts, validate handles first, treat empty geometry as
   a successful no-op second, then validate draw-specific values.
7. Rebuild DLL/import library. Do not copy binary artifacts from the patch into
   a branch with a different XRT or renderer ABI.

The supplied merge patch contains source, tests, examples, and documentation;
generated EXE/DLL/LIB files are deliberately excluded.
