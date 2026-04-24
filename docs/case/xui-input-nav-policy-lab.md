# XUI Input Nav Policy Lab

`examples/xui_input_nav_policy_lab` focuses on navigation rules for a single-line input. It isolates word-jump, `Home/End`, selection collapse, and scroll-follow behavior so the example can act as a compact regression point for cursor policy.

## Covered API

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetSelection`
- `xgeXuiInputGetSelection`
- `xgeXuiInputEvent`
- `xgeXuiInputEventProc`
- `xgeXuiInputUpdateProc`
- `xgeXuiInputPaintProc`

## Run

```bat
examples\xui_input_nav_policy_lab\build.bat
build\xge_xui_input_nav_policy_lab.exe --frames 5
```

## Auto Checks

- Verifies the widget is bound to `EventProc`, `UpdateProc`, and `PaintProc`.
- Starts from the end of a multi-word string and checks `Ctrl+Left` moves by word boundaries.
- Checks `Ctrl+Right` returns to the next word boundary.
- Checks `Home` jumps to cursor `0` and `End` jumps to the text tail.
- Creates a selection and verifies plain `Left` collapses to selection start while plain `Right` collapses to selection end.
- Jumps to `End` on a long line and verifies horizontal scroll becomes non-zero.

## Manual Checks

- The single input row should show a long sentence, with the caret ending near the right side.
- The final state should have no active selection.
- The input should appear horizontally scrolled rather than clipped awkwardly.

## Skip Conditions

- If no usable Windows font can be loaded, initialization fails before the example starts.
